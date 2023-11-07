#include "Camera.h"

Camera::Camera()
{
	pos = Vec3::Zero();
	invPos = Vec3::Zero();
	rot = Mat4::Identity();

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
	float aspectRatio = (float)WIDTH / (float)HEIGHT; //normalizes y to 1
	float tanHalfFOV = (float)tan(((float)fov / 2.) * ((float)PI / 180.)); //the max y position with screen center as origin, projected 1 unit deep

	bottomLeft = Vec3(-aspectRatio * tanHalfFOV, -tanHalfFOV, -1); //bottom left corner of screen
	Vec3 bottomRight = Vec3(aspectRatio * tanHalfFOV, -tanHalfFOV, -1); //top right corner of screen
	Vec3 topLeft = Vec3(-aspectRatio * tanHalfFOV, tanHalfFOV, -1);

	//transform the directions
	bottomLeft = Mat4::Transform(bottomLeft, rot, 0.0f);
	bottomRight = Mat4::Transform(bottomRight, rot, 0.0f);
	topLeft = Mat4::Transform(topLeft, rot, 0.0f);

	lengthY = topLeft - bottomLeft;
	lengthX = bottomRight - bottomLeft;

	//treat top left as 0,0
#pragma omp parallel for schedule(dynamic) 
	for (int h = 0; h < HEIGHT; h++) for (int w = 0; w < WIDTH; w++) {
		Vec3 pixelPos = bottomLeft + (lengthY * (h / (float)HEIGHT)) + (lengthX * (w / (float)WIDTH)); //get interpolated direction

		Vec3 dir = pixelPos.Normalized();//Mat4::Transform(Vec3(x, y, -1.0f), rot, 0.0f).Normalized();
		rays[w][h].Set(pos, dir);
	}
}
