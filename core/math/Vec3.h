#pragma once
#include <Platform.h>
#include <cmath>

struct Vec3
{
	f32 x, y, z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(f32 v) : x(v), y(v), z(v) {}
	Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}

	Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
	Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
	Vec3 operator*(f32 s) const { return {x * s, y * s, z * s}; }
	Vec3 operator/(f32 s) const { f32 inv = 1.0f / s; return {x * inv, y * inv, z * inv}; }

	Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
	Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
	Vec3& operator*=(f32 s) { x *= s; y *= s; z *= s; return *this; }

	Vec3 operator-() const { return {-x, -y, -z}; }

	f32 LengthSq() const { return x * x + y * y + z * z; }
	f32 Length() const { return sqrtf(LengthSq()); }
	Vec3 Normalized() const { f32 len = Length(); return len > 0.0001f ? *this / len : Vec3(); }

	f32 Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
	Vec3 operator*(const Vec3& o) const { return {x * o.x, y * o.y, z * o.z}; }
	Vec3 Cross(const Vec3& o) const
	{
		return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
	}

	static Vec3 Lerp(const Vec3& a, const Vec3& b, f32 t) { return a + (b - a) * t; }
};
