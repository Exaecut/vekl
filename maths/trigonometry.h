#pragma once

#ifndef TRIGONOMETRY_H
#define TRIGONOMETRY_H

constant float PI = 3.14159265358979323846f;
constant float TAU = 6.28318530717958647692f;
constant float HALF_PI = 1.57079632679489661923f;
constant float INV_PI = 0.31830988618379067154f;

inline float radians(float deg)
{
	return deg * (M_PI_F / 180.0f);
}

inline float degrees(float rad)
{
	return rad * (180.0f / M_PI_F);
}

inline float wrap_pi(float h)
{
    h = fmodf(h + PI, TAU);
    if (h < 0.0f) h += TAU;
    return h - PI;
}

#endif
