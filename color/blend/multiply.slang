implementing vekl;

/// Multiply blend. Use for shadows, vignettes, and darkening.
/// Result is always darker than either input.
public T BlendMultiply<T>(T base, T blend) where T : IFloat
{
    return base * blend;
}
