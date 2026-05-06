implementing vekl;

/// Multiply: darkens. Standard choice for shadows / vignettes.
public T BlendMultiply<T>(T base, T blend) where T : IFloat
{
    return base * blend;
}
