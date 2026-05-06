implementing vekl;

/// Screen: brightens. `1 - (1 - base)(1 - blend)`. Inverse of Multiply.
public T BlendScreen<T>(T base, T blend) where T : IFloat
{
    return T(1.0) - (T(1.0) - base) * (T(1.0) - blend);
}
