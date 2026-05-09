implementing vekl;

// Cheap hash-based value noise. Pairs well with analog / glitch styles
// (VHS, CRT, film grain) where speckled, slightly aliased noise is
// desirable. Prefer [`PerlinNoise`] (smooth gradient) when aliasing would
// distract — Perlin is ~2× more expensive but free of the "sin-fract"
// streakiness near cardinal angles.

/// Scalar hash from a 2D coordinate in `[0, 1)`. Deterministic, cheap,
/// and good enough for stylistic noise. Not suitable for Monte-Carlo
/// sampling or cryptography.
public float Hash21(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}

/// Smoothstep-interpolated value noise on an integer lattice. `lattice`
/// controls the cell grid (e.g. `float2(8.0, 8.0)` = 8×8 cells across
/// `p ∈ [0, 1)`). Higher → finer detail; use [`HashFbm`] to stack
/// multiple scales.
public float HashValueNoise(float2 p, float2 lattice)
{
    float2 pl = p * lattice;
    float2 pi = floor(pl);
    float2 ip = smoothstep(float2(0.0, 0.0), float2(1.0, 1.0), frac(pl));

    float h00 = Hash21(pi / lattice);
    float h10 = Hash21((pi + float2(1.0, 0.0)) / lattice);
    float h01 = Hash21((pi + float2(0.0, 1.0)) / lattice);
    float h11 = Hash21((pi + float2(1.0, 1.0)) / lattice);

    return lerp(lerp(h00, h10, ip.x), lerp(h01, h11, ip.x), ip.y);
}

/// Multi-octave hash value noise (fractional Brownian motion). `octaves`
/// copies of [`HashValueNoise`] are stacked at `lattice = 2 * 2^i` with
/// amplitude `1/2^i`. The output range is approximately `[0, 1]`.
public float HashFbm(float2 p, int octaves)
{
    float sum = 0.0;
    for (int i = 1; i <= octaves; ++i)
    {
        float fi = float(i);
        float pow2i = exp2(fi);
        sum += HashValueNoise(p + float2(fi, fi), float2(2.0 * pow2i, 2.0 * pow2i)) / pow2i;
    }
    return sum;
}
