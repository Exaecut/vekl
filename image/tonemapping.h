#pragma once

inline float srgb_to_linear_fast(float x)
{
	float lo = x * (1.0f / 12.92f);
	float hi = pow((x + 0.055f) * (1.0f / 1.055f), 2.4f);
	float t = saturate((x - 0.04045f) * 5957.8977f);
	return mix(lo, hi, t);
}

inline float3 srgb_to_linear_fast(float3 c)
{
	return float3(srgb_to_linear_fast(c.x),
				  srgb_to_linear_fast(c.y),
				  srgb_to_linear_fast(c.z));
}

inline float linear_to_srgb_fast(float x)
{
	float lo = x * 12.92f;
	float hi = 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
	float t = saturate((x - 0.0031308f) * 323.2649f);
	return mix(lo, hi, t);
}

inline float3 linear_to_srgb_fast(float3 c)
{
	return float3(linear_to_srgb_fast(c.x),
				  linear_to_srgb_fast(c.y),
				  linear_to_srgb_fast(c.z));
}

inline float3 aces_filmic(float3 x)
{
	float3 num = x * (2.51f * x + 0.03f);
	float3 den = x * (2.43f * x + 0.59f) + 0.14f;
	return saturate(num * (1.0f / den));
}

inline float3 apply_exposure(float3 rgb, float EV)
{
	float scale = exp2(EV);
	return rgb * scale;
}

inline float4 tonemap_srgb(float4 src, float EV, float premul)
{
	float3 rgb = float3(src.x, src.y, src.z);
	float a = src.w;

	float inv_a = (a > 0.0f) ? (1.0f / a) : 0.0f;
	rgb = mix(rgb, rgb * inv_a, premul);

	float3 lin = srgb_to_linear_fast(rgb);
	float3 exposed = apply_exposure(lin, EV);
	float3 mapped = aces_filmic(exposed);
	float3 out_rgb = linear_to_srgb_fast(mapped);

	out_rgb = mix(out_rgb, out_rgb * a, premul);

	return float4(out_rgb.x, out_rgb.y, out_rgb.z, a);
}

inline float4 tonemap_linear(float4 src, float EV, float premul)
{
	float3 rgb = float3(src.x, src.y, src.z);
	float a = src.w;

	float inv_a = (a > 0.0f) ? (1.0f / a) : 0.0f;
	rgb = mix(rgb, rgb * inv_a, premul);

	float3 exposed = apply_exposure(rgb, EV);
	float3 mapped = aces_filmic(exposed);

	mapped = mix(mapped, mapped * a, premul);

	return float4(mapped.x, mapped.y, mapped.z, a);
}
