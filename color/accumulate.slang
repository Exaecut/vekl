implementing vekl;

// Color-property-preserving accumulators for multi-tap effects (radial blur,
// motion blur, bokeh).
//
// Naive `sum_rgba += sample; sum_w += weight` strips color two ways:
//   * Alpha-edge darkening — averaging straight RGBA pulls semi-transparent
//     pixels' RGB toward zero. Accumulate premultiplied (rgb*a, a) and divide
//     by the summed weight (NOT alpha) to get a perceptually correct mix.
//   * Chroma washout — a weighted average of N pixels with similar luma but
//     slightly different hues desaturates the middle. Track mean per-sample
//     saturation and re-saturate at the end to recover lost chroma without
//     altering hue.
//
// `SweepAccumulator` is sampling-shape agnostic; consume `(RGBA, weight)` from
// any source and call `Resolve(restoreSaturation)` (0 = naive premultiplied
// mean, 1 = full chroma restore).

public struct SweepAccumulator
{
    /// Sum of premultiplied RGBA: (rgb * a * w, a * w).
    public float4 sumPremul;
    public float  sumW;
    /// Sum of per-sample saturation × weight; `Resolve()` uses it to restore chroma.
    public float  sumSatW;

    public static SweepAccumulator Create()
    {
        SweepAccumulator a;
        a.sumPremul = float4(0.0, 0.0, 0.0, 0.0);
        a.sumW      = 0.0;
        a.sumSatW   = 0.0;
        return a;
    }

    /// Add a sample. `px` is straight (un-premultiplied) RGBA; `weight >= 0`.
    [mutating]
    public void Add(float4 px, float weight)
    {
        float w = max(weight, 0.0);
        sumPremul += float4(px.rgb * px.a, px.a) * w;
        sumW      += w;
        sumSatW   += SampleSaturation(px.rgb) * w;
    }

    /// Resolve to a final straight RGBA. `restoreSaturation` in [0, 1]:
    /// 0 = raw averaged RGB, 1 = chroma-restored to the weighted mean per-sample saturation.
    public float4 Resolve(float restoreSaturation)
    {
        float invW = (sumW > 1e-6) ? 1.0 / sumW : 0.0;
        float4 premul = sumPremul * invW;            
        float  alpha  = premul.a;
        float3 rgb    = (alpha > 1e-5) ? premul.rgb / alpha : float3(0.0, 0.0, 0.0);

        float meanSat   = sumSatW * invW;
        float resultSat = SampleSaturation(rgb);
        float boost     = (resultSat > 1e-5) ? meanSat / resultSat : 1.0;
        // Cap chroma boost at 2× to avoid runaway when two near-equal-luma complementary hues average to grey.
        boost = clamp(boost, 1.0, 2.0);

        float3 luma = float3(SampleLuma(rgb), SampleLuma(rgb), SampleLuma(rgb));
        float3 boosted = lerp(rgb, luma + (rgb - luma) * boost, clamp(restoreSaturation, 0.0, 1.0));
        return float4(boosted, alpha);
    }
}


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
