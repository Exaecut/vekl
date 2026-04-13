#pragma once

#ifndef MATHS_EASING_METAL
#define MATHS_EASING_METAL

inline float clamp01(float t) {
    return clamp(t, 0.0f, 1.0f);
}

inline float pow_ease(float t, float exp) {
    return pow(clamp01(t), exp);
}

inline float expo_ease(float t, float base) {
    return (pow(base, clamp01(t)) - 1.0f) / (base - 1.0f);
}

inline float circ_ease_in(float t) {
    t = clamp01(t);
    return 1.0f - sqrt(1.0f - t * t);
}

inline float circ_ease_out(float t) {
    t = clamp01(t) - 1.0f;
    return sqrt(1.0f - t * t);
}

inline float circ_ease_in_out(float t) {
    t = clamp01(t) * 2.0f;
    if (t < 1.0f) return 0.5f * (1.0f - sqrt(1.0f - t * t));
    t -= 2.0f;
    return 0.5f * (sqrt(1.0f - t * t) + 1.0f);
}

inline float sine_ease_in(float t) {
    return 1.0f - cos(clamp01(t) * M_PI_F * 0.5f);
}
inline float sine_ease_out(float t) {
    return sin(clamp01(t) * M_PI_F * 0.5f);
}
inline float sine_ease_in_out(float t) {
    return 0.5f * (1.0f - cos(clamp01(t) * M_PI_F));
}

inline float ease_in(float t, float exp) {
    return pow_ease(t, exp);
}
inline float ease_out(float t, float exp) {
    return 1.0f - pow_ease(1.0f - t, exp);
}
inline float ease_in_out(float t, float exp) {
    t = clamp01(t) * 2.0f;
    if (t < 1.0f) return 0.5f * pow_ease(t, exp);
    return 1.0f - 0.5f * pow_ease(2.0f - t, exp);
}

inline float expo_ease_in(float t, float base) {
    return expo_ease(t, base);
}
inline float expo_ease_out(float t, float base) {
    return 1.0f - expo_ease(1.0f - t, base);
}
inline float expo_ease_in_out(float t, float base) {
    t = clamp01(t);
    if (t < 0.5f) return 0.5f * expo_ease(t * 2.0f, base);
    return 1.0f - 0.5f * expo_ease((1.0f - t) * 2.0f, base);
}

inline float spring_ease(float t, float stiffness, float damping, float mass) {
    t = clamp01(t);

    float omega0 = sqrt(stiffness / mass);
    float zeta   = damping / (2.0f * sqrt(stiffness * mass));

    if (zeta < 1.0f) {

        float omega_d = omega0 * sqrt(1.0f - zeta * zeta);
        return 1.0f - exp(-zeta * omega0 * t) *
                     (cos(omega_d * t) + (zeta / sqrt(1.0f - zeta * zeta)) * sin(omega_d * t));
    } else if (zeta == 1.0f) {

        return 1.0f - exp(-omega0 * t) * (1.0f + omega0 * t);
    } else {

        float omega_d = omega0 * sqrt(zeta * zeta - 1.0f);
        float A = exp((-zeta * omega0 + omega_d) * t);
        float B = exp((-zeta * omega0 - omega_d) * t);
        return 1.0f - (A - B) / (2.0f * omega_d);
    }
}

enum easing_mode : uint
{
	easing_ease_in_out = 0,
	easing_ease_in = 1,
	easing_ease_out = 2,
	easing_linear,
};

inline float apply_easing(float progress, uint mode, float exponent)
{
	switch (mode)
	{
	case easing_mode::easing_ease_in_out:
		return ease_in_out(progress, exponent);
	case easing_mode::easing_ease_in:
		return ease_in(progress, exponent);
	case easing_mode::easing_ease_out:
		return ease_out(progress, exponent);
	case easing_mode::easing_linear:
		return progress;
	default:
		return progress;
	}
}

#endif
