implementing vekl;

public float2 TexCoord(uint2 gid, uint2 sizePx)
{
    return (float2(gid) + 0.5) / float2(sizePx);
}

public float2 PixelCoord(float2 uv, uint2 sizePx)
{
    return uv * float2(sizePx) - 0.5;
}

public float2 ScaleUV(float2 uv, float2 scale)
{
    float2 centered = uv - 0.5;
    centered *= scale;
    return centered + 0.5;
}

public float2 ScaleUV(float2 uv, float scale)
{
    return ScaleUV(uv, float2(scale, scale));
}

public float2 RotateUV(float2 uv, float angle, float aspect)
{
    float2 centered = uv - 0.5;
    centered.x *= aspect;
    float c = cos(angle);
    float s = sin(angle);
    float2 rotated = float2(
        c * centered.x - s * centered.y,
        s * centered.x + c * centered.y
    );
    rotated.x /= aspect;
    return rotated + 0.5;
}

public float2 UniformAspectRatio(float2 uv, uint2 sizePx)
{
    float aspect = float(sizePx.x) / float(sizePx.y);
    return ScaleUV(uv, float2(aspect, 1.0));
}
