implementing vekl;

// Karis pyramid filters (Jimenez & Karis, COD: AW, SIGGRAPH 2014).
// 13-tap downsample (5 overlapping 2x2 box averages) + 9-tap tent upsample,
// both bilinearly oversampled. UV is in source-level coordinates; `lod` is
// the source mip index.

public float4 Karis13Downsample(TextureView src, float2 uv, uint lod)
{
    uint2 src_size = src.Size(lod);
    float2 texel = 1.0 / float2(src_size);

    float4 a = src.SampleLinear(uv + float2(-2, -2) * texel, lod);
    float4 b = src.SampleLinear(uv + float2( 0, -2) * texel, lod);
    float4 c = src.SampleLinear(uv + float2( 2, -2) * texel, lod);
    float4 d = src.SampleLinear(uv + float2(-2,  0) * texel, lod);
    float4 e = src.SampleLinear(uv + float2( 0,  0) * texel, lod);
    float4 f = src.SampleLinear(uv + float2( 2,  0) * texel, lod);
    float4 g = src.SampleLinear(uv + float2(-2,  2) * texel, lod);
    float4 h = src.SampleLinear(uv + float2( 0,  2) * texel, lod);
    float4 i = src.SampleLinear(uv + float2( 2,  2) * texel, lod);

    float4 j = src.SampleLinear(uv + float2(-1, -1) * texel, lod);
    float4 k = src.SampleLinear(uv + float2( 1, -1) * texel, lod);
    float4 l = src.SampleLinear(uv + float2(-1,  1) * texel, lod);
    float4 m = src.SampleLinear(uv + float2( 1,  1) * texel, lod);

    float4 box_inner = (j + k + l + m) * 0.25;
    float4 box_tl    = (a + b + d + e) * 0.25;
    float4 box_tr    = (b + c + e + f) * 0.25;
    float4 box_bl    = (d + e + g + h) * 0.25;
    float4 box_br    = (e + f + h + i) * 0.25;

    return box_inner * 0.5 + (box_tl + box_tr + box_bl + box_br) * 0.125;
}

// Per-box `1 / (1 + luma)` weighting suppresses single-pixel HDR spikes
// (energy-preserving across smooth highlights). Use only on the first
// downsample of a chain; later passes inherit a clean source.
public float4 Karis13DownsampleFirefly(TextureView src, float2 uv, uint lod)
{
    uint2 src_size = src.Size(lod);
    float2 texel = 1.0 / float2(src_size);

    float4 a = src.SampleLinear(uv + float2(-2, -2) * texel, lod);
    float4 b = src.SampleLinear(uv + float2( 0, -2) * texel, lod);
    float4 c = src.SampleLinear(uv + float2( 2, -2) * texel, lod);
    float4 d = src.SampleLinear(uv + float2(-2,  0) * texel, lod);
    float4 e = src.SampleLinear(uv + float2( 0,  0) * texel, lod);
    float4 f = src.SampleLinear(uv + float2( 2,  0) * texel, lod);
    float4 g = src.SampleLinear(uv + float2(-2,  2) * texel, lod);
    float4 h = src.SampleLinear(uv + float2( 0,  2) * texel, lod);
    float4 i = src.SampleLinear(uv + float2( 2,  2) * texel, lod);

    float4 j = src.SampleLinear(uv + float2(-1, -1) * texel, lod);
    float4 k = src.SampleLinear(uv + float2( 1, -1) * texel, lod);
    float4 l = src.SampleLinear(uv + float2(-1,  1) * texel, lod);
    float4 m = src.SampleLinear(uv + float2( 1,  1) * texel, lod);

    float4 box_inner = (j + k + l + m) * 0.25;
    float4 box_tl    = (a + b + d + e) * 0.25;
    float4 box_tr    = (b + c + e + f) * 0.25;
    float4 box_bl    = (d + e + g + h) * 0.25;
    float4 box_br    = (e + f + h + i) * 0.25;

    float W_inner = 0.5   / (1.0 + Luma709(box_inner.rgb));
    float W_tl    = 0.125 / (1.0 + Luma709(box_tl.rgb));
    float W_tr    = 0.125 / (1.0 + Luma709(box_tr.rgb));
    float W_bl    = 0.125 / (1.0 + Luma709(box_bl.rgb));
    float W_br    = 0.125 / (1.0 + Luma709(box_br.rgb));

    float4 num = box_inner * W_inner
               + box_tl    * W_tl
               + box_tr    * W_tr
               + box_bl    * W_bl
               + box_br    * W_br;
    float den = W_inner + W_tl + W_tr + W_bl + W_br;
    return num / max(den, 1e-6);
}

// `radius` is unitless: 1.0 = canonical Karis tent (one source-texel
// neighbourhood); smaller tightens, larger softens. Stays sharp up to
// ~1.5× radius. Above that the tap gaps exceed a source texel and the
// image starts to ring; use `GaussianUpsample21` for high-radius bloom.
public float4 Karis9Upsample(TextureView src, float2 uv, uint lod, float radius)
{
    uint2 src_size = src.Size(lod);
    float2 texel = (1.0 / float2(src_size)) * radius;

    float4 a = src.SampleLinear(uv + float2(-1, -1) * texel, lod);
    float4 b = src.SampleLinear(uv + float2( 0, -1) * texel, lod);
    float4 c = src.SampleLinear(uv + float2( 1, -1) * texel, lod);
    float4 d = src.SampleLinear(uv + float2(-1,  0) * texel, lod);
    float4 e = src.SampleLinear(uv + float2( 0,  0) * texel, lod);
    float4 f = src.SampleLinear(uv + float2( 1,  0) * texel, lod);
    float4 g = src.SampleLinear(uv + float2(-1,  1) * texel, lod);
    float4 h = src.SampleLinear(uv + float2( 0,  1) * texel, lod);
    float4 i = src.SampleLinear(uv + float2( 1,  1) * texel, lod);

    return e * 0.25
         + (b + d + f + h) * 0.125
         + (a + c + g + i) * 0.0625;
}

// 21-tap polar Gaussian upsample for large `radius` values (up to ~16
// source-texels). Three concentric tap groups, each angularly distributed
// to keep the chord-gap between neighbouring taps below 2 × radius texels:
//
//   centre              ×1   at  r = 0
//   inner ring (45°)    ×8   at  r = radius
//   outer ring (30°)    ×12  at  r = 2 × radius
//
// Weights are sampled from a separable Gaussian σ = radius (in source
// texels) at each tap's distance and re-normalised to 1, so the filter
// stays energy-preserving across the pyramid summation.
//
//   centre weight  : exp(0)                = 1.0
//   inner ring     : exp(-r²/(2σ²)) = e⁻⁰·⁵ ≈ 0.6065
//   outer ring     : exp(-4r²/(2σ²)) = e⁻²  ≈ 0.1353
//   normaliser     : 1 + 8·0.6065 + 12·0.1353 ≈ 7.4754
//
// The outer ring's angular gap is 2·sin(15°)·2r ≈ 1.04 r texels, vs the
// 13-tap variant's 2·sin(45°)·2r ≈ 2.83 r texels; halving the gap is
// what eliminates the "spokes" the older kernel produces above radius ~3.
public float4 GaussianUpsample21(TextureView src, float2 uv, uint lod, float radius)
{
    const float W_CENTRE = 1.0;
    const float W_INNER  = 0.60653066; // e^-0.5
    const float W_OUTER  = 0.13533528; // e^-2.0
    const float INV_SUM  = 1.0 / (W_CENTRE + 8.0 * W_INNER + 12.0 * W_OUTER);

    // Inner ring (8 taps, 45° apart). cos/sin precomputed.
    //   k=0: ( 1,  0)
    //   k=1: ( cos45,  sin45) ≈ (0.707, 0.707)
    //   k=2: ( 0,  1)
    //   k=3: (-cos45,  sin45)
    //   k=4: (-1,  0)
    //   k=5: (-cos45, -sin45)
    //   k=6: ( 0, -1)
    //   k=7: ( cos45, -sin45)
    const float C45 = 0.70710678;

    // Outer ring (12 taps, 30° apart). cos/sin at 0°, 30°, 60°, 90°, ...
    const float C30 = 0.86602540; // cos 30 = sin 60
    const float S30 = 0.50000000; // sin 30 = cos 60

    uint2 src_size = src.Size(lod);
    float r = max(radius, 1e-3);
    float2 t1 = (1.0 / float2(src_size)) * r;
    float2 t2 = t1 * 2.0;

    float4 c0 = src.SampleLinear(uv,                                       lod);

    float4 i0 = src.SampleLinear(uv + float2( 1.0,  0.0) * t1,             lod);
    float4 i1 = src.SampleLinear(uv + float2( C45,  C45) * t1,             lod);
    float4 i2 = src.SampleLinear(uv + float2( 0.0,  1.0) * t1,             lod);
    float4 i3 = src.SampleLinear(uv + float2(-C45,  C45) * t1,             lod);
    float4 i4 = src.SampleLinear(uv + float2(-1.0,  0.0) * t1,             lod);
    float4 i5 = src.SampleLinear(uv + float2(-C45, -C45) * t1,             lod);
    float4 i6 = src.SampleLinear(uv + float2( 0.0, -1.0) * t1,             lod);
    float4 i7 = src.SampleLinear(uv + float2( C45, -C45) * t1,             lod);

    float4 o0  = src.SampleLinear(uv + float2( 1.0,  0.0) * t2,            lod);
    float4 o1  = src.SampleLinear(uv + float2( C30,  S30) * t2,            lod);
    float4 o2  = src.SampleLinear(uv + float2( S30,  C30) * t2,            lod);
    float4 o3  = src.SampleLinear(uv + float2( 0.0,  1.0) * t2,            lod);
    float4 o4  = src.SampleLinear(uv + float2(-S30,  C30) * t2,            lod);
    float4 o5  = src.SampleLinear(uv + float2(-C30,  S30) * t2,            lod);
    float4 o6  = src.SampleLinear(uv + float2(-1.0,  0.0) * t2,            lod);
    float4 o7  = src.SampleLinear(uv + float2(-C30, -S30) * t2,            lod);
    float4 o8  = src.SampleLinear(uv + float2(-S30, -C30) * t2,            lod);
    float4 o9  = src.SampleLinear(uv + float2( 0.0, -1.0) * t2,            lod);
    float4 o10 = src.SampleLinear(uv + float2( S30, -C30) * t2,            lod);
    float4 o11 = src.SampleLinear(uv + float2( C30, -S30) * t2,            lod);

    float4 inner_sum = i0 + i1 + i2 + i3 + i4 + i5 + i6 + i7;
    float4 outer_sum = o0 + o1 + o2 + o3 + o4 + o5 + o6 + o7
                     + o8 + o9 + o10 + o11;

    float4 sum = c0 * W_CENTRE
               + inner_sum * W_INNER
               + outer_sum * W_OUTER;

    return sum * INV_SUM;
}

// 49-tap polar Gaussian upsample for Ultra-quality high-radius bloom.
// Four concentric tap groups, three angularly distributed to keep the
// chord-gap between neighbouring taps below 1.5 × radius source-texels
// even at radius 16:
//
//   centre              ×1   at  r = 0
//   inner ring (45°)    ×8   at  r = radius
//   middle ring (22.5°) ×16  at  r = 2 × radius
//   outer ring (15°)    ×24  at  r = 3 × radius
//
// Weights sampled from separable Gaussian σ = radius, re-normalised.
//   centre  : exp(0)                = 1.0
//   ring 1  : exp(-r²/(2σ²)) = e⁻⁰·⁵ ≈ 0.6065
//   ring 2  : exp(-4r²/(2σ²)) = e⁻²  ≈ 0.1353
//   ring 3  : exp(-9r²/(2σ²)) = e⁻⁴·⁵ ≈ 0.0111
public float4 GaussianUpsample49(TextureView src, float2 uv, uint lod, float radius)
{
    const float W_CENTRE = 1.0;
    const float W_R1     = 0.60653066; // e^-0.5
    const float W_R2     = 0.13533528; // e^-2.0
    const float W_R3     = 0.01110900; // e^-4.5
    const float INV_SUM  = 1.0 / (W_CENTRE + 8.0 * W_R1 + 16.0 * W_R2 + 24.0 * W_R3);

    uint2 src_size = src.Size(lod);
    float r = max(radius, 1e-3);
    float2 t1 = (1.0 / float2(src_size)) * r;
    float2 t2 = t1 * 2.0;
    float2 t3 = t1 * 3.0;

    float4 sum = src.SampleLinear(uv, lod) * W_CENTRE;

    // Ring 1: 8 taps at r, 45° apart
    {
        const float step = 6.283185307 / 8.0;
        [unroll]
        for (int i = 0; i < 8; i++)
        {
            float angle = float(i) * step;
            float2 offset = float2(cos(angle), sin(angle)) * t1;
            sum += src.SampleLinear(uv + offset, lod) * W_R1;
        }
    }

    // Ring 2: 16 taps at 2r, 22.5° apart
    {
        const float step = 6.283185307 / 16.0;
        [unroll]
        for (int i = 0; i < 16; i++)
        {
            float angle = float(i) * step;
            float2 offset = float2(cos(angle), sin(angle)) * t2;
            sum += src.SampleLinear(uv + offset, lod) * W_R2;
        }
    }

    // Ring 3: 24 taps at 3r, 15° apart
    {
        const float step = 6.283185307 / 24.0;
        [unroll]
        for (int i = 0; i < 24; i++)
        {
            float angle = float(i) * step;
            float2 offset = float2(cos(angle), sin(angle)) * t3;
            sum += src.SampleLinear(uv + offset, lod) * W_R3;
        }
    }

    return sum * INV_SUM;
}
