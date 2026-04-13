#pragma once

static inline float2 noise_hash2(float2 p, float2 seed)
{
	float3 p3 = fract(float3(p.x, p.y, dot(p, float2(0.5f, 0.25f))) +
					  float3(seed.x, seed.y, seed.x * 1.37f + seed.y * 2.11f));

	p3 += dot(p3, float3(p3.y, p3.z, p3.x) + 19.19f);
	float2 h = fract(float2((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y));
	return h * 2.0f - 1.0f;
}

static inline float noise_fade(float t)
{
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static inline float noise_perlin_scalar(float2 p, float2 seed)
{
	float2 i = floor(p);
	float2 f = p - i;

	float2 g00 = normalize(noise_hash2(i + float2(0.0f, 0.0f), seed));
	float2 g10 = normalize(noise_hash2(i + float2(1.0f, 0.0f), seed));
	float2 g01 = normalize(noise_hash2(i + float2(0.0f, 1.0f), seed));
	float2 g11 = normalize(noise_hash2(i + float2(1.0f, 1.0f), seed));

	float2 d00 = f - float2(0.0f, 0.0f);
	float2 d10 = f - float2(1.0f, 0.0f);
	float2 d01 = f - float2(0.0f, 1.0f);
	float2 d11 = f - float2(1.0f, 1.0f);

	float n00 = dot(g00, d00);
	float n10 = dot(g10, d10);
	float n01 = dot(g01, d01);
	float n11 = dot(g11, d11);

	float2 u = float2(noise_fade(f.x), noise_fade(f.y));
	float nx0 = mix(n00, n10, u.x);
	float nx1 = mix(n01, n11, u.x);
	float nxy = mix(nx0, nx1, u.y);
	return nxy;
}

static inline float2 noise_perlin_vec2(float2 p, float2 seed)
{
	float n1 = noise_perlin_scalar(p, seed + float2(13.27f, 7.11f));
	float n2 = noise_perlin_scalar(p + float2(1.37f, 9.79f), seed + float2(5.33f, 17.71f));
	return float2(n1, n2);
}

static inline float2 noise_perlin_fbm_vec2(float2 p, float2 seed, int octaves, float lacunarity, float gain)
{
	float2 sum = float2(0.0f);
	float amp = 1.0f;
	float2 q = p;
	for (int i = 0; i < octaves; ++i)
	{
		sum += noise_perlin_vec2(q, seed) * amp * 0.5f;
		q *= lacunarity;
		amp *= gain;
	}
	return sum;
}

static inline float noise_rand(float2 p, float2 seed)
{
	float3 q = fract(float3(dot(p, float2(127.1f, 311.7f)) + dot(seed, float2(269.5f, 183.3f)),
							dot(p, float2(269.5f, 183.3f)) + dot(seed, float2(127.1f, 311.7f)),
							dot(p, float2(419.2f, 371.9f)) + dot(seed, float2(419.2f, 371.9f))));
	q += dot(q, float3(q.y, q.z, q.x) + 19.19f);
	return fract((q.x + q.y) * q.z);
}

static inline float noise_value(float2 p, float2 seed)
{
	float2 i = floor(p);
	float2 f = p - i;

	float v00 = noise_rand(i + float2(0.0f, 0.0f), seed);
	float v10 = noise_rand(i + float2(1.0f, 0.0f), seed);
	float v01 = noise_rand(i + float2(0.0f, 1.0f), seed);
	float v11 = noise_rand(i + float2(1.0f, 1.0f), seed);

	float2 u = float2(noise_fade(f.x), noise_fade(f.y));
	float vx0 = mix(v00, v10, u.x);
	float vx1 = mix(v01, v11, u.x);
	float vxy = mix(vx0, vx1, u.y);
	return vxy * 2.0f - 1.0f;
}

static inline float noise_worley(float2 p, float2 seed)
{
	float2 i = floor(p);
	float min_d2 = 1e9f;
	for (int y = -1; y <= 1; ++y)
	{
		for (int x = -1; x <= 1; ++x)
		{
			float2 cell = i + float2(float(x), float(y));
			float2 h = noise_hash2(cell, seed);
			float2 feature = cell + 0.5f + 0.5f * h;
			float2 d = p - feature;
			float d2 = dot(d, d);
			min_d2 = min(min_d2, d2);
		}
	}
	float d = sqrt(min_d2);
	return 1.0f - clamp(d, 0.0f, 1.0f);
}
