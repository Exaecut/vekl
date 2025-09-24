inline float envelope(float x, float a, float b, float c)
{
	// rising edge: 0 → 1 between a..b
	float rise = smoothstep(a, b, x);

	// falling edge: 1 → 0 between b..c
	float fall = 1.0 - smoothstep(b, c, x);

	return rise * fall;
}

// x: input in [0..1], typically a mask/gradient value
// a, b, c: envelope points controlling activation (like envelope())
// color: neon base color
// thickness: core stroke width
// glow_radius: size of glow halo
// intensity: overall brightness
inline float3 glow_envelope(float x,
								 float a, float b, float c,
								 float3 color,
								 float thickness,
								 float glow_radius,
								 float intensity)
{
	// envelope-shaped mask: 0 at a/c, 1 at b
	float rise = smoothstep(a, b, x);
	float fall = 1.0 - smoothstep(b, c, x);
	float mask = rise * fall;

	// distance proxy (how far from the "peak" b we are)
	float d = abs(x - b);

	// --- Core stroke ---
	float core = smoothstep(thickness, 0.0, d);

	// --- Mid glow ---
	float mid = exp(-3.0 * d / glow_radius);

	// --- Outer halo ---
	float outer = exp(-1.0 * d / (2.0 * glow_radius));

	// Combine with envelope mask
	float3 result =
		color * core * 1.0 * mask + // solid tube
		color * mid * 0.6 * mask +	// inner glow
		color * outer * 0.3 * mask; // wide halo

	return result * intensity;
}
