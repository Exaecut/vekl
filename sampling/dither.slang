implementing vekl;

// Triangular-PDF dither for low-bit-depth quantisation. ~1/(2^bits) of
// triangular noise added before the round breaks 8-bit banding without
// visible noise. Triangular > uniform: zero bias, perceptually closer to
// blue noise.

// Integer hash; replaces `frac(sin(...))` for stable Metal/CUDA/CPU output.
public float Dither_Hash11(uint n)
{
    n = (n << 13u) ^ n;
    n = n * (n * n * 15731u + 789221u) + 1376312589u;
    return float(n & 0x7fffffffu) / float(0x7fffffff);
}

// `frame` decorrelates the pattern across video frames.
public float Dither_Triangular(uint2 pixel, uint frame)
{
    uint seed = pixel.x * 1973u + pixel.y * 9277u + frame * 26699u;
    float a = Dither_Hash11(seed);
    float b = Dither_Hash11(seed + 0x9E3779B9u);
    return (a + b) - 1.0;
}

// Float storage skips the dither — no quantise step.
public float3 Dither(float3 rgb, uint2 pixel, uint frame, PixelStorage storage)
{
    if (storage == PixelStorage.Float32x4)
        return rgb;

    float bits = (storage == PixelStorage.Unorm16x4) ? 16.0 : 8.0;
    float amplitude = 1.0 / (exp2(bits) - 1.0);
    float n = Dither_Triangular(pixel, frame);
    return rgb + float3(n, n, n) * amplitude;
}
