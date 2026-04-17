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

#define VEKL_GAUSSIAN_WEIGHT_CACHE 48

/// Separable 1D Gaussian convolution — **CPU fast path**.
///
/// - Hoists the `exp()` weight computation into a thread-local cache keyed on
///   `(sigma, radius)` (each rayon worker has its own, no locks).
/// - Replaces `sample_linear` (4 `pixel_load` calls per tap) by direct
///   `read_unchecked` with signed-int clamping: since sample offsets are
///   exact integer texel multiples of `texel_size`, the bilinear interpolation
///   was collapsing to the nearest texel anyway — the 3 extra taps were pure
///   waste (≥ 3× pixel_load reduction on the hot path).
///
/// Preserves the existing `gaussian_weight_1d(i, sigma)` formula and `i*3`
/// stride, so output is bit-identical to the old path modulo FP32 rounding
/// (well below U8 quantization).
template <typename Storage, typename Layout = layout_rgba>
inline float4 gaussian_1d(image_2d<const Storage, Layout> tex, float2 uv, float sigma, int radius, bool vertical)
{
	thread_local static float _gk_weights[VEKL_GAUSSIAN_WEIGHT_CACHE];
	thread_local static float _gk_sigma = -1.0f;
	thread_local static int   _gk_radius = -1;

	int n_weights = radius / 3 + 1;
	if (n_weights > VEKL_GAUSSIAN_WEIGHT_CACHE) {
		n_weights = VEKL_GAUSSIAN_WEIGHT_CACHE;
	}

	if (sigma != _gk_sigma || radius != _gk_radius) {
		for (int i = 0; i < n_weights; ++i) {
			_gk_weights[i] = gaussian_weight_1d(i, sigma);
		}
		_gk_sigma = sigma;
		_gk_radius = radius;
	}

	float2 pf = pixel_coord(uv, tex.size_px);
	int base_x = int(pf.x + 0.5f);
	int base_y = int(pf.y + 0.5f);

	const int sz_x = int(tex.size_px.x);
	const int sz_y = int(tex.size_px.y);
	const int dx = vertical ? 0 : 1;
	const int dy = vertical ? 1 : 0;

	int cx = base_x < 0 ? 0 : (base_x >= sz_x ? sz_x - 1 : base_x);
	int cy = base_y < 0 ? 0 : (base_y >= sz_y ? sz_y - 1 : base_y);
	float4 sum = tex.read_unchecked(uint2((unsigned)cx, (unsigned)cy)) * _gk_weights[0];
	float weight_sum = _gk_weights[0];

	const int half = radius / 3;
	for (int i = 1; i <= half; ++i)
	{
		const float w = _gk_weights[i];
		const int off = i * 3;

		int px1 = base_x + dx * off;
		int py1 = base_y + dy * off;
		int px2 = base_x - dx * off;
		int py2 = base_y - dy * off;

		px1 = px1 < 0 ? 0 : (px1 >= sz_x ? sz_x - 1 : px1);
		py1 = py1 < 0 ? 0 : (py1 >= sz_y ? sz_y - 1 : py1);
		px2 = px2 < 0 ? 0 : (px2 >= sz_x ? sz_x - 1 : px2);
		py2 = py2 < 0 ? 0 : (py2 >= sz_y ? sz_y - 1 : py2);

		float4 c1 = tex.read_unchecked(uint2((unsigned)px1, (unsigned)py1));
		float4 c2 = tex.read_unchecked(uint2((unsigned)px2, (unsigned)py2));

		sum += (c1 + c2) * w;
		weight_sum += 2.0f * w;
	}

	return sum / max(weight_sum, 1e-8f);
}

#else // !VEKL_CPU — GPU backends keep the original bilinear-sampling path.

/// Separable 1D Gaussian convolution — GPU path (CUDA / Metal / OpenCL).
///
/// Kept as-is: `sample_linear` maps to hardware-accelerated bilinear on the
/// GPU so the CPU-specific perf trade-offs do not apply. `thread_local` is
/// not supported in device code so the weight cache is CPU-only.
template <typename Storage, typename Layout = layout_rgba>
inline float4 gaussian_1d(image_2d<const Storage, Layout> tex, float2 uv, float sigma, int radius, bool vertical)
{
	float2 texel_size = 1.0 / float2(tex.size_px);
	float2 dir = vertical ? float2(0.0, 1.0) : float2(1.0, 0.0);

	float4 sum = float4(0.0);
	float weight_sum = 0.0;

	float wc = gaussian_weight_1d(0, sigma);
	sum += tex.sample_linear(uv) * wc;
	weight_sum += wc;

	for (int i = 1; i <= radius / 3; ++i)
	{
		float w = gaussian_weight_1d(i, sigma);
		float2 offset = dir * (float(i * 3) * texel_size);

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
