implementing vekl;

/// Color Dodge: brightens via `base / (1 - blend)`, clamped to [0,1].
public float3 BlendColorDodge(float3 base, float3 blend)
{
    float3 r = base / max(1.0 - blend, float3(1e-6));
    return saturate(r);
}
