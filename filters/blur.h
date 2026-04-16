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
