implementing vekl;

/// Difference: `|base - blend|`.
public float3 BlendDifference(float3 base, float3 blend)
{
    return abs(base - blend);
}
