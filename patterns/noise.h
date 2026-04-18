#pragma once

static inline float2 noise_hash2(float2 p, float2 seed)
{
    float2 q = float2(
        dot(p + seed, float2(127.1f, 311.7f)),
        dot(p + seed, float2(269.5f, 183.3f))
    );

    float2 s = sin(q) * 43758.5453f;
    return fract(s) * 2.0f - 1.0f;
}

static inline float2 noise_grad(float2 p, float2 seed)
{
    float2 h = noise_hash2(p, seed);
    float inv = rsqrt(h.x*h.x + h.y*h.y + 1e-6f);
    return h * inv;
}

static inline float noise_fade(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static inline float noise_perlin_scalar(float2 p, float2 seed)
{
    float2 i = floor(p);
    float2 f = p - i;

    float2 g00 = noise_grad(i + float2(0.0f, 0.0f), seed);
    float2 g10 = noise_grad(i + float2(1.0f, 0.0f), seed);
    float2 g01 = noise_grad(i + float2(0.0f, 1.0f), seed);
    float2 g11 = noise_grad(i + float2(1.0f, 1.0f), seed);

    float n00 = dot(g00, f - float2(0.0f, 0.0f));
    float n10 = dot(g10, f - float2(1.0f, 0.0f));
    float n01 = dot(g01, f - float2(0.0f, 1.0f));
    float n11 = dot(g11, f - float2(1.0f, 1.0f));

    float2 u = float2(noise_fade(f.x), noise_fade(f.y));

    float nx0 = mix(n00, n10, u.x);
    float nx1 = mix(n01, n11, u.x);

    return mix(nx0, nx1, u.y);
}

static inline float2 noise_perlin_vec2(float2 p, float2 seed)
{
    return float2(
        noise_perlin_scalar(p, seed + float2(13.1f, 7.7f)),
        noise_perlin_scalar(p + float2(5.2f, 1.3f), seed + float2(3.3f, 9.9f))
    );
}

static inline float2 noise_perlin_fbm_vec2(float2 p, float2 seed, int octaves, float lacunarity, float gain)
{
    float2 sum = float2(0.0f);
    float amp = 1.0f;
    float2 q = p;

    for (int i = 0; i < octaves; ++i)
    {
        sum += noise_perlin_vec2(q, seed) * amp;
        q *= lacunarity;
        amp *= gain;
    }

    return sum;
}