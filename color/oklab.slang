implementing vekl;

// Oklab perceptual color space (Björn Ottosson, 2020). Decouples luma `L`
// from a green↔red axis `a` and a blue↔yellow axis `b`. The (a, b) plane
// is roughly perceptually uniform: hue is its polar angle and chroma its
// magnitude, so a literal 2D rotation of (a, b) gives a hue shift that
// preserves apparent lightness.
//
// Inputs are treated as linear RGB. Adobe hosts in 32-bit float modes are
// already linear; for 8-bit / 16-bit integer modes the values are sRGB
// gamma-encoded — the math below still runs, but a small perceptual
// mismatch is the price of skipping a per-pixel decode/encode pair.

internal float SafeCbrt(float v)
{
    // `pow(neg, 1/3)` is undefined in HLSL/Slang; fold the sign back in.
    return sign(v) * pow(abs(v), 1.0 / 3.0);
}

/// Linear RGB → Oklab `(L, a, b)`.
public float3 RGBToOklab(float3 rgb)
{
    float l = 0.4122214708 * rgb.r + 0.5363325363 * rgb.g + 0.0514459929 * rgb.b;
    float m = 0.2119034982 * rgb.r + 0.6806995451 * rgb.g + 0.1073969566 * rgb.b;
    float s = 0.0883024619 * rgb.r + 0.2817188376 * rgb.g + 0.6299787005 * rgb.b;

    float l_ = SafeCbrt(l);
    float m_ = SafeCbrt(m);
    float s_ = SafeCbrt(s);

    return float3(
        0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
        1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
        0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_
    );
}

/// Oklab `(L, a, b)` → linear RGB. May produce slightly out-of-gamut
/// channels after a round trip; clamp if writing to an unsigned-norm target.
public float3 OklabToRGB(float3 lab)
{
    float l_ = lab.x + 0.3963377774 * lab.y + 0.2158037573 * lab.z;
    float m_ = lab.x - 0.1055613458 * lab.y - 0.0638541728 * lab.z;
    float s_ = lab.x - 0.0894841775 * lab.y - 1.2914855480 * lab.z;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    return float3(
        +4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s,
        -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s,
        -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s
    );
}

/// Rotate hue by `shift` radians in Oklab's (a, b) plane. Luma and chroma
/// magnitude are preserved; only the polar angle changes. The result is
/// perceptually uniform — equal `shift` steps look like equal hue steps.
public float3 HueShiftOklab(float3 rgb, float shift)
{
    float3 lab = RGBToOklab(rgb);
    float c = cos(shift);
    float s = sin(shift);
    float a = lab.y;
    float b = lab.z;
    lab.y = c * a - s * b;
    lab.z = s * a + c * b;
    return OklabToRGB(lab);
}
