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

	void UpdateRays();

private:
	Ray rays[WIDTH][HEIGHT];
	Camera();
public:
	Camera(Camera const&) = delete;
	void operator=(Camera const&) = delete;
};