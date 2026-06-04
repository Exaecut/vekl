implementing vekl;

// Returns a safe mip lod clamped to `[0, mipLevelCount-1]` so out-of-range requests degrade gracefully. A 1-level descriptor always returns 0.
uint MipLod(TextureDesc desc, uint lod)
{
    return min(lod, max(desc.mipLevelCount, 1u) - 1u);
}

// Resolve a top-left pixel coordinate to the physical buffer row. For a
// bottom-up host buffer (`desc.flipY == 1`, e.g. Premiere CPU) row 0 maps to
// the last memory row, so reads, writes and procedural UV all stay top-left and
// match the GPU path. `heightPx` is the height of the level being accessed.
uint2 FlipY(TextureDesc desc, uint2 px, uint heightPx)
{
    if (desc.flipY != 0u && heightPx > 0u)
        px.y = heightPx - 1u - px.y;
    return px;
}

public struct TextureView
{
    public StructuredBuffer<uint> buffer;
    public TextureDesc desc;

    public bool Contains(uint2 pixel)
    {
        return pixel.x < desc.width && pixel.y < desc.height;
    }

    public uint2 Size()
    {
        return uint2(desc.width, desc.height);
    }

    public uint2 Size(uint lod)
    {
        uint l = MipLod(desc, lod);
        return uint2(desc.mipWidth[l], desc.mipHeight[l]);
    }

    public bool Contains(uint2 pixel, uint lod)
    {
        uint2 s = Size(lod);
        return pixel.x < s.x && pixel.y < s.y;
    }

    public float4 Load(uint2 pixel)
    {
        return LoadPixel(buffer, desc.pitchBytes, FlipY(desc, pixel, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
    }

    public float4 Load(uint2 pixel, uint lod)
    {
        uint l = MipLod(desc, lod);
        uint baseWords = desc.mipOffsetBytes[l] / 4u;
        return LoadPixel(buffer, desc.mipPitchBytes[l], FlipY(desc, pixel, desc.mipHeight[l]), desc.bytesPerPixel, desc.storage, desc.layout, baseWords);
    }

    public float4 LoadSafe(uint2 pixel)
    {
        return LoadPixelSafe(buffer, desc.pitchBytes, pixel, Size(), desc.bytesPerPixel, desc.storage, desc.layout, desc.addressMode, desc.flipY);
    }

    public float4 SampleNearest(float2 uv)
    {
        float2 nearestF = PixelCoord(uv, Size()) + 0.5;
        uint2 xy = AddressClamp(uint2(int2(nearestF)), Size());
        return LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
    }

    public float4 SampleLinear(float2 uv)
    {
        uint2 sizePx = Size();
        float2 p = PixelCoord(uv, sizePx);
        float2 pf = floor(p);
        float2 f = clamp(p - pf, 0.0, 1.0);

        uint2 ipf = uint2(pf);
        uint2 xy00 = AddressClamp(ipf, sizePx);
        uint2 xy10 = AddressClamp(ipf + uint2(1, 0), sizePx);
        uint2 xy01 = AddressClamp(ipf + uint2(0, 1), sizePx);
        uint2 xy11 = AddressClamp(ipf + uint2(1, 1), sizePx);

        float4 c00 = LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy00, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c10 = LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy10, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c01 = LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy01, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c11 = LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy11, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);

        float4 cx0 = lerp(c00, c10, f.x);
        float4 cx1 = lerp(c01, c11, f.x);
        return lerp(cx0, cx1, f.y);
    }

    public float4 SampleLinear(float2 uv, uint lod)
    {
        uint l = MipLod(desc, lod);
        uint2 sizePx = uint2(desc.mipWidth[l], desc.mipHeight[l]);
        uint pitch = desc.mipPitchBytes[l];
        uint baseWords = desc.mipOffsetBytes[l] / 4u;

        float2 p = PixelCoord(uv, sizePx);
        float2 pf = floor(p);
        float2 f = clamp(p - pf, 0.0, 1.0);

        uint2 ipf = uint2(pf);
        uint2 xy00 = AddressClamp(ipf, sizePx);
        uint2 xy10 = AddressClamp(ipf + uint2(1, 0), sizePx);
        uint2 xy01 = AddressClamp(ipf + uint2(0, 1), sizePx);
        uint2 xy11 = AddressClamp(ipf + uint2(1, 1), sizePx);

        float4 c00 = LoadPixel(buffer, pitch, FlipY(desc, xy00, sizePx.y), desc.bytesPerPixel, desc.storage, desc.layout, baseWords);
        float4 c10 = LoadPixel(buffer, pitch, FlipY(desc, xy10, sizePx.y), desc.bytesPerPixel, desc.storage, desc.layout, baseWords);
        float4 c01 = LoadPixel(buffer, pitch, FlipY(desc, xy01, sizePx.y), desc.bytesPerPixel, desc.storage, desc.layout, baseWords);
        float4 c11 = LoadPixel(buffer, pitch, FlipY(desc, xy11, sizePx.y), desc.bytesPerPixel, desc.storage, desc.layout, baseWords);

        float4 cx0 = lerp(c00, c10, f.x);
        float4 cx1 = lerp(c01, c11, f.x);
        return lerp(cx0, cx1, f.y);
    }

    /// Trilinear sampler: lerps between mip levels `floor(lodF)` and `ceil(lodF)`.
    /// Use from kernels that pick a continuous lod per pixel (pyramidal blur / glow)
    /// to avoid visible seams at mip boundaries.
    public float4 SampleLinearTrilinear(float2 uv, float lodF)
    {
        float maxL = float(max(desc.mipLevelCount, 1u) - 1u);
        float clamped = clamp(lodF, 0.0, maxL);
        uint lo = uint(floor(clamped));
        uint hi = min(lo + 1u, uint(maxL));
        float t = clamp(clamped - float(lo), 0.0, 1.0);
        float4 a = SampleLinear(uv, lo);
        float4 b = SampleLinear(uv, hi);
        return lerp(a, b, t);
    }

    public float4 SampleLinearRepeat(float2 uv)
    {
        uint2 sizePx = Size();
        float2 p = PixelCoord(frac(uv), sizePx);
        float2 pf = floor(p);
        float2 f = clamp(p - pf, 0.0, 1.0);

        uint2 ipf = uint2(pf);
        uint2 xy00 = AddressRepeat(ipf, sizePx);
        uint2 xy10 = AddressRepeat(ipf + uint2(1, 0), sizePx);
        uint2 xy01 = AddressRepeat(ipf + uint2(0, 1), sizePx);
        uint2 xy11 = AddressRepeat(ipf + uint2(1, 1), sizePx);

        float4 c00 = LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy00, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c10 = LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy10, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c01 = LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy01, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c11 = LoadPixel(buffer, desc.pitchBytes, FlipY(desc, xy11, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);

        float4 cx0 = lerp(c00, c10, f.x);
        float4 cx1 = lerp(c01, c11, f.x);
        return lerp(cx0, cx1, f.y);
    }

    public float4 SampleLinearMirror(float2 uv)
    {
        float2 uv_mirrored = abs(frac(uv * 0.5) * 2.0 - 1.0);
        uv_mirrored.x = 1.0 - uv_mirrored.x;
        uv_mirrored.y = 1.0 - uv_mirrored.y;
        return SampleLinear(uv_mirrored);
    }
}

public struct RWTextureView
{
    public RWStructuredBuffer<uint> buffer;
    public TextureDesc desc;

    public bool Contains(uint2 pixel)
    {
        return pixel.x < desc.width && pixel.y < desc.height;
    }

    public bool Contains(uint2 pixel, uint lod)
    {
        uint2 s = Size(lod);
        return pixel.x < s.x && pixel.y < s.y;
    }

    public uint2 Size()
    {
        return uint2(desc.width, desc.height);
    }

    public uint2 Size(uint lod)
    {
        uint l = MipLod(desc, lod);
        return uint2(desc.mipWidth[l], desc.mipHeight[l]);
    }

    public float4 Load(uint2 pixel)
    {
        return LoadPixel(buffer, desc.pitchBytes, FlipY(desc, pixel, desc.height), desc.bytesPerPixel, desc.storage, desc.layout);
    }

    public float4 Load(uint2 pixel, uint lod)
    {
        uint l = MipLod(desc, lod);
        uint baseWords = desc.mipOffsetBytes[l] / 4u;
        return LoadPixel(buffer, desc.mipPitchBytes[l], FlipY(desc, pixel, desc.mipHeight[l]), desc.bytesPerPixel, desc.storage, desc.layout, baseWords);
    }

    public void Store(uint2 pixel, float4 value)
    {
        StorePixel(buffer, desc.pitchBytes, FlipY(desc, pixel, desc.height), value, desc.bytesPerPixel, desc.storage, desc.layout);
    }

    public void Store(uint2 pixel, float4 value, uint lod)
    {
        uint l = MipLod(desc, lod);
        uint baseWords = desc.mipOffsetBytes[l] / 4u;
        StorePixel(buffer, desc.mipPitchBytes[l], FlipY(desc, pixel, desc.mipHeight[l]), value, desc.bytesPerPixel, desc.storage, desc.layout, baseWords);
    }
}
