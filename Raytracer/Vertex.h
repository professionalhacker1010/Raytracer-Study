#pragma once
#include "Math.h"
#include <xmmintrin.h>

struct Vertex
{
	Vec3 position;
	Vec3 color_diffuse;
	Vec3 color_specular;
	Vec3 normal;
	float shininess;
	//union {
	//	Vec3 normal;
	//	struct {
	//		float pad0, pad1, pad2, shininess;
	//	};
	//	__m128 norm4;
	//};
	
	void Reset() {
		position = Vec3::Zero();
		color_diffuse = Vec3::Zero();
		color_specular = Vec3::Zero();
		normal = Vec3::Zero();
		shininess = 0;
	}
};
