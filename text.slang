implementing vekl;

// In-shader text rendering against a single-channel SDF glyph atlas.
//
// The atlas and per-glyph metrics are plain `StructuredBuffer<uint>`s (the
// proven prgpu buffer convention): the atlas packs four SDF bytes per word
// (edge at 128), metrics pack eight floats per glyph. `DrawString` walks the
// packed character codes, and for the queried destination pixel finds the
// covering glyph, samples its SDF with bilinear filtering, and returns the
// composited colour. Pure per-pixel compute — no quads, no vertex/geometry.
//
// `TextDrawParams` is byte-identical to prgpu's Rust `TextOverlayParams`; the
// host fills it and dispatches over the text's bounding box only.

/// Per-glyph layout, eight floats, matching prgpu's Rust `GlyphMetric`.
/// `left`/`top` and `cell*`/`advance` are in atlas base-px; the host passes a
/// `scale = pxSize / basePx` so the same atlas draws at any size.
public struct GlyphMetric
{
    public float atlasX;
    public float atlasY;
    public float cellW;
    public float cellH;
    public float left;
    public float top;
    public float advance;
    public float pad;
}

/// One DrawString invocation's parameters. Byte-identical to the Rust
/// `TextOverlayParams`. `packed` holds up to 256 char codes, four per word.
public struct TextDrawParams
{
    public float color[4];     // straight RGBA text colour
    public float bgColor[4];   // full-width background band (straight RGBA, a=0 -> none)
    public float penX;         // baseline pen origin in dst pixels
    public float penY;
    public float scale;        // pxSize / atlas base-px
    public float spread;       // atlas SDF spread in base-px (edge softness)
    public uint  atlasW;
    public uint  atlasH;
    public uint  frameW;       // full destination dims (absolute bounds clamp)
    public uint  frameH;
    public uint  bboxX;        // dispatch bounding box in dst pixels
    public uint  bboxY;
    public uint  bboxW;
    public uint  bboxH;
    public uint  charCount;
    public uint  firstChar;    // first code point covered by the atlas
    public uint  glyphCount;   // number of glyph metrics
    public uint  pad0;
    public uint  packed[64];   // 256 char codes, 4 per word
}

GlyphMetric LoadGlyph(StructuredBuffer<uint> metrics, uint gi)
{
    uint b = gi * 8u;
    GlyphMetric g;
    g.atlasX  = asfloat(metrics[b + 0u]);
    g.atlasY  = asfloat(metrics[b + 1u]);
    g.cellW   = asfloat(metrics[b + 2u]);
    g.cellH   = asfloat(metrics[b + 3u]);
    g.left    = asfloat(metrics[b + 4u]);
    g.top     = asfloat(metrics[b + 5u]);
    g.advance = asfloat(metrics[b + 6u]);
    g.pad     = asfloat(metrics[b + 7u]);
    return g;
}

// One SDF texel as a 0..1 float, clamped to atlas bounds.
float AtlasTexel(StructuredBuffer<uint> atlas, uint atlasW, uint atlasH, int2 xy)
{
    int x = clamp(xy.x, 0, int(atlasW) - 1);
    int y = clamp(xy.y, 0, int(atlasH) - 1);
    uint idx = uint(y) * atlasW + uint(x);
    uint word = atlas[idx >> 2u];
    uint sh = (idx & 3u) * 8u;
    return float((word >> sh) & 0xFFu) / 255.0;
}

// Bilinear SDF sample at atlas-pixel coordinate `uv`.
float SampleAtlas(StructuredBuffer<uint> atlas, uint atlasW, uint atlasH, float2 uv)
{
    float2 p = uv - 0.5;
    int2 ip = int2(floor(p));
    float2 f = p - float2(ip);
    float s00 = AtlasTexel(atlas, atlasW, atlasH, ip + int2(0, 0));
    float s10 = AtlasTexel(atlas, atlasW, atlasH, ip + int2(1, 0));
    float s01 = AtlasTexel(atlas, atlasW, atlasH, ip + int2(0, 1));
    float s11 = AtlasTexel(atlas, atlasW, atlasH, ip + int2(1, 1));
    return lerp(lerp(s00, s10, f.x), lerp(s01, s11, f.x), f.y);
}

/// Composite the queried string onto `dst` for destination pixel-centre `px`.
/// Walks the glyph run, finds the cell covering `px`, samples the SDF, and
/// blends `params.color` by the antialiased coverage (straight alpha).
public float4 DrawString(
    float4 dst,
    float2 px,
    StructuredBuffer<uint> atlas,
    StructuredBuffer<uint> metrics,
    ConstantBuffer<TextDrawParams> params)
{
    float scale = params.scale;
    float penX = params.penX;
    float3 textRgb = float3(params.color[0], params.color[1], params.color[2]);
    float textA = params.color[3];
    // Edge softness: ~1 destination pixel expressed in SDF value units.
    float aa = max((0.5 / params.spread) / max(scale, 1e-3), 1e-3);

    float cov = 0.0;
    for (uint i = 0u; i < params.charCount; i++)
    {
        uint code = (params.packed[i >> 2u] >> ((i & 3u) * 8u)) & 0xFFu;
        if (code < params.firstChar || code >= params.firstChar + params.glyphCount)
        {
            penX += LoadGlyph(metrics, 0u).advance * scale; // unknown -> space advance
            continue;
        }
        GlyphMetric g = LoadGlyph(metrics, code - params.firstChar);

        // Glyphs with no cell (space) only advance the pen.
        if (g.cellW > 0.0 && g.cellH > 0.0)
        {
            float2 cellTL = float2(penX + g.left * scale, params.penY + g.top * scale);
            float2 rel = px - cellTL;
            float2 cellSize = float2(g.cellW, g.cellH) * scale;
            if (rel.x >= 0.0 && rel.y >= 0.0 && rel.x < cellSize.x && rel.y < cellSize.y)
            {
                float2 atlasUV = float2(g.atlasX, g.atlasY) + rel / scale;
                float d = SampleAtlas(atlas, params.atlasW, params.atlasH, atlasUV);
                cov = max(cov, smoothstep(0.5 - aa, 0.5 + aa, d));
            }
        }
        penX += g.advance * scale;
    }

    float a = saturate(textA * cov);
    return float4(lerp(dst.rgb, textRgb, a), max(dst.a, a));
}
