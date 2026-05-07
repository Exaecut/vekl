implementing vekl;

// Color-property-preserving accumulators for multi-tap effects (radial blur,
// motion blur, bokeh, etc.).
//
// The naive `sum_rgba += sample; sum_w += weight` approach has two well-known
// failure modes that strip color from the result:
//
//   1. Alpha-edge darkening. Averaging straight RGBA pulls semi-transparent
//      pixels' RGB toward zero (because their RGB is "off" outside the alpha
//      mask). Accumulating *premultiplied* (rgb*a, a) and dividing by the
//      summed weight (NOT by alpha) produces a result that is itself
//      premultiplied; un-premultiplying gives the perceptually correct
//      mix.
//
//   2. Chroma washout. A weighted average of N pixels with similar luminance
//      but slightly different hues yields a desaturated middle. The mean
//      saturation across samples is *higher* than the saturation of the mean
//      RGB. We track the mean saturation and gently re-saturate at the end
//      to recover that lost chroma without altering hue.
//
// `SweepAccumulator` is sampling-shape agnostic - it consumes (RGBA, weight)
// tuples and produces a chroma-correct mean, regardless of whether the source
// samples come from `AngularSample`, `RadialSample`, a linear path, a Gaussian
// blur, or any other multi-tap operator. Use:
//
//     SweepAccumulator acc = SweepAccumulator.Create();
//     for (uint i = 0; i < N; ++i) {
//         SweepSample s = AngularSample(uv, center, angle, i, opts);
//         float4 px = src.SampleLinear(saturate(s.uv));
//         acc.Add(px, s.weight);
//     }
//     float4 result = acc.Resolve(/*restoreSaturation*/ 1.0);
//
// `restoreSaturation` in [0..1]:
//   0 = naive premultiplied mean (no chroma boost)
//   1 = full chroma restore (target mean per-sample saturation)

public struct SweepAccumulator
{
    /// Sum of premultiplied RGBA: (rgb * a * w, a * w)
    public float4 sumPremul;
    /// Sum of weights.
    public float  sumW;
    /// Sum of per-sample saturation * weight. Used by Resolve() to restore.
    public float  sumSatW;

    public static SweepAccumulator Create()
    {
        SweepAccumulator a;
        a.sumPremul = float4(0.0, 0.0, 0.0, 0.0);
        a.sumW      = 0.0;
        a.sumSatW   = 0.0;
        return a;
    }

    /// Add a sample. `px` is straight (un-premultiplied) RGBA from a texture
    /// fetch; `weight` >= 0.
    [mutating]
    public void Add(float4 px, float weight)
    {
        float w = max(weight, 0.0);
        sumPremul += float4(px.rgb * px.a, px.a) * w;
        sumW      += w;
        sumSatW   += SampleSaturation(px.rgb) * w;
    }

    /// Resolve to a final straight RGBA. `restoreSaturation` in [0,1] mixes
    /// between the raw averaged RGB (0) and a chroma-restored RGB (1) that
    /// targets the weighted mean per-sample saturation.
    public float4 Resolve(float restoreSaturation)
    {
        float invW = (sumW > 1e-6) ? 1.0 / sumW : 0.0;
        float4 premul = sumPremul * invW;            // premultiplied mean
        float  alpha  = premul.a;
        float3 rgb    = (alpha > 1e-5) ? premul.rgb / alpha : float3(0.0, 0.0, 0.0);

        float meanSat   = sumSatW * invW;
        float resultSat = SampleSaturation(rgb);
        float boost     = (resultSat > 1e-5) ? meanSat / resultSat : 1.0;
        // Clamp so we never over-pump (1.0 means "match", we cap at 2x to
        // avoid runaway when two near-equal-luma complementary hues average
        // to a near-grey).
        boost = clamp(boost, 1.0, 2.0);

        float3 luma = float3(SampleLuma(rgb), SampleLuma(rgb), SampleLuma(rgb));
        float3 boosted = lerp(rgb, luma + (rgb - luma) * boost, clamp(restoreSaturation, 0.0, 1.0));
        return float4(boosted, alpha);
    }
}

// --- Lightweight luma / saturation helpers (Rec.709) ------------------------

internal float SampleLuma(float3 c)
{
    return dot(c, float3(0.2126, 0.7152, 0.0722));
}

internal float SampleSaturation(float3 c)
{
    float mn = min(min(c.r, c.g), c.b);
    float mx = max(max(c.r, c.g), c.b);
    return (mx > 1e-5) ? (mx - mn) / mx : 0.0;
}
