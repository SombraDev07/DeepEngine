#include "Camera.h"
#include <cmath>

void Camera::Update()
{
	float cy = cosf(yaw), sy = sinf(yaw);
	float cp = cosf(pitch), sp = sinf(pitch);

	// Forward in LH: +Z forward, +X right, +Y up
	Vec3 fwd(cp * sy, sp, cp * cy);

	// If orbit mode, compute position from orbitTarget
	if (bOrbit) {
		position = orbitTarget - fwd * orbitDist;
	}

	Vec3 worldUp(0, 1, 0);
	Vec3 right;
	if (fabs(fwd.Dot(worldUp)) > 0.999f) {
		Vec3 ref(1, 0, 0);
		right = fwd.Cross(ref).Normalized();
	} else {
		right = worldUp.Cross(fwd).Normalized();
	}
	Vec3 up = fwd.Cross(right).Normalized();

	memset(viewMatrix.m, 0, sizeof(viewMatrix.m));
	viewMatrix.m[0][0] = right.x; viewMatrix.m[1][0] = right.y; viewMatrix.m[2][0] = right.z;
	viewMatrix.m[3][0] = -position.Dot(right);
	viewMatrix.m[0][1] = up.x;    viewMatrix.m[1][1] = up.y;    viewMatrix.m[2][1] = up.z;
	viewMatrix.m[3][1] = -position.Dot(up);
	viewMatrix.m[0][2] = fwd.x;   viewMatrix.m[1][2] = fwd.y;   viewMatrix.m[2][2] = fwd.z;
	viewMatrix.m[3][2] = -position.Dot(fwd);
	viewMatrix.m[0][3] = 0; viewMatrix.m[1][3] = 0; viewMatrix.m[2][3] = 0; viewMatrix.m[3][3] = 1;

	float fovRad = fov * 3.14159265f / 180.0f;
	float h = 1.0f / tanf(fovRad * 0.5f);
	float w = h / aspect;
	float q = farPlane / (farPlane - nearPlane);
	memset(projMatrix.m, 0, sizeof(projMatrix.m));
	projMatrix.m[0][0] = w;
	projMatrix.m[1][1] = h;
	projMatrix.m[2][2] = q;
	projMatrix.m[2][3] = 1;
	projMatrix.m[3][2] = -q * nearPlane;
}
