#include "Camera.h"

Camera::Camera()
{
	position = Vec3::Zero();

	UpdateRays();
}

Ray& Camera::GetRay(unsigned int x, unsigned int y)
{
	return rays[x][y];
}

void Camera::UpdateRays()
{
	// calculate camera ray directions
	double aspectRatio = (double)WIDTH / (double)HEIGHT;
	double tanHalfFOV = tan(((double)fov / 2.) * ((double)PI / 180.));
	double xMax = aspectRatio * tanHalfFOV;
	double xMin = -aspectRatio * tanHalfFOV;
	double yMax = tanHalfFOV;
	double yMin = -tanHalfFOV;

	//treat top left as 0,0
	for (int h = 0; h < HEIGHT; h++) {
		double y = yMin + (2 * yMax * h / (double)HEIGHT);
		for (int w = 0; w < WIDTH; w++) {
			double x = xMin + (2 * xMax * w / (double)WIDTH);
			Vec3 dir = Vec3(x, y, -1.0f) - position;
			rays[w][h].Set(position, dir.Normalized());//dir.Normalized();
		}
	}
}
