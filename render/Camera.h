#pragma once
#include <Vec3.h>
#include <Quat.h>

struct Camera
{
	Vec3 position = Vec3(0, 5, -10);
	f32 yaw = 0, pitch = -0.4f;
	f32 fov = 60.0f;
	f32 nearPlane = 0.1f, farPlane = 1000.0f, aspect = 1.777f;
	// Orbit mode
	Vec3 orbitTarget = Vec3(0, 2, 0);
	f32 orbitDist = 10.0f;
	bool bOrbit = false;
	// Smoothing
	f32 smoothPos = 0.15f, smoothRot = 0.15f;

	struct { f32 m[4][4]{}; } viewMatrix, projMatrix;
	void Update();
};
