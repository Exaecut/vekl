implementing vekl;

/// Divide: `base / blend`, clamped to [0, 1].
public float3 BlendDivide(float3 base, float3 blend)
{
    return saturate(base / max(blend, float3(1e-6)));
}
