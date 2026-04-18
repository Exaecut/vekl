#pragma once

inline float gaussian_weight(int x, int y, float sigma)
{
	float s2 = sigma * sigma;
	return exp(-(x * x + y * y) / (2.0f * s2));
}

inline float gaussian_weight_1d(int x, float sigma)
{
	float s2 = sigma * sigma;
	return exp(-(float(x * x)) / (2.0f * s2));
}

#ifdef VEKL_CPU

#define VEKL_GAUSSIAN_WEIGHT_CACHE 64

/// Separable 1D Gaussian convolution — **CPU fast path**.
///
/// - Hoists the `exp()` weight computation into a thread-local cache keyed on
///   `(sigma, radius)` (each rayon worker has its own, no locks).
/// - Uses cheap 1D linear interpolation along the blur axis: 2 `pixel_load`s
///   per tap (vs 1 for nearest, 4 for full bilinear). This eliminates the
///   banding artifacts of nearest-neighbour at low radius while keeping the
///   hot path at 50 % of full bilinear cost.
///
/// Samples at stride-2 (offsets 0, 2, 4, …) with weights evaluated at the
/// actual pixel offset, producing a faithful Gaussian shape with half the
/// texture fetches of stride-1.
template <typename Storage, typename Layout = layout_rgba>
inline float4 gaussian_1d(image_2d<const Storage, Layout> tex, float2 uv, float sigma, int radius, bool vertical)
{
	thread_local static float _gk_weights[VEKL_GAUSSIAN_WEIGHT_CACHE];
	thread_local static float _gk_sigma = -1.0f;
	thread_local static int   _gk_radius = -1;

	int n_weights = radius / 2 + 1;
	if (n_weights > VEKL_GAUSSIAN_WEIGHT_CACHE) {
		n_weights = VEKL_GAUSSIAN_WEIGHT_CACHE;
	}

	if (sigma != _gk_sigma || radius != _gk_radius) {
		for (int i = 0; i < n_weights; ++i) {
			_gk_weights[i] = gaussian_weight_1d(i * 2, sigma);
		}
		_gk_sigma = sigma;
		_gk_radius = radius;
	}

	float2 pf = pixel_coord(uv, tex.size_px);
	int ix = int(floor(pf.x));
	int iy = int(floor(pf.y));
	float frac = vertical ? (pf.y - float(iy)) : (pf.x - float(ix));

	const int sz_x = int(tex.size_px.x);
	const int sz_y = int(tex.size_px.y);

	// Clamp-to-edge helpers returning unsigned for read_unchecked
	auto cx = [sz_x](int x) -> unsigned { return (unsigned)(x < 0 ? 0 : (x >= sz_x ? sz_x - 1 : x)); };
	auto cy = [sz_y](int y) -> unsigned { return (unsigned)(y < 0 ? 0 : (y >= sz_y ? sz_y - 1 : y)); };

	// Step along the blur axis
	const int d0 = vertical ? 0 : 1;
	const int d1 = vertical ? 1 : 0;

	// Centre tap — 1D lerp along blur axis
	float4 s0 = tex.read_unchecked(uint2(cx(ix),      cy(iy)));
	float4 s1 = tex.read_unchecked(uint2(cx(ix + d0), cy(iy + d1)));
	float4 sum = mix(s0, s1, frac) * _gk_weights[0];
	float weight_sum = _gk_weights[0];

	const int half = radius / 2;
	for (int i = 1; i <= half; ++i)
	{
		const float w = _gk_weights[i];
		const int off = i * 2;

		// +offset tap: lerp between base and base+1 along blur axis
		int ax = ix + d0 * off;
		int ay = iy + d1 * off;
		float4 a0 = tex.read_unchecked(uint2(cx(ax),      cy(ay)));
		float4 a1 = tex.read_unchecked(uint2(cx(ax + d0), cy(ay + d1)));

		// -offset tap
		int bx = ix - d0 * off;
		int by = iy - d1 * off;
		float4 b0 = tex.read_unchecked(uint2(cx(bx),      cy(by)));
		float4 b1 = tex.read_unchecked(uint2(cx(bx + d0), cy(by + d1)));

		sum += (mix(a0, a1, frac) + mix(b0, b1, frac)) * w;
		weight_sum += 2.0f * w;
	}

	return sum / max(weight_sum, 1e-8f);
}

#else // !VEKL_CPU

/// Separable 1D Gaussian convolution
///
/// Uses stride-2 sampling with hardware bilinear interpolation to blend
/// adjacent texel pairs per tap. For each pair at offsets (2i-1, 2i), the
/// bilinear sample is placed at the weighted midpoint so that the combined
/// tap reproduces the exact weighted sum `w_a * c_a + w_b * c_b` in a
/// single texture fetch.
template <typename Storage, typename Layout = layout_rgba>
inline float4 gaussian_1d(image_2d<const Storage, Layout> tex, float2 uv, float sigma, int radius, bool vertical)
{
	float2 texel_size = 1.0 / float2(tex.size_px);
	float2 dir = vertical ? float2(0.0, 1.0) : float2(1.0, 0.0);

	float4 sum = float4(0.0);
	float weight_sum = 0.0;

	// Centre tap
	float wc = gaussian_weight_1d(0, sigma);
	sum += tex.sample_linear(uv) * wc;
	weight_sum += wc;

	// Stride-2: each iteration covers two texel offsets (2i-1, 2i)
	// using one bilinear fetch placed at the Gaussian-weighted midpoint.
	const int half = radius / 2;
	for (int i = 1; i <= half; ++i)
	{
		float w_a = gaussian_weight_1d(i * 2 - 1, sigma);
		float w_b = gaussian_weight_1d(i * 2,     sigma);
		float w_combined = w_a + w_b;

		// Weighted midpoint between texel (2i-1) and texel (2i):
		// bilinear at this offset yields (w_a*c_a + w_b*c_b) / w_combined
		float midpoint = float(i * 2 - 1) + w_b / max(w_combined, 1e-8f);
		float2 offset = dir * (midpoint * texel_size);

		float4 c1 = tex.sample_linear(uv + offset);
		float4 c2 = tex.sample_linear(uv - offset);

		sum += (c1 + c2) * w_combined;
		weight_sum += 2.0 * w_combined;
	}

	// If radius is odd, one texel remains uncovered — add it separately
	if ((radius & 1) != 0 && radius > 0)
	{
		float w = gaussian_weight_1d(radius, sigma);
		float2 offset = dir * (float(radius) * texel_size);

		float4 c1 = tex.sample_linear(uv + offset);
		float4 c2 = tex.sample_linear(uv - offset);

		sum += (c1 + c2) * w;
		weight_sum += 2.0 * w;
	}

	return sum / max(weight_sum, 1e-8);
}

#endif // VEKL_CPU

template <typename Storage, typename Layout = layout_rgba>
inline float4 gaussian_2d(image_2d<const Storage, Layout> tex, float2 uv, float sigma, int radius)
{
	float4 sum = float4(0.0);
	float weight_sum = 0.0;

	for (int y = -radius; y <= radius; ++y)
	{
		for (int x = -radius; x <= radius; ++x)
		{
			float w = gaussian_weight(x, y, sigma);
			float2 offset = float2(x, y) / float2(tex.size_px);
			float4 c = tex.sample_linear(uv + offset);

			sum += c * w;
			weight_sum += w;
		}
	}

	return sum / weight_sum;
}

template <typename Storage, typename Layout = layout_rgba>
inline float4 radial_blur_linear(image_2d<const Storage, Layout> src,
								 float2 uv,
								 float strength,
								 float2 center,
								 float influence,
								 uint2 gid)
{
	float2 direction = (uv - center) * strength;
	int samples = 16;

	if (influence <= 0.0001f)
	{
		return src.sample_linear(uv);
	}

	float weight_accum = 0.0;
	float4 color_accum = float4(0.0);

	float influence_mask = smoothstep(1.0f - influence, 1.0f, length(direction));

	float rand = fract(sin(dot(float2(gid), float2(12.9898f, 78.233f))) * 43758.5453f);
	for (int i = 0; i < samples; ++i)
	{
		float progression = ((float(i) + 0.5f) + (rand - 0.5f)) / float(samples);
		float2 offset = uv + direction * influence_mask * progression;

		float weight = (1.0f - progression);
		weight *= weight;

		color_accum += src.sample_linear_mirror(offset) * weight;
		weight_accum += weight;
	}

	return color_accum / max(weight_accum, 1e-6f);
}

template <typename Storage, typename Layout = layout_rgba>
inline float4 directional_blur_linear(image_2d<const Storage, Layout> src,
									  float2 uv,
									  float angle_deg,
									  float strength,
									  float spread,
									  uint2 gid)
{
	float angle = radians(angle_deg);
	float2 dir = float2(cos(angle), sin(angle)) * strength;

	int samples = 32;

	float weight_accum = 0.0;
	float4 color_accum = float4(0.0);

	float rand = fract(sin(dot(float2(gid), float2(12.9898f, 78.233f))) * 43758.5453f);

	for (int i = 0; i < samples; ++i)
	{
		float progression = (float(i) + 0.5f) / float(samples);
		float jitter = (rand - 0.5f) * spread / float(samples);
		float2 offset = uv + dir * (progression + jitter);
		float weight = exp(-progression * progression * 4.0);

		color_accum += src.sample_linear_mirror(offset) * weight;
		weight_accum += weight;
	}

	return color_accum / max(weight_accum, 1e-6f);
}
