#pragma once
#include "Math.h"
#include <xmmintrin.h>

struct Vertex
{
	Vec3 position;
	Vec3 color_diffuse;
	//Vec3 color_specular;
	Vec3 normal;
	//float shininess;
	
	void Reset() {
		//position = Vec3::Zero();
		color_diffuse = Vec3::Zero();
		//color_specular = Vec3::Zero();
		normal = Vec3::Zero();
		//shininess = 0;
	}
};

struct TriVerts
{
	//Vertex verts[3];
	Vec3 norm[3];
	Vec3 uv1_1_2;
	Vec3 uv2_3_3;
};