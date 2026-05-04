implementing vekl;

// --- Internal helpers ---

float2 Hash2(float2 p, float2 seed)
{
    float2 q = float2(
        dot(p + seed, float2(127.1, 311.7)),
        dot(p + seed, float2(269.5, 183.3))
    );
    float2 s = sin(q) * 43758.5453;
    return frac(s) * 2.0 - 1.0;
}

float2 Grad(float2 p, float2 seed)
{
    float2 h = Hash2(p, seed);
    float inv = rsqrt(h.x * h.x + h.y * h.y + 1e-6);
    return h * inv;
}

float Fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float ScalarNoise(float2 p, float2 seed)
{
    float2 i = floor(p);
    float2 f = p - i;

    float2 g00 = Grad(i + float2(0.0, 0.0), seed);
    float2 g10 = Grad(i + float2(1.0, 0.0), seed);
    float2 g01 = Grad(i + float2(0.0, 1.0), seed);
    float2 g11 = Grad(i + float2(1.0, 1.0), seed);

    float n00 = dot(g00, f - float2(0.0, 0.0));
    float n10 = dot(g10, f - float2(1.0, 0.0));
    float n01 = dot(g01, f - float2(0.0, 1.0));
    float n11 = dot(g11, f - float2(1.0, 1.0));

    float2 u = float2(Fade(f.x), Fade(f.y));

    float nx0 = lerp(n00, n10, u.x);
    float nx1 = lerp(n01, n11, u.x);

    return lerp(nx0, nx1, u.y);
}

// --- Public API ---

public float PerlinNoise(float2 p, float2 seed)
{
    return ScalarNoise(p, seed + float2(13.1, 7.7));
}

public float2 PerlinNoise2(float2 p, float2 seed)
{
    return float2(
        ScalarNoise(p, seed + float2(13.1, 7.7)),
        ScalarNoise(p + float2(5.2, 1.3), seed + float2(3.3, 9.9))
    );
}
