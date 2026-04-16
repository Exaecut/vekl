#pragma once

inline float3 opGlowOutline(float d, float3 glowColor, float thickness)
{
	float dist = abs(d);
	float x = clamp(1.0f - dist / thickness, 0.0f, 1.0f);
	float intensity = pow(x, 2.0f);

	return glowColor * intensity;
}

inline float3 opNeonGlow(float d, float3 color, float thickness, float glowRadius)
{
	float dist = abs(d);
	float core = smoothstep(thickness, 0.0f, dist);
	float mid = exp(-3.0f * dist / glowRadius);
	float outer = exp(-1.0f * dist / (2.0f * glowRadius));

	float3 result =
		color * core * 1.0f
		+ color * mid * 0.6f
		+ color * outer * 0.3f;

	return result;
}
