implementing vekl;

// HSL non-separable blend primitives (W3C / PDF spec). Used by Hue, Saturation, Color, Luminosity.

internal float Lum(float3 c)
{
    return dot(c, float3(0.2126, 0.7152, 0.0722));
}

internal float Sat(float3 c)
{
    return max(max(c.r, c.g), c.b) - min(min(c.r, c.g), c.b);
}

// Clip out-of-gamut channels back into [0, 1] while preserving Lum.
internal float3 ClipColor(float3 c)
{
    float l = Lum(c);
    float n = min(min(c.r, c.g), c.b);
    float x = max(max(c.r, c.g), c.b);
    if (n < 0.0)
        c = l + (c - l) * l / (l - n);
    if (x > 1.0)
        c = l + (c - l) * (1.0 - l) / (x - l);
    return c;
}

// Replace luminance of `c` with `l`, preserving hue & saturation.
internal float3 SetLum(float3 c, float l)
{
    return ClipColor(c + (l - Lum(c)));
}

// Set saturation of `c` to `s`, preserving channel ordering and luminance via linear remap of [min, max] to [0, s].
internal float3 SetSat(float3 c, float s)
{
    float cmin = min(min(c.r, c.g), c.b);
    float cmax = max(max(c.r, c.g), c.b);
    float3 r;
    if (cmax > cmin)
        r = (c - cmin) * s / (cmax - cmin);
    else
        r = float3(0.0, 0.0, 0.0);
    return r;
}
