#include "Camera.h"

Camera::Camera()
{
	pos = Vec3::Zero();
	invPos = Vec3::Zero();

	UpdateRays();
}

Ray Camera::GetRay(unsigned int x, unsigned int y)
{
	return rays[x][y];
}

Ray Camera::GetRayInterp(unsigned int x, unsigned int y, int idx)
{
	Ray& up = rays[x][y];
	return Ray();
}

void Camera::SetTransform(Vec3 position, Mat4 rotation)
{
	pos = position;
	invPos = -1 * pos;
	rot = rotation;
	UpdateRays();
}

void Camera::UpdateRays()
{
	// calculate camera ray directions
	float aspectRatio = (float)WIDTH / (float)HEIGHT;
	float tanHalfFOV = (float)tan(((float)fov / 2.) * ((float)PI / 180.));
	float xMax = aspectRatio * tanHalfFOV;
	float xMin = -aspectRatio * tanHalfFOV;
	float yMax = tanHalfFOV;
	float yMin = -tanHalfFOV;

	//treat top left as 0,0
#pragma omp parallel for schedule(dynamic) 
	for (int h = 0; h < HEIGHT; h++) for (int w = 0; w < WIDTH; w++) {
		float y = yMin + (2 * yMax * h / (float)HEIGHT);
		float x = xMin + (2 * xMax * w / (float)WIDTH);

		Vec3 dir = Mat4::Transform(Vec3(x, y, -1.0f), rot, 0.0f).Normalized();
		rays[w][h].Set(pos, dir);
	}
}
