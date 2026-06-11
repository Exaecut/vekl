implementing vekl;

// When the output canvas is larger than the source layer (Premiere full-canvas path),
// every source read must map output -> source as p_src = p_out - ext and return
// premultiplied zero outside the layer. With ext = (0,0) and canvas == layer
// these helpers collapse to the identity mapping, so the same shader code
// runs unchanged on the AE clip-space path.

/// Output pixel -> source pixel. Displacing effects (camera shake) add their
/// offset to the result of this mapping, never to the write coordinate.
[ForceInline]
public int2 CanvasToSourcePx(uint2 outPx, FrameParams frame)
{
    return int2(outPx) - int2(frame.extX, frame.extY);
}

/// True when the mapped source pixel falls inside the layer rect.
[ForceInline]
public bool InSource(int2 srcPx, TextureView src)
{
    uint2 s = src.Size();
    return srcPx.x >= 0 && srcPx.y >= 0 && srcPx.x < int(s.x) && srcPx.y < int(s.y);
}

/// Exact-texel source read at an output-canvas coordinate. Out-of-source
/// taps return premultiplied zero (never wrap, never clamp).
[ForceInline]
public float4 SampleSource(TextureView src, uint2 outPx, FrameParams frame)
{
    int2 srcPx = CanvasToSourcePx(outPx, frame);
    if (!InSource(srcPx, src))
        return float4(0.0, 0.0, 0.0, 0.0);
    return src.Load(uint2(srcPx));
}

/// Bilinear source read at a fractional output-canvas coordinate. UV is
/// normalised by the SOURCE size (never the canvas). Out-of-source returns
/// premultiplied zero.
[ForceInline]
public float4 SampleSourceLinear(TextureView src, float2 outPx, FrameParams frame)
{
    float2 srcPx = outPx - float2(float(frame.extX), float(frame.extY));
    uint2 s = src.Size();
    float2 uv = srcPx / float2(s);
    if (any(uv < float2(0.0, 0.0)) || any(uv > float2(1.0, 1.0)))
        return float4(0.0, 0.0, 0.0, 0.0);
    return src.SampleLinear(uv);
}

/// True when the output-canvas pixel falls inside the expanded rendering
/// region: the source rect inset by (extX, extY) with the caller's declared
/// margins. Every kernel that writes to a Premiere full-canvas outFrame MUST
/// check this on entry and skip pixels outside the expanded region, so that
/// the effect's own FrameSetup / SmartPreRender expansion extent is honoured
/// even though the GPU outFrame covers the whole sequence.
[ForceInline]
public bool InExpandedBounds(
    uint2 outPx,
    FrameParams frame,
    int expLeft,
    int expRight,
    int expTop,
    int expBottom)
{
    int x = int(outPx.x);
    int y = int(outPx.y);
    return x >= int(frame.extX) - expLeft
        && x <  int(frame.extX) + int(frame.layerWidth)  + expRight
        && y >= int(frame.extY) - expTop
        && y <  int(frame.extY) + int(frame.layerHeight) + expBottom;
}
