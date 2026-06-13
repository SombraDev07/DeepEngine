#pragma once
#include "Vec4.h"

struct Quat
{
	f32 x, y, z, w;

	Quat() : x(0), y(0), z(0), w(1) {}
	Quat(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

	static Quat Identity() { return {0, 0, 0, 1}; }
};

struct Mat4
{
	f32 m[4][4]{};

	Mat4() { m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f; }

	static Mat4 Identity() { return Mat4(); }
};
