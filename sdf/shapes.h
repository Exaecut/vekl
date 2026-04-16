#pragma once
#include "maths/trigonometry.h"

inline float circle(float2 uv, float2 center, float radius)
{
	return length(uv - center) - radius;
}

inline float box(float2 uv, float2 center, float2 size)
{
	float2 half_size = size * 0.5f;
	float2 d = abs(uv - center) - half_size;
	return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f));
}

inline float rounded_box(float2 uv, float2 center, float2 size, float angle, float radius)
{
	float max_r = 0.5f * min(size.x, size.y);
	radius = clamp(radius, 0.0f, max_r);

	float2 p = uv - center;
	float c = cos(angle);
	float s = sin(angle);

	float2 local = float2(
		c * p.x + s * p.y,
		-s * p.x + c * p.y);

	float2 half_size = size * 0.5f - radius;
	float2 d = abs(local) - half_size;
	return min(max(d.x, d.y), 0.0f) + length(max(d, 0.0f)) - radius;
}

inline float ngon(float2 uv, float2 center, float radius, float nsides, float angle)
{
	float2 p = uv - center;

	float c = cos(angle);
	float s = sin(angle);
	float2 local = float2(c * p.x + s * p.y,
							-s * p.x + c * p.y);

	float ang = atan2(local.y, local.x) + HALF_PI;
	float split = TAU / nsides;
	return length(local) * cos(split * floorf(0.5f + ang / split) - ang) - radius;
}

inline float star(float2 uv, float2 center, float radius, int branches, float innerFactor, float angle)
{
	float2 p = uv - center;

	angle += radians(180.0f);
	float c = cos(angle);
	float s = sin(angle);
	p = float2(c * p.x - s * p.y, s * p.x + c * p.y);

	float angleStep = M_PI_F / float(branches);
	float innerAngle = M_PI_F / innerFactor;

	float2 acs = float2(cos(angleStep), sin(angleStep));
	float2 ecs = float2(cos(innerAngle), sin(innerAngle));

	float localAngle = atan2(p.x, p.y);
	localAngle = fmodf(fmodf(localAngle, 2.0f * angleStep) + 2.0f * angleStep, 2.0f * angleStep) - angleStep;
	p = length(p) * float2(cos(localAngle), fabs(sin(localAngle)));

	p -= radius * acs;
	p += ecs * clamp(-dot(p, ecs), 0.0f, radius * acs.y / ecs.y);

	return length(p) * sign(p.x);
}

inline float moon(float2 uv, float2 center, float angle, float distance, float outer_radius, float inner_radius)
{
	float2 p = uv - center;

	float c = cos(angle);
	float s = sin(angle);
	p = float2(c * p.x - s * p.y, s * p.x + c * p.y);

	p.y = fabs(p.y);

	float a = (outer_radius * outer_radius - inner_radius * inner_radius + distance * distance) / (2.0f * distance);
	float b = sqrt(max(outer_radius * outer_radius - a * a, 0.0f));

	if (distance * (p.x * b - p.y * a) > distance * distance * max(b - p.y, 0.0f))
	{
		return length(p - float2(a, b));
	}

	return max(length(p) - outer_radius, -(length(p - float2(distance, 0.0f)) - inner_radius));
}

inline float oriented_vesica(float2 uv, float2 center, float2 size, float angle)
{
	float2 p = uv - center;

	float c = cos(angle);
	float s = sin(angle);
	float2 local = float2(c * p.x + s * p.y,
							-s * p.x + c * p.y);

	float ra = size.x * 0.5f;
	float rb = size.y * 0.5f;
	float d = size.x;

	float a = (ra * ra - rb * rb + d * d) / (2.0f * d);
	float b = sqrt(max(ra * ra - a * a, 0.0f));

	if (d * (local.x * b - local.y * a) > d * d * max(b - local.y, 0.0f))
	{
		return length(local - float2(a, b));
	}
	return max(length(local) - ra,
				-(length(local - float2(d, 0.0f)) - rb));
}

inline float heart(float2 uv, float2 center, float radius, float angle)
{
	float2 p = uv - center;
	angle += radians(180.0f);
	float c = cos(angle), s = sin(angle);
	p = float2(c * p.x - s * p.y, s * p.x + c * p.y);

	p /= radius;
	p.y += 0.5f;

	p.x = fabs(p.x);

	if (p.y + p.x > 1.0f)
	{
		return (length(p - float2(0.25f, 0.75f)) - sqrt(2.0f) * 0.25f) * radius;
	}

	float2 a = p - float2(0.0f, 1.0f);
	float2 b = p - 0.5f * max(p.x + p.y, 0.0f);
	return (min(length(a), length(b)) * sign(p.x - p.y)) * radius;
}

inline float rounded_x(float2 uv, float2 center, float width, float radius, float angle)
{
	float2 p = uv - center;
	float c = cos(angle), s = sin(angle);
	p = float2(c * p.x - s * p.y, s * p.x + c * p.y);
	p = abs(p);

	return length(p - min(p.x + p.y, width) * 0.5f) - radius;
}
