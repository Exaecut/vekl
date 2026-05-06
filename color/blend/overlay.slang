implementing vekl;

/// Overlay: Multiply where base < 0.5, Screen otherwise. Boosts contrast.
/// Per-channel branch — float3-only.
public float3 BlendOverlay(float3 base, float3 blend)
{
    float3 r;
    r.x = base.x < 0.5 ? 2.0 * base.x * blend.x : 1.0 - 2.0 * (1.0 - base.x) * (1.0 - blend.x);
    r.y = base.y < 0.5 ? 2.0 * base.y * blend.y : 1.0 - 2.0 * (1.0 - base.y) * (1.0 - blend.y);
    r.z = base.z < 0.5 ? 2.0 * base.z * blend.z : 1.0 - 2.0 * (1.0 - base.z) * (1.0 - blend.z);
    return r;
}
