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
	void SetPosition(Vec3 position) {
		pos = position;
		invPos = -1 * pos;
	}
	Vec3 GetInvPosition() { return invPos; }
	Vec3 GetPosition() { return pos; }

	void UpdateRays();

private:
	Ray rays[WIDTH][HEIGHT];
	Vec3 pos;
	Vec3 invPos;
	Camera();
public:
	Camera(Camera const&) = delete;
	void operator=(Camera const&) = delete;
};