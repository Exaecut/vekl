implementing vekl;

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

    public float4 Load(uint2 pixel)
    {
        return LoadPixel(buffer, desc.pitchBytes, pixel, desc.bytesPerPixel, desc.storage, desc.layout);
    }

    public float4 LoadSafe(uint2 pixel)
    {
        return LoadPixelSafe(buffer, desc.pitchBytes, pixel, Size(), desc.bytesPerPixel, desc.storage, desc.layout, desc.addressMode);
    }

    public float4 SampleNearest(float2 uv)
    {
        uint2 xy = AddressClamp(uint2(PixelCoord(uv, Size()) + 0.5), Size());
        return LoadPixel(buffer, desc.pitchBytes, xy, desc.bytesPerPixel, desc.storage, desc.layout);
    }

    public float4 SampleLinear(float2 uv)
    {
        uint2 sizePx = Size();
        float2 p = PixelCoord(uv, sizePx);
        float2 pf = floor(p);
        float2 f = clamp(p - pf, 0.0, 1.0);

        uint2 xy00 = AddressClamp(uint2(pf), sizePx);
        uint2 xy10 = AddressClamp(uint2(pf.x + 1.0, pf.y), sizePx);
        uint2 xy01 = AddressClamp(uint2(pf.x, pf.y + 1.0), sizePx);
        uint2 xy11 = AddressClamp(uint2(pf + 1.0), sizePx);

        float4 c00 = LoadPixel(buffer, desc.pitchBytes, xy00, desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c10 = LoadPixel(buffer, desc.pitchBytes, xy10, desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c01 = LoadPixel(buffer, desc.pitchBytes, xy01, desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c11 = LoadPixel(buffer, desc.pitchBytes, xy11, desc.bytesPerPixel, desc.storage, desc.layout);

        float4 cx0 = lerp(c00, c10, f.x);
        float4 cx1 = lerp(c01, c11, f.x);
        return lerp(cx0, cx1, f.y);
    }

    public float4 SampleLinearRepeat(float2 uv)
    {
        uint2 sizePx = Size();
        float2 p = PixelCoord(frac(uv), sizePx);
        float2 pf = floor(p);
        float2 f = clamp(p - pf, 0.0, 1.0);

        uint2 xy00 = AddressRepeat(uint2(pf), sizePx);
        uint2 xy10 = AddressRepeat(uint2(pf.x + 1.0, pf.y), sizePx);
        uint2 xy01 = AddressRepeat(uint2(pf.x, pf.y + 1.0), sizePx);
        uint2 xy11 = AddressRepeat(uint2(pf + 1.0), sizePx);

        float4 c00 = LoadPixel(buffer, desc.pitchBytes, xy00, desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c10 = LoadPixel(buffer, desc.pitchBytes, xy10, desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c01 = LoadPixel(buffer, desc.pitchBytes, xy01, desc.bytesPerPixel, desc.storage, desc.layout);
        float4 c11 = LoadPixel(buffer, desc.pitchBytes, xy11, desc.bytesPerPixel, desc.storage, desc.layout);

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

    public uint2 Size()
    {
        return uint2(desc.width, desc.height);
    }

    public float4 Load(uint2 pixel)
    {
        return LoadPixel(buffer, desc.pitchBytes, pixel, desc.bytesPerPixel, desc.storage, desc.layout);
    }

    public void Store(uint2 pixel, float4 value)
    {
        StorePixel(buffer, desc.pitchBytes, pixel, value, desc.bytesPerPixel, desc.storage, desc.layout);
    }
}
