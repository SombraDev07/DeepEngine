#pragma once
#include <Vec3.h>
#include <Quat.h>

struct Camera
{
	Vec3 position = Vec3(0, 5, -10);
	f32 yaw   = 0;
	f32 pitch = -0.4f;
	f32 fov       = 60.0f;
	f32 nearPlane = 0.1f;
	f32 farPlane  = 1000.0f;
	f32 aspect    = 1.777f;

	struct { f32 m[4][4]{}; } viewMatrix, projMatrix;

	void Update();
};
