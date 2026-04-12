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
	// VUYA → RGBA (BT.601)
	inline float4 to_rgba(float4 c) const {
		float v = c.x - 0.5f;
		float u = c.y - 0.5f;
		float y = c.z;
		return float4(
			y + 1.402f * v,
			y - 0.344136f * u - 0.714136f * v,
			y + 1.772f * u,
			c.w
		);
	}
	// RGBA → VUYA (BT.601)
	inline float4 from_rgba(float4 c) const {
		float y = 0.299f * c.x + 0.587f * c.y + 0.114f * c.z;
		return float4(
			(c.x - y) / 1.402f + 0.5f,
			(c.z - y) / 1.772f + 0.5f,
			y,
			c.w
		);
	}
};

struct layout_vuya709 {
	// VUYA → RGBA (BT.709)
	inline float4 to_rgba(float4 c) const {
		float v = c.x - 0.5f;
		float u = c.y - 0.5f;
		float y = c.z;
		return float4(
			y + 1.5748f * v,
			y - 0.1873f * u - 0.4681f * v,
			y + 1.8556f * u,
			c.w
		);
	}
	// RGBA → VUYA (BT.709)
	inline float4 from_rgba(float4 c) const {
		float y = 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
		return float4(
			(c.x - y) / 1.5748f + 0.5f,
			(c.z - y) / 1.8556f + 0.5f,
			y,
			c.w
		);
	}
};

struct layout_auto {
	uint layout_type; // 0=RGBA, 1=BGRA, 2=VUYA601, 3=VUYA709

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
