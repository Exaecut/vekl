implementing vekl;

// sRGB ↔ linear (IEC 61966-2-1) and EV exposure scaling. Per the convention
// captured in `StorageIsPerceptual`, 8/16-bit unorm storage is treated as
// sRGB-encoded and 32-bit float as scene-linear; `LinearizeIfNeeded` /
// `EncodeIfNeeded` route through the right path on that flag.

public float SrgbToLinearChannel(float c)
{
    return (c <= 0.04045)
        ? c / 12.92
        : pow((c + 0.055) / 1.055, 2.4);
}

public float LinearToSrgbChannel(float c)
{
    return (c <= 0.0031308)
        ? c * 12.92
        : 1.055 * pow(c, 1.0 / 2.4) - 0.055;
}

public float3 SrgbToLinear(float3 c)
{
    return float3(
        SrgbToLinearChannel(c.r),
        SrgbToLinearChannel(c.g),
        SrgbToLinearChannel(c.b)
    );
}

public float3 LinearToSrgb(float3 c)
{
    return float3(
        LinearToSrgbChannel(saturate(c.r)),
        LinearToSrgbChannel(saturate(c.g)),
        LinearToSrgbChannel(saturate(c.b))
    );
}

public bool StorageIsPerceptual(PixelStorage storage)
{
    return storage == PixelStorage.Unorm8x4
        || storage == PixelStorage.Unorm16x4;
}

public float3 LinearizeIfNeeded(float3 rgb, PixelStorage storage)
{
    return StorageIsPerceptual(storage) ? SrgbToLinear(rgb) : rgb;
}

public float3 EncodeIfNeeded(float3 rgb, PixelStorage storage)
{
    return StorageIsPerceptual(storage) ? LinearToSrgb(rgb) : rgb;
}

public float ExposureToScale(float ev)
{
    return exp2(ev);
}

public float3 ApplyExposure(float3 linear_rgb, float ev)
{
    return linear_rgb * ExposureToScale(ev);
}
