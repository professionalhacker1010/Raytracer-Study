#pragma once
#include "Math.h"

struct Vertex
{
	Vec3 position;
	Vec3 albedo;
	//Vec3 color_specular;
	Vec3 normal;
	//float shininess;
	
	void Reset() {
		//position = Vec3::Zero();
		albedo = Vec3::Zero();
		//color_specular = Vec3::Zero();
		normal = Vec3::Zero();
		//shininess = 0;
	}
};

struct TriVerts
{
	union { Vec3 norm0; struct { float pad0[3]; float uv2x; }; };
	union { Vec3 norm1; struct { float pad1[3]; float uv2y; }; };
	Vec3 norm2;
	Vec2 uv0;
	Vec2 uv1;
};