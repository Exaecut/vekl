implementing vekl;

/// Linear Dodge / Add: `base + blend`. Caller clamps if needed.
public T BlendAdd<T>(T base, T blend) where T : IFloat
{
    return base + blend;
}
