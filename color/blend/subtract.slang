implementing vekl;

/// Subtract: `max(base - blend, 0)`. Darkens.
public float3 BlendSubtract(float3 base, float3 blend)
{
    return max(base - blend, float3(0.0));
}
