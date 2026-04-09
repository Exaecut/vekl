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

inline float4 pixel_load(device const pixel *data, uint pitch_px, uint2 xy) {
	return float4(data[xy.y * pitch_px + xy.x]);
}

inline void pixel_store(device pixel *data, uint pitch_px, uint2 xy, float4 c) {
	data[xy.y * pitch_px + xy.x] = pixel(c);
}
