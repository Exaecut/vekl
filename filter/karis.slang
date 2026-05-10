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
// neighbourhood); smaller tightens, larger softens.
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
