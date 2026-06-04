implementing vekl;

// Stateless angular / radial sampling for sweep blurs (zoom & spin). Both
// rotate or scale the offset `(uv - center)` around `center`; for sample
// `i ∈ [0..N)` they return a `SweepSample { uv, weight }` and the caller
// handles texture fetch and accumulation (see `SweepAccumulator`).
//
// `SweepOptions` knobs:
//   * sampleCount     - >= 2.
//   * direction       - signed [-1, 1] trail bias (0 = symmetric).
//   * distribution    - linear / exponential / gaussian.
//   * jitter          - [0, 1] perturbation to break banding.
//   * anisotropy      - per-axis scale of the offset before rotate/scale.
//   * weightExponent  - `pow(naturalWeight, exp)`; > 1 emphasises the anchor.

public enum SampleDistribution : uint
{
    Linear      = 0,
    Exponential = 1,
    Gaussian    = 2,
}

/// Per-instance options shared by Angular & Radial sampling.
public struct SweepOptions
{
    /// Number of samples >= 2.
    public uint sampleCount;
    /// Trail directional bias in [-1, +1]. 0 = symmetric.
    public float direction;
    /// Sample positioning curve.
    public SampleDistribution distribution;
    /// Jitter strength in [0..1]. 0 = deterministic.
    public float jitter;
    /// Per-axis anisotropy applied to the offset (1,1) = circular.
    public float2 anisotropy;
    /// Weight curve exponent; 1.0 = natural distribution weight.
    public float weightExponent;

    /// Convenience factory: deterministic, linear distribution, symmetric.
    public static SweepOptions Default(uint sampleCount)
    {
        SweepOptions o;
        o.sampleCount    = max(sampleCount, 2u);
        o.direction      = 0.0;
        o.distribution   = SampleDistribution.Linear;
        o.jitter         = 0.0;
        o.anisotropy     = float2(1.0, 1.0);
        o.weightExponent = 1.0;
        return o;
    }
}

/// Result of a single sample query: `uv` + post-weight.
public struct SweepSample
{
    public float2 uv;
    public float  weight;
}


internal float Hash11(float x)
{
    return frac(sin(x * 12.9898) * 43758.5453);
}

/// Map index `i ∈ [0..N)` to `t ∈ [-1, +1]`, biased by `direction` (`0` = symmetric, `±1` = one-sided).
internal float SampleParam(uint i, uint n, float direction, float jitter, uint salt)
{
    float u = (n > 1u) ? float(i) / float(n - 1u) : 0.5;       
    if (jitter > 0.0)
    {
        // Deterministic per-sample jitter; salt decorrelates separate sweeps.
        float h = Hash11(float(i) * 1.61803 + float(salt) * 0.7071);
        float du = (h - 0.5) * (1.0 / max(float(n - 1u), 1.0));
        u = clamp(u + du * jitter, 0.0, 1.0);
    }
    float t = u * 2.0 - 1.0;
    // Bias toward `direction`: lerp between centred t and a [0, 1]-mapped one-sided t.
    float biased = (direction >= 0.0) ? u : -u;
    return lerp(t, biased, clamp(abs(direction), 0.0, 1.0));
}

/// Map linear `t ∈ [-1, +1]` through the chosen distribution. Sign-preserving,
/// concentrates near `t = 0` (the trail anchor) so banding around the pixel of
/// interest is reduced.
internal float Redistribute(float t, SampleDistribution dist)
{
    float s  = sign(t);
    float at = abs(t);
    switch (dist)
    {
        case SampleDistribution.Exponential:
            // |t'| = |t|^2 — strong cluster near 0.
            return s * at * at;
        case SampleDistribution.Gaussian:
            // |t'| = 1 - cos(π/2 |t|) — smoother shoulder than Exponential, single cosine.
            return s * (1.0 - cos(1.5707963 * at));
        default:
            return t;
    }
}

/// Per-distribution natural weight at `t`. Caller normalizes by the running sum.
public float NaturalWeight(float t, SampleDistribution dist)
{
    switch (dist)
    {
        case SampleDistribution.Exponential:
            return 1.0 / max(1.0 + 4.0 * t * t, 1e-3);
        case SampleDistribution.Gaussian:
            // σ = 0.4 → e^(-t² / (2 · 0.16)).
            return exp(-(t * t) * 3.125);
        default:
            return 1.0;  
    }
}


/// Compute the i-th sample for an *angular* sweep around `center`, rotating
/// `(uv - center)` by an angle in `[-angle/2, +angle/2]` (modulated by direction).
public SweepSample AngularSample(
    float2 uv,
    float2 center,
    float  angle,
    uint   index,
    SweepOptions opts)
{
    float t  = SampleParam(index, opts.sampleCount, opts.direction, opts.jitter, 0xA5u);
    float td = Redistribute(t, opts.distribution);

    float theta = td * (0.5 * angle);

    float2 d = (uv - center) / max(opts.anisotropy, float2(1e-6, 1e-6));
    float c = cos(theta);
    float s = sin(theta);
    float2 rot = float2(c * d.x - s * d.y, s * d.x + c * d.y);

    SweepSample r;
    r.uv     = center + rot * opts.anisotropy;
    r.weight = pow(NaturalWeight(td, opts.distribution), max(opts.weightExponent, 0.0));
    return r;
}

/// Compute the i-th sample for a *radial* (zoom) sweep around `center`, scaling
/// `(uv - center)` by `1 + t · distance`. `distance` is the signed peak displacement
/// as a fraction of the offset length; negative reverses trail direction.
public SweepSample RadialSample(
    float2 uv,
    float2 center,
    float  distance,
    uint   index,
    SweepOptions opts)
{
    float t  = SampleParam(index, opts.sampleCount, opts.direction, opts.jitter, 0x5Au);
    float td = Redistribute(t, opts.distribution);

    float scale = 1.0 + td * distance;

    float2 d = (uv - center) / max(opts.anisotropy, float2(1e-6, 1e-6));
    float2 scaled = d * scale;

    SweepSample r;
    r.uv     = center + scaled * opts.anisotropy;
    r.weight = pow(NaturalWeight(td, opts.distribution), max(opts.weightExponent, 0.0));
    return r;
}

/// Anisotropy vector that compensates a non-square texture so the sweep traces a perceptual circle. Multiply your `opts.anisotropy` by it for non-square inputs.
public float2 PixelAspectAnisotropy(uint2 sizePx)
{
    if (sizePx.y == 0u) return float2(1.0, 1.0);
    float a = float(sizePx.x) / float(sizePx.y);
    return (a > 1.0) ? float2(1.0, a) : float2(1.0 / a, 1.0);
}

// Multi-tap sweeps that cover a per-pixel trail longer than the sample budget
// alias on level 0 (each tap skips source pixels). `PickSweepLod` selects a
// downsampled lod analytically; pair with `SampleLinearTrilinear(uv, lodF)` for
// hard-cut-free transitions. The chosen lod is per-pixel (not per-tap) so
// neighbouring taps agree on the source.

/// Pick a continuous mip lod for a sweep / multi-tap blur kernel.
///
/// `oversample` is taps per source pixel of trail at the chosen lod:
///   * `1.0` — Nyquist. Sharpest, runs the kernel at full sample budget on long trails.
///   * `4.0` — recommended real-time default. Smoother and ~4× cheaper than Nyquist
///     because the lod ramps `log2(4) = 2` levels earlier; detail still beats
///     fixed trail-px heuristics.
///   * `0.5` — undersampled. Sharper but mild aliasing; only when tap budget is known to exceed trail demand.
///
/// Formula: `lodF = clamp(log2(oversample · trailPx / sampleCount), 0, maxLod)`.
/// Pair with `TextureView.SampleLinearTrilinear(uv, lodF)`.
///
/// `trailPx` is the worst-case trail length in source pixels (lod-0 units).
/// `sampleCount <= 1` short-circuits to lod 0; `maxLod = 0` disables mip selection.
public float PickSweepLod(float trailPx, uint sampleCount, uint maxLod, float oversample)
{
    if (sampleCount <= 1u || maxLod == 0u || !(trailPx > 0.0) || !(oversample > 0.0))
        return 0.0;
    float lodF = log2(oversample * trailPx / float(sampleCount));
    return clamp(lodF, 0.0, float(maxLod));
}

/// Convenience overload: Nyquist (`oversample = 1.0`). Most callers should prefer the 4-arg overload with `oversample = 4.0`.
public float PickSweepLod(float trailPx, uint sampleCount, uint maxLod)
{
    return PickSweepLod(trailPx, sampleCount, maxLod, 1.0);
}
