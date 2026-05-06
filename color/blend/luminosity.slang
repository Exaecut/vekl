implementing vekl;

/// Luminosity: take Lum from `blend`, Hue & Sat from `base`. Inverse of Color.
public float3 BlendLuminosity(float3 base, float3 blend)
{
    return SetLum(base, Lum(blend));
}
