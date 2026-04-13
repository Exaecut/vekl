#pragma once

inline float3 opGlowOutline(float d, float3 glowColor, float thickness)
{
	float dist = abs(d);
	float x = clamp(1.0 - dist / thickness, 0.0, 1.0);
	float intensity = pow(x, 2.0);

	return glowColor * intensity;
}
inline float3 opNeonGlow(float d, float3 color, float thickness, float glowRadius)
{
	float dist = abs(d);
	float core = smoothstep(thickness, 0.0, dist);
	float mid = exp(-3.0 * dist / glowRadius);
	float outer = exp(-1.0 * dist / (2.0 * glowRadius));

	float3 result =
		color * core * 1.0
		+ color * mid * 0.6
		+ color * outer * 0.3;

	return result;
}


