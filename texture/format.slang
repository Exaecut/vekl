implementing vekl;

public float4 ToRGBA(float4 c, PixelLayout layout)
{
    switch (layout)
    {
    case PixelLayout.Bgra:
        return float4(c.z, c.y, c.x, c.w);
    case PixelLayout.Vuya:
    {
        float chroma_offset = 0.5;
        float v = c.x - chroma_offset;
        float u = c.y - chroma_offset;
        float y = c.z;
        return float4(
            y + 1.402 * v,
            y - 0.344136 * u - 0.714136 * v,
            y + 1.772 * u,
            c.w
        );
    }
    case PixelLayout.Vuya709:
    {
        float chroma_offset = 0.5;
        float v = c.x - chroma_offset;
        float u = c.y - chroma_offset;
        float y = c.z;
        return float4(
            y + 1.5748 * v,
            y - 0.1873 * u - 0.4681 * v,
            y + 1.8556 * u,
            c.w
        );
    }
    default:
        return c;
    }
}

public float4 FromRGBA(float4 c, PixelLayout layout)
{
    switch (layout)
    {
    case PixelLayout.Bgra:
        return float4(c.z, c.y, c.x, c.w);
    case PixelLayout.Vuya:
    {
        float chroma_offset = 0.5;
        float y = 0.299 * c.x + 0.587 * c.y + 0.114 * c.z;
        return float4(
            (c.x - y) / 1.402 + chroma_offset,
            (c.z - y) / 1.772 + chroma_offset,
            y,
            c.w
        );
    }
    case PixelLayout.Vuya709:
    {
        float chroma_offset = 0.5;
        float y = 0.2126 * c.x + 0.7152 * c.y + 0.0722 * c.z;
        return float4(
            (c.x - y) / 1.5748 + chroma_offset,
            (c.z - y) / 1.8556 + chroma_offset,
            y,
            c.w
        );
    }
    default:
        return c;
    }
}

// `baseWords` shifts the read window forward by a whole-word offset so
// mip-chain callers can read a non-zero lod from the same buffer. Callers
// reading lod 0 pass `0u` and the result is identical to the legacy form.
float4 LoadPixelRaw(StructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint bytesPerPixel, PixelStorage storage, uint baseWords)
{
    uint byteOffset = xy.y * pitchBytes + xy.x * bytesPerPixel;
    uint wordOffset = baseWords + byteOffset / 4;

    if (storage == PixelStorage.Unorm8x4)
    {
        uint packed = data[wordOffset];
        return float4(
            float((packed) & 0xFF) / 255.0,
            float((packed >> 8) & 0xFF) / 255.0,
            float((packed >> 16) & 0xFF) / 255.0,
            float((packed >> 24) & 0xFF) / 255.0
        );
    }
    else if (storage == PixelStorage.Unorm16x4)
    {
        uint lo = data[wordOffset];
        uint hi = data[wordOffset + 1];
        return float4(
            float(lo & 0xFFFF) / 65535.0,
            float(lo >> 16) / 65535.0,
            float(hi & 0xFFFF) / 65535.0,
            float(hi >> 16) / 65535.0
        );
    }
    else
    {
        uint w0 = data[wordOffset];
        uint w1 = data[wordOffset + 1];
        uint w2 = data[wordOffset + 2];
        uint w3 = data[wordOffset + 3];
        return float4(asfloat(w0), asfloat(w1), asfloat(w2), asfloat(w3));
    }
}

float4 LoadPixelRaw(StructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint bytesPerPixel, PixelStorage storage)
{
    return LoadPixelRaw(data, pitchBytes, xy, bytesPerPixel, storage, 0u);
}

float4 LoadPixelRaw(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint bytesPerPixel, PixelStorage storage, uint baseWords)
{
    uint byteOffset = xy.y * pitchBytes + xy.x * bytesPerPixel;
    uint wordOffset = baseWords + byteOffset / 4;

    if (storage == PixelStorage.Unorm8x4)
    {
        uint packed = data[wordOffset];
        return float4(
            float((packed) & 0xFF) / 255.0,
            float((packed >> 8) & 0xFF) / 255.0,
            float((packed >> 16) & 0xFF) / 255.0,
            float((packed >> 24) & 0xFF) / 255.0
        );
    }
    else if (storage == PixelStorage.Unorm16x4)
    {
        uint lo = data[wordOffset];
        uint hi = data[wordOffset + 1];
        return float4(
            float(lo & 0xFFFF) / 65535.0,
            float(lo >> 16) / 65535.0,
            float(hi & 0xFFFF) / 65535.0,
            float(hi >> 16) / 65535.0
        );
    }
    else
    {
        uint w0 = data[wordOffset];
        uint w1 = data[wordOffset + 1];
        uint w2 = data[wordOffset + 2];
        uint w3 = data[wordOffset + 3];
        return float4(asfloat(w0), asfloat(w1), asfloat(w2), asfloat(w3));
    }
}

float4 LoadPixelRaw(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint bytesPerPixel, PixelStorage storage)
{
    return LoadPixelRaw(data, pitchBytes, xy, bytesPerPixel, storage, 0u);
}

void StorePixelRaw(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy, float4 c, uint bytesPerPixel, PixelStorage storage, uint baseWords)
{
    uint byteOffset = xy.y * pitchBytes + xy.x * bytesPerPixel;
    uint wordOffset = baseWords + byteOffset / 4;

    if (storage == PixelStorage.Unorm8x4)
    {
        uint packed = (uint(saturate(c.x) * 255.0))
                    | (uint(saturate(c.y) * 255.0) << 8)
                    | (uint(saturate(c.z) * 255.0) << 16)
                    | (uint(saturate(c.w) * 255.0) << 24);
        data[wordOffset] = packed;
    }
    else if (storage == PixelStorage.Unorm16x4)
    {
        data[wordOffset]     = (uint(saturate(c.x) * 65535.0))
                              | (uint(saturate(c.y) * 65535.0) << 16);
        data[wordOffset + 1] = (uint(saturate(c.z) * 65535.0))
                              | (uint(saturate(c.w) * 65535.0) << 16);
    }
    else
    {
        data[wordOffset]     = asuint(c.x);
        data[wordOffset + 1] = asuint(c.y);
        data[wordOffset + 2] = asuint(c.z);
        data[wordOffset + 3] = asuint(c.w);
    }
}

void StorePixelRaw(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy, float4 c, uint bytesPerPixel, PixelStorage storage)
{
    StorePixelRaw(data, pitchBytes, xy, c, bytesPerPixel, storage, 0u);
}

public float4 LoadPixel(StructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint bytesPerPixel, PixelStorage storage, PixelLayout layout, uint baseWords)
{
    return ToRGBA(LoadPixelRaw(data, pitchBytes, xy, bytesPerPixel, storage, baseWords), layout);
}

public float4 LoadPixel(StructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint bytesPerPixel, PixelStorage storage, PixelLayout layout)
{
    return LoadPixel(data, pitchBytes, xy, bytesPerPixel, storage, layout, 0u);
}

public float4 LoadPixel(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint bytesPerPixel, PixelStorage storage, PixelLayout layout, uint baseWords)
{
    return ToRGBA(LoadPixelRaw(data, pitchBytes, xy, bytesPerPixel, storage, baseWords), layout);
}

public float4 LoadPixel(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint bytesPerPixel, PixelStorage storage, PixelLayout layout)
{
    return LoadPixel(data, pitchBytes, xy, bytesPerPixel, storage, layout, 0u);
}

public void StorePixel(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy, float4 c, uint bytesPerPixel, PixelStorage storage, PixelLayout layout, uint baseWords)
{
    StorePixelRaw(data, pitchBytes, xy, FromRGBA(c, layout), bytesPerPixel, storage, baseWords);
}

public void StorePixel(RWStructuredBuffer<uint> data, uint pitchBytes, uint2 xy, float4 c, uint bytesPerPixel, PixelStorage storage, PixelLayout layout)
{
    StorePixel(data, pitchBytes, xy, c, bytesPerPixel, storage, layout, 0u);
}

uint2 AddressClamp(uint2 xy, uint2 sizePx)
{
    return clamp(xy, uint2(0, 0), sizePx - uint2(1, 1));
}

uint2 AddressRepeat(uint2 xy, uint2 sizePx)
{
    return xy % sizePx;
}

uint2 AddressMirror(uint2 xy, uint2 sizePx)
{
    uint2 period = sizePx * 2u - 2u;
    uint2 m = xy % period;
    return select(m < sizePx, m, period - m);
}

uint2 AddressXY(uint2 xy, uint2 sizePx, AddressMode mode)
{
    if (mode == AddressMode.Repeat)
        return AddressRepeat(xy, sizePx);
    if (mode == AddressMode.Mirror)
        return AddressMirror(xy, sizePx);
    return AddressClamp(xy, sizePx);
}

public float4 LoadPixelSafe(StructuredBuffer<uint> data, uint pitchBytes, uint2 xy, uint2 sizePx, uint bytesPerPixel, PixelStorage storage, PixelLayout layout, AddressMode address)
{
    uint2 addr = AddressXY(xy, sizePx, address);
    return LoadPixel(data, pitchBytes, addr, bytesPerPixel, storage, layout);
}
