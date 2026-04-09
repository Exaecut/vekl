#pragma once

inline float envelope(float x, float a, float b, float c)
{
	float rise = smoothstep(a, b, x);
	float fall = 1.0 - smoothstep(b, c, x);
	return rise * fall;
}

inline float3 glow_envelope(float x,
							float a, float b, float c,
							float3 color,
							float thickness,
							float glow_radius,
							float intensity)
{
	float rise = smoothstep(a, b, x);
	float fall = 1.0 - smoothstep(b, c, x);
	float mask = rise * fall;

	float d = abs(x - b);

	float core = smoothstep(thickness, 0.0, d);
	float mid = exp(-3.0 * d / glow_radius);
	float outer = exp(-1.0 * d / (2.0 * glow_radius));

	float3 result =
		color * core * 1.0 * mask +
		color * mid * 0.6 * mask +
		color * outer * 0.3 * mask;

	return result * intensity;
}
