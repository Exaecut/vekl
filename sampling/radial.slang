implementing vekl;

// Radial / angular sampling primitives for sweep blurs (zoom & spin).
//
// Two complementary parametric paths around a center point:
//
//   AngularSampling - rotate the offset (uv - center) around `center` across
//                     an arc of `angle` radians. Produces a "spin" / rotational
//                     blur. Sample i in [0..N) maps to angle theta in
//                     [-angle/2, +angle/2] (re-mapped by `direction`).
//
//   RadialSampling  - scale the offset (uv - center) along the line uv-center.
//                     Produces a "zoom" blur. Sample i maps to a scale factor s
//                     such that the sampled point is `center + (uv - center) * s`.
//                     `distance` controls how far the trail extends (with
//                     `distance > 0` reaching outward, `< 0` reaching inward).
//
// Both share a single options struct so call sites stay short and effects can
// expose a uniform set of knobs:
//
//   * sampleCount    : >= 2, integer sample count.
//   * direction      : signed [-1..+1]; controls trail bias.
//                      0   = symmetric (center the trail on uv).
//                      +1  = trail reaches forward only (uv .. uv+full).
//                      -1  = trail reaches backward only (uv-full .. uv).
//   * distribution   : sample positioning curve along the trail
//                      (linear / exponential / gaussian).
//   * jitter         : [0..1] random perturbation of each sample's t to break
//                      banding on low sample counts.
//   * anisotropy     : per-axis scale of the offset before the rotation/scale,
//                      applied around `center`. (1,1) preserves circular shape;
//                      use (W/H, 1) to compensate non-square pixel aspect.
//   * weightExponent : sample weight = pow(falloff(t), weightExponent). 1 is
//                      the natural distribution weight, > 1 emphasizes the
//                      anchor sample and suppresses the trail tail.
//
// The trail "anchor" is always uv (i.e. the pixel being shaded). t = 0 means
// the sample sits on uv, t = 1 means it sits at the far end of the trail.
// `direction = 0` lays out t in [-1, +1] symmetrically; nonzero direction
// biases the trail to one side.
//
// Sampling is stateless: given the params and an index `i in [0..N)`, you
// get the sample uv + a normalized weight. Callers handle texture sampling
// and accumulation (see `RadialAccumulator` in `color/accumulate.slang`
// for an alpha-correct linear-light premultiplied integrator).

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

/// Result of a single sample query: uv to sample + post-weight.
public struct SweepSample
{
    public float2 uv;
    public float  weight;
}

// --- Internal helpers -------------------------------------------------------

internal float Hash11(float x)
{
    return frac(sin(x * 12.9898) * 43758.5453);
}

/// Map index i in [0..N) to a parametric `t` in [-1, +1], biased by direction.
/// direction= 0: t in [-1, +1], midpoint 0
/// direction=+1: t in [0, +1]
/// direction=-1: t in [-1, 0]
internal float SampleParam(uint i, uint n, float direction, float jitter, uint salt)
{
    float u = (n > 1u) ? float(i) / float(n - 1u) : 0.5;       // [0, 1]
    if (jitter > 0.0)
    {
        // Deterministic per-sample jitter; salt keeps separate sweeps decorrelated.
        float h = Hash11(float(i) * 1.61803 + float(salt) * 0.7071);
        float du = (h - 0.5) * (1.0 / max(float(n - 1u), 1.0));
        u = clamp(u + du * jitter, 0.0, 1.0);
    }
    // u in [0,1] -> base t in [-1, +1]
    float t = u * 2.0 - 1.0;
    // Bias toward `direction`: lerp(centered, [0..1] mapped to direction sign, |dir|)
    float biased = (direction >= 0.0) ? u : -u;
    return lerp(t, biased, clamp(abs(direction), 0.0, 1.0));
}

/// Map [-1,+1] linear t to a re-distributed t' in [-1,+1] per `distribution`.
/// Preserves sign so direction bias is honored. All curves are concentrating
/// (more samples near t=0); the trail anchor is the pixel itself, so denser
/// sampling near the anchor reduces banding around the pixel of interest.
internal float Redistribute(float t, SampleDistribution dist)
{
    float s  = sign(t);
    float at = abs(t);
    switch (dist)
    {
        case SampleDistribution.Exponential:
            // |t'| = |t|^2 - cluster strongly near 0.
            return s * at * at;
        case SampleDistribution.Gaussian:
            // |t'| = 1 - cos(pi/2 * |t|) - cluster near 0 with a smoother
            // shoulder than Exponential. Cheap (one cosine).
            return s * (1.0 - cos(1.5707963 * at));
        default:
            return t;
    }
}

/// Per-distribution natural weight for a sample at [-1,+1] t.
/// All weights are >=0 and the caller normalizes by the running sum.
internal float NaturalWeight(float t, SampleDistribution dist)
{
    switch (dist)
    {
        case SampleDistribution.Exponential:
            return 1.0 / max(1.0 + 4.0 * t * t, 1e-3);
        case SampleDistribution.Gaussian:
            // sigma=0.4 -> e^(-t^2 / (2*0.16))
            return exp(-(t * t) * 3.125);
        default:
            return 1.0;  // Box / linear distribution: equal weights
    }
}

// --- Public sampling API ----------------------------------------------------

/// Compute the i-th sample for an *angular* sweep around `center`, rotating
/// the offset (uv - center) by an angle in [-angle/2, +angle/2] (modulated by
/// direction).
///
/// `angle` is the full sweep arc in radians.
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

    // Anisotropic frame around center.
    float2 d = (uv - center) / max(opts.anisotropy, float2(1e-6, 1e-6));
    float c = cos(theta);
    float s = sin(theta);
    float2 rot = float2(c * d.x - s * d.y, s * d.x + c * d.y);

    SweepSample r;
    r.uv     = center + rot * opts.anisotropy;
    r.weight = pow(NaturalWeight(td, opts.distribution), max(opts.weightExponent, 0.0));
    return r;
}

/// Compute the i-th sample for a *radial* (zoom) sweep around `center`,
/// scaling the offset (uv - center) by a factor `1 + t*distance`, where t is
/// the distribution-shaped parameter in [-1, +1] (modulated by direction).
///
/// `distance` is the signed peak displacement as a fraction of the offset
/// length (e.g. 0.1 means the far end of the trail sits 10 % further out).
/// Negative `distance` reverses the trail direction.
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

/// Pixel aspect helper: returns an anisotropy vector that compensates a
/// non-square texture so the sweep traces a perceptual circle in screen
/// space. Multiply your `opts.anisotropy` by this if the input is non-square.
public float2 PixelAspectAnisotropy(uint2 sizePx)
{
    if (sizePx.y == 0u) return float2(1.0, 1.0);
    float a = float(sizePx.x) / float(sizePx.y);
    return (a > 1.0) ? float2(1.0, a) : float2(1.0 / a, 1.0);
}
