#pragma once
#include "Math.h"
#include "Constants.h"
#include "Tri.h"
#include "BVH.h"

class Mesh {
public:
	Mesh(Tri* triangles, TriVerts* triVertData, int numTriangles, int meshId);
	~Mesh();
	
	void Animate(float deltaTime);
	Tri* tris;
	TriVerts* vertData;
private:
	Tri* bindPoseTris;

	float rotation = 0;
	float rotationSpeed = 5.0f; 
	int numTris = 0;
public:
	int id;
	float pad;
};