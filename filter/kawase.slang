implementing vekl;

// Dual-Kawase pyramid filters (Marius Bjørge, ARM — "Bandwidth-Efficient
// Rendering", SIGGRAPH 2015). A down/up pyramid whose effective blur radius
// grows with pyramid depth at a fixed per-pass cost of 5 taps (down) / 8 taps
// (up), independent of the effective radius. For wide, soft blur this beats a
// separable Gaussian, whose cost scales linearly with radius. UV is in
// source-level coordinates; `lod` is the source mip index.
//
// Unlike the Karis bloom pyramid (`karis.slang`), the upsample REPLACES rather
// than accumulates: a down chain followed by an up chain reconstructs a blurred
// copy of level 0, with blur strength set by how many levels the chain spans.

// 5-tap downsample: centre tap weighted ×4 plus four diagonal corner taps at a
// half-source-texel, normalised by 8. Bilinear oversampling folds each corner
// into a 2×2 box, so the 5 fetches cover a 4×4 footprint.
public float4 KawaseDownsample(TextureView src, float2 uv, uint lod)
{
    uint2 srcSize = src.Size(lod);
    float2 hp = 0.5 / float2(srcSize);

    float4 sum = src.SampleLinear(uv, lod) * 4.0;
    sum += src.SampleLinear(uv + float2(-hp.x, -hp.y), lod);
    sum += src.SampleLinear(uv + float2( hp.x, -hp.y), lod);
    sum += src.SampleLinear(uv + float2(-hp.x,  hp.y), lod);
    sum += src.SampleLinear(uv + float2( hp.x,  hp.y), lod);
    return sum * (1.0 / 8.0);
}

// 8-tap tent upsample: four edge taps (×1) and four corner taps (×2) on a
// one-source-texel ring scaled by `radius`, normalised by 12. `radius` 1.0 is
// the canonical kernel; larger softens, smaller tightens.
public float4 KawaseUpsample(TextureView src, float2 uv, uint lod, float radius)
{
    uint2 srcSize = src.Size(lod);
    float2 o = (1.0 / float2(srcSize)) * max(radius, 1e-3);

    float4 sum = src.SampleLinear(uv + float2(-o.x * 2.0, 0.0), lod);
    sum += src.SampleLinear(uv + float2(-o.x,  o.y), lod) * 2.0;
    sum += src.SampleLinear(uv + float2( 0.0,  o.y * 2.0), lod);
    sum += src.SampleLinear(uv + float2( o.x,  o.y), lod) * 2.0;
    sum += src.SampleLinear(uv + float2( o.x * 2.0, 0.0), lod);
    sum += src.SampleLinear(uv + float2( o.x, -o.y), lod) * 2.0;
    sum += src.SampleLinear(uv + float2( 0.0, -o.y * 2.0), lod);
    sum += src.SampleLinear(uv + float2(-o.x, -o.y), lod) * 2.0;
    return sum * (1.0 / 12.0);
}
