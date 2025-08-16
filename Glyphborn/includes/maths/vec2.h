#ifndef VEC2_H
#define VEC2_H

typedef struct
{
	float x;
	float y;
} Vec2;

Vec2 vec2_add(Vec2 a, Vec2 b);
Vec2 vec2_sub(Vec2 a, Vec2 b);
Vec2 vec2_scale(Vec2 v, float scalar);
float vec2_dot(Vec2 a, Vec2 b);
float vec2_length(Vec2 v);
Vec2 vec2_normalize(Vec2 v);

#endif // !VEC2_H
