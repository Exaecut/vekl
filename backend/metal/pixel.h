#pragma once

#ifdef USE_HALF_PRECISION
typedef half4 pixel;
#else
typedef float4 pixel;
#endif

struct layout_rgba {
	inline float4 to_rgba(float4 c) const { return c; }
	inline float4 from_rgba(float4 c) const { return c; }
};

struct layout_bgra {
	inline float4 to_rgba(float4 c) const { return float4(c.b, c.g, c.r, c.a); }
	inline float4 from_rgba(float4 c) const { return float4(c.z, c.y, c.x, c.w); }
};

struct layout_vuya {

	inline float4 to_rgba(float4 c) const {
		// For 32f VUYA (format==16), chroma is already centered at 0 (range [-0.5, 0.5]).
		// For 8u/16u VUYA, chroma is in [0,1] after normalization and needs 0.5 offset.
		float chroma_offset = (format == 16u) ? 0.0f : 0.5f;
		float v = c.x - chroma_offset;
		float u = c.y - chroma_offset;
		float y = c.z;
		return float4(
			y + 1.402f * v,
			y - 0.344136f * u - 0.714136f * v,
			y + 1.772f * u,
			c.w
		);
	}

	inline float4 from_rgba(float4 c) const {
		float chroma_offset = (format == 16u) ? 0.0f : 0.5f;
		float y = 0.299f * c.x + 0.587f * c.y + 0.114f * c.z;
		return float4(
			(c.x - y) / 1.402f + chroma_offset,
			(c.z - y) / 1.772f + chroma_offset,
			y,
			c.w
		);
	}
};

struct layout_vuya709 {

	inline float4 to_rgba(float4 c) const {
		float chroma_offset = (format == 16u) ? 0.0f : 0.5f;
		float v = c.x - chroma_offset;
		float u = c.y - chroma_offset;
		float y = c.z;
		return float4(
			y + 1.5748f * v,
			y - 0.1873f * u - 0.4681f * v,
			y + 1.8556f * u,
			c.w
		);
	}

	inline float4 from_rgba(float4 c) const {
		float chroma_offset = (format == 16u) ? 0.0f : 0.5f;
		float y = 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
		return float4(
			(c.x - y) / 1.5748f + chroma_offset,
			(c.z - y) / 1.8556f + chroma_offset,
			y,
			c.w
		);
	}
};

struct layout_auto {
	uint layout_type;

	inline float4 to_rgba(float4 c) const {
		switch (layout_type) {
			case 2: return layout_vuya().to_rgba(c);
			case 3: return layout_vuya709().to_rgba(c);
			case 1: return layout_bgra().to_rgba(c);
			default: return c;
		}
	}

	inline float4 from_rgba(float4 c) const {
		switch (layout_type) {
			case 2: return layout_vuya().from_rgba(c);
			case 3: return layout_vuya709().from_rgba(c);
			case 1: return layout_bgra().from_rgba(c);
			default: return c;
		}
	}
};

inline float4 pixel_load(device const pixel *data, uint pitch_px, uint2 xy) {
	return float4(data[xy.y * pitch_px + xy.x]);
}

inline void pixel_store(device pixel *data, uint pitch_px, uint2 xy, float4 c) {
	data[xy.y * pitch_px + xy.x] = pixel(c);
}
