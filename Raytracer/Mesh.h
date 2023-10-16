#pragma once
#include "Math.h"
#include "Constants.h"
#include "Tri.h"

class Tri;
class BVH;

class Mesh {
public:
	Mesh(Tri* triangles, int numTriangles);
	~Mesh();
	
	void Animate(float deltaTime);

	BVH* bvh;
	//Vec3 position;
private:
	Tri* bindPoseTris;


	float rotation = 0;
	float rotationSpeed = 5.0f;
	float numTris = 0;
};