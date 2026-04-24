#pragma once

inline uint index_of(uint2 xy, uint pitch_px) { return xy.y * pitch_px + xy.x; }

inline uint2 clamp_xy(uint2 xy, uint2 size_px)
{
	int x = clamp(int(xy.x), 0, int(size_px.x) - 1);
	int y = clamp(int(xy.y), 0, int(size_px.y) - 1);
	return uint2(x, y);
}

inline float4 bilinear_sample(device const pixel *data, uint pitch_px,
							   uint2 size_px, float2 uv, uint format)
{
	float2 p = pixel_coord(uv, size_px);
	float2 pf = floor(p);
	float2 f = clamp(p - pf, 0.0f, 1.0f);

	uint2 xy00 = clamp_xy(uint2(pf), size_px);
	uint2 xy10 = clamp_xy(uint2(pf.x + 1.0f, pf.y), size_px);
	uint2 xy01 = clamp_xy(uint2(pf.x, pf.y + 1.0f), size_px);
	uint2 xy11 = clamp_xy(uint2(pf + 1.0f), size_px);

	float4 c00 = pixel_load(data, pitch_px, xy00, format);
	float4 c10 = pixel_load(data, pitch_px, xy10, format);
	float4 c01 = pixel_load(data, pitch_px, xy01, format);
	float4 c11 = pixel_load(data, pitch_px, xy11, format);

	float4 cx0 = mix(c00, c10, f.x);
	float4 cx1 = mix(c01, c11, f.x);
	return mix(cx0, cx1, f.y);
}

template <typename Storage, typename Layout = layout_rgba>
struct image_2d
{
	device Storage *data;
	uint pitch_px;
	uint2 size_px;
	Layout layout;
	uint format;

	image_2d() : data(nullptr), pitch_px(0), size_px(uint2(0, 0)), format(VEKL_FORMAT) {}
	image_2d(device Storage *d, uint p, uint2 s)
		: data(d), pitch_px(p), size_px(s), format(VEKL_FORMAT) {}
	image_2d(device Storage *d, uint p, uint2 s, Layout l)
		: data(d), pitch_px(p), size_px(s), layout(l), format(VEKL_FORMAT) {}
	image_2d(device Storage *d, uint p, uint2 s, Layout l, uint fmt)
		: data(d), pitch_px(p), size_px(s), layout(l), format(fmt) {}

	inline float4 read(uint2 xy) const
	{
		xy = clamp_xy(xy, size_px);
		return layout.to_rgba(pixel_load(data, pitch_px, xy, format));
	}

	inline void write(uint2 xy, float4 c)
	{
		xy = clamp_xy(xy, size_px);
		pixel_store(data, pitch_px, xy, layout.from_rgba(c), format);
	}

	inline float4 sample_nearest(float2 uv) const
	{
		uint2 xy = clamp_xy(uint2(pixel_coord(uv, size_px) + 0.5f), size_px);
		return layout.to_rgba(pixel_load(data, pitch_px, xy, format));
	}

	inline float4 sample_linear(float2 uv) const
	{
		return layout.to_rgba(bilinear_sample(data, pitch_px, size_px, uv, format));
	}

	inline float4 sample_linear_repeat(float2 uv) const
	{
		return layout.to_rgba(bilinear_sample(data, pitch_px, size_px, fract(uv), format));
	}

	inline float4 sample_nearest_repeat(float2 uv) const
	{
		float2 uv_wrapped = fract(uv);
		uint2 xy = clamp_xy(uint2(pixel_coord(uv_wrapped, size_px) + 0.5f), size_px);
		return layout.to_rgba(pixel_load(data, pitch_px, xy, format));
	}

	inline float4 sample_linear_mirror(float2 uv) const
	{
		float2 uv_mirrored = abs(fract(uv * 0.5f) * 2.0f - 1.0f);
		uv_mirrored.x = 1.0f - uv_mirrored.x;
		uv_mirrored.y = 1.0f - uv_mirrored.y;
		return layout.to_rgba(bilinear_sample(data, pitch_px, size_px, uv_mirrored, format));
	}

	inline float4 sample_nearest_mirror(float2 uv) const
	{
		float2 uv_mirrored = abs(fract(uv * 0.5f) * 2.0f - 1.0f);
		uv_mirrored.y = 1.0f - uv_mirrored.y;
		uint2 xy = clamp_xy(uint2(pixel_coord(uv_mirrored, size_px) + 0.5f), size_px);
		return layout.to_rgba(pixel_load(data, pitch_px, xy, format));
	}

	inline float4 sample_bicubic(float2 uv) const
	{
		float2 p = pixel_coord(uv, size_px);
		float2 pf = floor(p);
		float2 f = p - pf;
		float2 f2 = f * f;
		float2 f3 = f2 * f;

		float2 w0 = f3 - 2.0f * f2 + f;
		float2 w1 = 3.0f * f3 - 5.0f * f2 + 2.0f;
		float2 w2 = -3.0f * f3 + 4.0f * f2 + f;
		float2 w3 = f3 - f2;

		float2 g0 = w0 + w1;
		float2 g1 = w2 + w3;
		float2 h0 = (w1 / g0) - 0.5f + pf;
		float2 h1 = (w3 / g1) + 0.5f + pf;

		float4 c00 = sample_linear((h0 + float2(0.0f, 0.0f)) / float2(size_px));
		float4 c10 = sample_linear((h0 + float2(1.0f, 0.0f)) / float2(size_px));
		float4 c01 = sample_linear((h0 + float2(0.0f, 1.0f)) / float2(size_px));
		float4 c11 = sample_linear((h0 + float2(1.0f, 1.0f)) / float2(size_px));
		float4 c0 = mix(c00, c10, g1.x);
		float4 c1 = mix(c01, c11, g1.x);
		return mix(c0, c1, g1.y);
	}

	inline float4 read_unchecked(uint2 xy) const
	{
		return layout.to_rgba(pixel_load(data, pitch_px, xy, format));
	}

	inline void write_unchecked(uint2 xy, float4 c)
	{
		pixel_store(data, pitch_px, xy, layout.from_rgba(c), format);
	}

	inline float4 sample_linear_bias(float2 uv, float bias) const
	{
		float b = clamp(bias, 0.0f, 4.0f);
		int r = (int)floor(pow(2.0f, b));
		float2 texel = 1.0f / float2(size_px);
		if (r <= 0)
			return sample_linear_repeat(uv);
		float sigma = max(1.0f, float(r)) * 0.5f;
		float two_sigma2 = 2.0f * sigma * sigma;
		float4 sum_c = float4(0.0f);
		float sum_w = 0.0f;
		for (int y = -r; y <= r; ++y)
		{
			for (int x = -r; x <= r; ++x)
			{
				float d2 = float(x * x + y * y);
				float w = exp(-d2 / two_sigma2);
				float2 off = float2(float(x) * texel.x, float(y) * texel.y);
				float4 c = sample_linear_repeat(uv + off);
				sum_c += c * w;
				sum_w += w;
			}
		}
		return sum_c / max(sum_w, 1e-6f);
	}
};
