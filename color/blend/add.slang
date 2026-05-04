implementing vekl;

/// Additive blend. Use for light leaks, glow, and additive light effects.
public T BlendAdd<T>(T base, T blend) where T : IFloat
{
    return base + blend;
}
