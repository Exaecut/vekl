implementing vekl;

/// Hue: take Hue from `blend`, Sat & Lum from `base`.
public float3 BlendHue(float3 base, float3 blend)
{
    return SetLum(SetSat(blend, Sat(base)), Lum(base));
}
