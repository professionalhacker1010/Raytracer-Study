#pragma once
#include "Math.h"
#include "Constants.h"
#include "Tri.h"
#include "BVH.h"
class Tri;

class Mesh {
public:
	Mesh(Tri* triangles, int numTriangles, int meshId);
	~Mesh();
	
	void Animate(float deltaTime);
	Tri* tris;
private:
	Tri* bindPoseTris;


	float rotation = 0;
	float rotationSpeed = 5.0f;
	float numTris = 0;
public:
	int id;
};