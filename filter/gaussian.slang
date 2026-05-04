implementing vekl;

public float GaussianWeight1d(int x, float sigma)
{
    float s2 = sigma * sigma;
    return exp(-(float(x * x)) / (2.0 * s2));
}

public float4 Gaussian1d(TextureView tex, float2 uv, float sigma, int radius, bool vertical)
{
    uint2 sizePx = tex.Size();
    float2 texelSize = 1.0 / float2(sizePx);
    float2 dir = vertical ? float2(0.0, 1.0) : float2(1.0, 0.0);

    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    float weightSum = 0.0;

    // Centre tap
    float wc = GaussianWeight1d(0, sigma);
    sum += tex.SampleLinear(uv) * wc;
    weightSum += wc;

    // Stride-2: each iteration covers two texel offsets (2i-1, 2i)
    const int half = radius / 2;
    for (int i = 1; i <= half; ++i)
    {
        float w_a = GaussianWeight1d(i * 2 - 1, sigma);
        float w_b = GaussianWeight1d(i * 2, sigma);
        float w_combined = w_a + w_b;

        float midpoint = float(i * 2 - 1) + w_b / max(w_combined, 1e-8);
        float2 offset = dir * (midpoint * texelSize);

        float4 c1 = tex.SampleLinear(uv + offset);
        float4 c2 = tex.SampleLinear(uv - offset);

        sum += (c1 + c2) * w_combined;
        weightSum += 2.0 * w_combined;
    }

    // If radius is odd, one texel remains uncovered
    if ((radius & 1) != 0 && radius > 0)
    {
        float w = GaussianWeight1d(radius, sigma);
        float2 offset = dir * (float(radius) * texelSize);

        float4 c1 = tex.SampleLinear(uv + offset);
        float4 c2 = tex.SampleLinear(uv - offset);

        sum += (c1 + c2) * w;
        weightSum += 2.0 * w;
    }

    return sum / max(weightSum, 1e-8);
}
