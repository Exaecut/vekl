implementing vekl;

// Rec.601 (NTSC) luma/chroma decomposition. YIQ separates luminance (Y)
// from in-phase (I) and quadrature (Q) chroma components, which is how
// NTSC broadcast encoded color. Useful for VHS / analog-TV style effects
// where chroma is subsampled or distorted independently of luma.

/// RGB → YIQ (Rec.601). Returns `(Y, I, Q)`.
public float3 RGBToYIQ(float3 rgb)
{
    return float3(
        0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b,
        0.596 * rgb.r - 0.275 * rgb.g - 0.321 * rgb.b,
        0.212 * rgb.r - 0.523 * rgb.g + 0.311 * rgb.b
    );
}

/// YIQ → RGB (Rec.601). Inverse of [`RGBToYIQ`].
public float3 YIQToRGB(float3 yiq)
{
    return float3(
        yiq.x + 0.956 * yiq.y + 0.621 * yiq.z,
        yiq.x - 0.272 * yiq.y - 0.647 * yiq.z,
        yiq.x - 1.106 * yiq.y + 1.703 * yiq.z
    );
}
