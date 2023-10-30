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
	//Vertex verts[3];
	Vec3 norm[3];
	Vec2 uv[3];
};