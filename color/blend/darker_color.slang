implementing vekl;

/// Darker Color: pick whichever of base/blend has lower Rec.709 luminance, returns the full RGB triple.
public float3 BlendDarkerColor(float3 base, float3 blend)
{
    float3 w = float3(0.2126, 0.7152, 0.0722);
    float lb = dot(base, w);
    float lc = dot(blend, w);
    return lb < lc ? base : blend;
}
