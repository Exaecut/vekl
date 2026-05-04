implementing vekl;

public float2 FbmNoise(float2 p, float2 seed, int octaves, float lacunarity, float gain)
{
    float2 sum = float2(0.0, 0.0);
    float amp = 1.0;
    float2 q = p;

    for (int i = 0; i < octaves; ++i)
    {
        sum += PerlinNoise2(q, seed) * amp;
        q *= lacunarity;
        amp *= gain;
    }

    return sum;
}
