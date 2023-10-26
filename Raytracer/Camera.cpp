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
	for (int h = 0; h < HEIGHT; h++) for (int w = 0; w < WIDTH; w++) {
		float y = yMin + (2 * yMax * h / (float)HEIGHT);
		float x = xMin + (2 * xMax * w / (float)WIDTH);
		Vec3 dir = Vec3(x, y, -1.0f);
		rays[w][h].Set(Vec3::Zero(), dir.Normalized());
	}
}
