implementing vekl;

// Catmull-Rom cubic B-spline weights: 4 taps centered at the nearest texel; sharper than bilinear without ringing.
internal float4 CubicWeights(float v)
{
    float4 n = float4(1.0, 2.0, 3.0, 4.0) - v;
    float4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return float4(x, y, z, w) * (1.0 / 6.0);
}

/// Bicubic (Catmull-Rom) sampler. 4 bilinear taps via the weight-fold trick
/// instead of 16 raw fetches. Use over `SampleLinear` when reconstructing soft
/// signals (upsampling, bicubic warp fields).
public float4 SampleBicubic(TextureView tex, float2 uv)
{
    uint2 sizePx = tex.Size();
    float2 res = float2(sizePx);
    float2 invRes = 1.0 / res;

    float2 p = uv * res - 0.5;
    float2 pf = frac(p);
    float2 pi = p - pf;

    float4 xc = CubicWeights(pf.x);
    float4 yc = CubicWeights(pf.y);

    float4 c = float4(pi.x - 0.5, pi.x + 1.5, pi.y - 0.5, pi.y + 1.5);
    float4 s = float4(xc.x + xc.y, xc.z + xc.w, yc.x + yc.y, yc.z + yc.w);
    float4 offset = c + float4(xc.y / s.x, xc.w / s.y, yc.y / s.z, yc.w / s.w);

    float2 uv00 = float2(offset.x * invRes.x, offset.z * invRes.y);
    float2 uv10 = float2(offset.y * invRes.x, offset.z * invRes.y);
    float2 uv01 = float2(offset.x * invRes.x, offset.w * invRes.y);
    float2 uv11 = float2(offset.y * invRes.x, offset.w * invRes.y);

    float4 c00 = tex.SampleLinear(uv00);
    float4 c10 = tex.SampleLinear(uv10);
    float4 c01 = tex.SampleLinear(uv01);
    float4 c11 = tex.SampleLinear(uv11);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return lerp(lerp(c11, c01, sx), lerp(c10, c00, sx), sy);
}
