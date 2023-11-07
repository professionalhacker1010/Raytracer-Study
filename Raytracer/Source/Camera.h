#pragma once
#include "Math.h"
#include "RayCast.h"
#include "Constants.h"

class Camera {
public:
	static Camera& Get() {
		static Camera instance;
		return instance;
	}
	Ray GetRay(unsigned int x, unsigned int y);
	Ray GetRayInterp(unsigned int x, unsigned int y, int idx);
	void SetTransform(Vec3 position, Mat4 rotation);
	Vec3 GetInvPosition() { return invPos; }
	Vec3 GetPosition() { return pos; }

	void UpdateRays();

private:
	Ray rays[WIDTH][HEIGHT];
	Mat4 rot;
	Vec3 pos;
	Vec3 invPos;

public:
	Vec3 bottomLeft;
	Vec3 lengthY;
	Vec3 lengthX;

private:
	Camera();
public:
	Camera(Camera const&) = delete;
	void operator=(Camera const&) = delete;
};