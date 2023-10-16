#pragma once
#include "Math.h"

struct Vertex
{
	Vec3 position;
	Vec3 color_diffuse;
	Vec3 color_specular;
	Vec3 normal;
	double shininess;
	void Reset() {
		position = Vec3::Zero();
		color_diffuse = Vec3::Zero();
		color_specular = Vec3::Zero();
		normal = Vec3::Zero();
		shininess = 0;
	}
};
