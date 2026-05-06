implementing vekl;

/// Difference: `|base - blend|`. Useful for diffing layers.
public float3 BlendDifference(float3 base, float3 blend)
{
    return abs(base - blend);
}
