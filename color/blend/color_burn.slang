implementing vekl;

/// Color Burn: `1 - (1 - base) / blend`, clamped to [0, 1].
public float3 BlendColorBurn(float3 base, float3 blend)
{
    float3 r = 1.0 - (1.0 - base) / max(blend, float3(1e-6));
    return saturate(r);
}
