#pragma once
#include "Vector.h"
#include "RayCast.h"
#include "Constants.h"

class Camera {
public:
	Camera();
	Ray GetRay(unsigned int x, unsigned int y);
	Vec3 position;
	Vec3 rayDirections[WIDTH][HEIGHT];

	void UpdateRays();
	Ray rays[WIDTH][HEIGHT];

};