#pragma once
#include "Vec3.h"

struct Vec4
{
	f32 x, y, z, w;

	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
	Vec4(const Vec3& v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}

	Vec3 XYZ() const { return {x, y, z}; }
};
