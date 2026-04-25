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

#endif
