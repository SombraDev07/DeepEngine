#include "Camera.h"
#include <cmath>

static void LookAt(f32 out[4][4], const Vec3& eye, const Vec3& at, const Vec3& up)
{
	Vec3 f = (at - eye).Normalized();
	Vec3 s = f.Cross(up).Normalized();
	Vec3 u = s.Cross(f);

	memset(out, 0, sizeof(f32) * 16);
	out[0][0] = s.x;  out[0][1] = u.x;  out[0][2] = -f.x;
	out[1][0] = s.y;  out[1][1] = u.y;  out[1][2] = -f.y;
	out[2][0] = s.z;  out[2][1] = u.z;  out[2][2] = -f.z;
	out[3][0] = -s.Dot(eye);
	out[3][1] = -u.Dot(eye);
	out[3][2] = f.Dot(eye);
	out[3][3] = 1.0f;
}

static void PerspectiveFov(f32 out[4][4], f32 fovY, f32 aspect, f32 zn, f32 zf)
{
	f32 h = 1.0f / tanf(fovY * 0.5f * 3.14159265f / 180.0f);
	f32 w = h / aspect;
	memset(out, 0, sizeof(f32) * 16);
	out[0][0] = w;
	out[1][1] = h;
	out[2][2] = zf / (zf - zn);
	out[2][3] = 1.0f;
	out[3][2] = -zn * zf / (zf - zn);
}

void Camera::Update()
{
	LookAt(viewMatrix.m, position, target, up);
	PerspectiveFov(projMatrix.m, fov, aspect, nearPlane, farPlane);
}
