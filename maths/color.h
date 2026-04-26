#pragma once

#ifndef VEKL_COLOR_H
#define VEKL_COLOR_H

// Branchless RGB to HSV (Inigo Quilez algorithm).
// All channels in [0,1]. H in [0,1] (not degrees).
inline float3 rgb_to_hsv(float3 c) {
    float k = (c.z <= c.y) ? 1.0f : 0.0f;

    float px = mix(c.z, c.y, k);
    float py = mix(c.y, c.z, k);
    float pz = mix(-1.0f, 0.0f, k);
    float pw = mix(2.0f / 3.0f, -1.0f / 3.0f, k);

    float k2 = (px <= c.x) ? 1.0f : 0.0f;

    float mx  = mix(px, c.x, k2);
    float mid = mix(c.x, py, k2);
    float mn  = mix(pw, c.x, k2);
    float hb  = mix(pz, 1.0f / 3.0f, k2);

    float delta = mx - mn;
    float inv_delta = 1.0f / (delta + 1e-10f);

    float h = fabsf(hb + (mid - mn) * inv_delta / 6.0f);
    float s = delta / (mx + 1e-10f);

    return float3(h, s, mx);
}

// Branchless HSV to RGB (Inigo Quilez algorithm).
// H in [0,1] (not degrees), S in [0,1], V in [0,1].
inline float3 hsv_to_rgb(float3 c) {
    float3 k = float3(1.0f, 2.0f / 3.0f, 1.0f / 3.0f);
    float3 p = abs(fract(c.xxx + k) * 6.0f - 3.0f);

    return c.z * mix(float3(1.0f), clamp(p - 1.0f, 0.0f, 1.0f), c.y);
}

inline bool rgb_in_gamut(float3 c) {
    return
        c.x >= 0.0f && c.x <= 1.0f &&
        c.y >= 0.0f && c.y <= 1.0f &&
        c.z >= 0.0f && c.z <= 1.0f;
}

inline bool rgb_in_gamut_eps(float3 c, float eps) {
    return
        c.x >= -eps && c.x <= 1.0f + eps &&
        c.y >= -eps && c.y <= 1.0f + eps &&
        c.z >= -eps && c.z <= 1.0f + eps;
}

// sRGB gamma decode (gamma-encoded → linear)
inline float srgb_to_linear(float x) {
    return (x >= 0.04045f) ? powf((x + 0.055f) / 1.055f, 2.4f) : x / 12.92f;
}

inline float3 srgb_to_linear(float3 c) {
    return float3(srgb_to_linear(c.x), srgb_to_linear(c.y), srgb_to_linear(c.z));
}

// sRGB gamma encode (linear → gamma-encoded)
inline float linear_to_srgb(float x) {
    x = max(x, 0.0f);
    return (x >= 0.0031308f) ? 1.055f * powf(x, 1.0f / 2.4f) - 0.055f : 12.92f * x;
}

inline float3 linear_to_srgb(float3 c) {
    return float3(linear_to_srgb(c.x), linear_to_srgb(c.y), linear_to_srgb(c.z));
}

// Sign-preserving cube root for OKLab LMS conversion.
// Standard cbrtf/cbrt is unavailable on some GPU backends (Metal).
inline float oklab_cbrt(float x) {
    return (x >= 0.0f) ? powf(x, 1.0f / 3.0f) : -powf(-x, 1.0f / 3.0f);
}

// Linear sRGB → OKLab (Björn Ottosson 2020).
// Input: linear sRGB in [0,1]. Output: L in [0,1], a/b unbounded.
inline float3 linear_srgb_to_oklab(float3 c) {
    float l = fmaf(0.4122214708f, c.x,
              fmaf(0.5363325363f, c.y,
                   0.0514459929f * c.z));

    float m = fmaf(0.2119034982f, c.x,
              fmaf(0.6806995451f, c.y,
                   0.1073969566f * c.z));

    float s = fmaf(0.0883024619f, c.x,
              fmaf(0.2817188376f, c.y,
                   0.6299787005f * c.z));

    float l_ = oklab_cbrt(l);
    float m_ = oklab_cbrt(m);
    float s_ = oklab_cbrt(s);

    return float3(
        fmaf(0.2104542553f, l_,
        fmaf(0.7936177850f, m_,
            -0.0040720468f * s_)),

        fmaf(1.9779984951f, l_,
        fmaf(-2.4285922050f, m_,
             0.4505937099f * s_)),

        fmaf(0.0259040371f, l_,
        fmaf(0.7827717662f, m_,
            -0.8086757660f * s_))
    );
}

// OKLab → linear sRGB (Björn Ottosson 2020).
// Output may fall outside [0,1] for out-of-gamut colors.
inline float3 oklab_to_linear_srgb(float3 c) {

    float l_ = c.x + 0.3963377774f * c.y + 0.2158037573f * c.z;
    float m_ = c.x - 0.1055613458f * c.y - 0.0638541728f * c.z;
    float s_ = c.x - 0.0894841775f * c.y - 1.2914855480f * c.z;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    return float3(
        fmaf( 4.0767416621f, l,
        fmaf(-3.3077115913f, m,
              0.2309699292f * s)),

        fmaf(-1.2684380046f, l,
        fmaf( 2.6097574011f, m,
             -0.3413193965f * s)),

        fmaf(-0.0041960863f, l,
        fmaf(-0.7034186147f, m,
              1.7076147010f * s))
    );
}

// OKLab → OKLCH (cylindrical form).
// L unchanged, C = chroma (>= 0), H = hue in radians [-PI, PI].
inline float3 oklab_to_oklch(float3 c) {
    float C = sqrtf(c.y * c.y + c.z * c.z);
    float H = atan2f(c.z, c.y);
    return float3(c.x, C, H);
}

// OKLCH → OKLab.
// Expects H in radians.
inline float3 oklch_to_oklab(float3 c) {
    float a = c.y * cosf(c.z);
    float b = c.y * sinf(c.z);
    return float3(c.x, a, b);
}

inline float3 fit_oklch_to_srgb(float3 lch) {
    if (lch.y <= 1e-6f)
        return lch;

    {
        float3 rgb = oklab_to_linear_srgb(
            oklch_to_oklab(lch)
        );

        if (rgb_in_gamut_eps(rgb, 1e-5f))
            return lch;
    }


    float low  = 0.0f;
    float high = lch.y;
    float C    = lch.y;

    // Two aggressive contractions are usually enough
    for (int i = 0; i < 2; ++i)
    {
        C *= 0.5f;

        float3 test = float3(lch.x, C, lch.z);

        float3 rgb = oklab_to_linear_srgb(
            oklch_to_oklab(test)
        );

        if (rgb_in_gamut_eps(rgb, 1e-5f))
        {
            low = C;
            break;
        }

        high = C;
    }

    if (low == 0.0f)
        high = C;

    for (int i = 0; i < 3; ++i)
    {
        float mid = 0.5f * (low + high);

        float3 test = float3(lch.x, mid, lch.z);

        float3 rgb = oklab_to_linear_srgb(
            oklch_to_oklab(test)
        );

        if (rgb_in_gamut_eps(rgb, 1e-5f))
            low = mid;
        else
            high = mid;
    }

    lch.y = low;
    return lch;
}

// Precomputed variant: caller provides cos/sin of the shift angle,
// avoiding per-pixel transcendentals when the shift is constant.
inline float3 hue_shift_oklab_fast(float3 srgb, float cos_s, float sin_s) {
    float3 linear = srgb_to_linear(srgb);
    float3 lab = linear_srgb_to_oklab(linear);

    // Achromatic early-out: rotating a zero-length (a,b) vector is a no-op
    float chroma_sq = lab.y * lab.y + lab.z * lab.z;
    if (chroma_sq < 1e-12f) {
        return srgb;
    }

    // Rotate hue directly in the OKLab (a,b) plane.
    // Equivalent to oklab→oklch→rotate→oklab but avoids
    // sqrtf, atan2f, fmodf, cosf, sinf per pixel.
    float a = lab.y * cos_s - lab.z * sin_s;
    float b = lab.y * sin_s + lab.z * cos_s;
    float3 lab_shifted = float3(lab.x, a, b);

    // Fast path: try converting directly to sRGB, check gamut
    float3 rgb_linear = oklab_to_linear_srgb(lab_shifted);
    if (rgb_in_gamut_eps(rgb_linear, 1e-5f)) {
        return saturate(linear_to_srgb(rgb_linear));
    }

    // Slow path: out-of-gamut, clip via OKLCH chroma reduction
    float3 lch = oklab_to_oklch(lab_shifted);
    lch = fit_oklch_to_srgb(lch);
    float3 rgb_clipped = oklab_to_linear_srgb(oklch_to_oklab(lch));
    return saturate(linear_to_srgb(rgb_clipped));
}

inline float3 hue_shift_oklab(float3 srgb, float shift_radians) {
    return hue_shift_oklab_fast(srgb, cosf(shift_radians), sinf(shift_radians));
}

#endif
