#pragma once

inline float2 arc_sampling(float2 center, float2 origin, float angle, int index, int samples, float2 aspect_scale, uint uniform_aspect)
{
    float2 delta;
    if (uniform_aspect == 1u) {
        delta = (center - origin) / aspect_scale;
    } else {
        delta = center - origin;
    }

    float radius = length(delta);
    float theta = atan2(delta.y, delta.x);

    float half_angle = angle * 0.5f;
    float theta_i = theta + (-half_angle + angle * float(index) / float(samples - 1));

    float u, v;
    if (uniform_aspect == 1u) {
        u = origin.x + (radius * cos(theta_i)) * aspect_scale.x;
        v = origin.y + (radius * sin(theta_i)) * aspect_scale.y;
    } else {
        u = origin.x + radius * cos(theta_i);
        v = origin.y + radius * sin(theta_i);
    }

    return float2(u, v);
}

inline float2 linear_sampling(float2 center, float2 origin, float distance, int index, int samples, float2 aspect_scale, uint uniform_aspect)
{
    float step_distance = distance / float(samples - 1);
    step_distance = step_distance * float(index);

    float2 direction;
    if (uniform_aspect == 1u) {
        direction = normalize((center - origin) / aspect_scale);
    } else {
        direction = normalize(center - origin);
    }

    float2 sample_uv;
    if (uniform_aspect == 1u) {
        sample_uv = center + (direction * step_distance) * aspect_scale;
    } else {
        sample_uv = center + (direction * step_distance);
    }

    return sample_uv;
}
