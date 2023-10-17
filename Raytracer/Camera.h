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
	Vec3 position;
	Vec3 rayDirections[WIDTH][HEIGHT];

	void UpdateRays();
	Ray rays[WIDTH][HEIGHT];

private:
	Camera();
public:
	Camera(Camera const&) = delete;
	void operator=(Camera const&) = delete;
};