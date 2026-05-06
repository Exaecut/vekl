implementing vekl;

/// Saturation: take Sat from `blend`, Hue & Lum from `base`.
public float3 BlendSaturation(float3 base, float3 blend)
{
    return SetLum(SetSat(base, Sat(blend)), Lum(base));
}
