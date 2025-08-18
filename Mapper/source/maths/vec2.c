#include "maths/vec2.h"

Vec2 vec2_add(Vec2 a, Vec2 b)
{
	return (Vec2) { a.x + b.x, a.y + b.y };
}

Vec2 vec2_sub(Vec2 a, Vec2 b)
{
	return (Vec2) { a.x - b.x, a.y - b.y };
}

Vec2 vec2_scale(Vec2 v, float scalar)
{
	return (Vec2) { v.x * scalar, v.y * scalar };
}

float vec2_dot(Vec2 a, Vec2 b)
{
	return a.x * b.x + a.y * b.y;
}

float vec2_length(Vec2 v)
{
	return sqrtf(v.x * v.x + v.y * v.y);
}

Vec2 vec2_normalize(Vec2 v)
{
	float len = vec2_length(v);
	if (len == 0.0f) return (Vec2) { 0.0f, 0.0f };
	return (Vec2) { v.x / len, v.y / len };
}