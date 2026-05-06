implementing vekl;

/// Color: take Hue & Sat from `blend`, Lum from `base`. Tints while preserving brightness.
public float3 BlendColor(float3 base, float3 blend)
{
    return SetLum(blend, Lum(base));
}
