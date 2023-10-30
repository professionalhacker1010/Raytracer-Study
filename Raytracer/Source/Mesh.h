#pragma once
//#include "External/stdafx.h"
#include "Surface.h"
#include "Math.h"
#include "Constants.h"
#include "Tri.h"
#include "BVH.h"


class Mesh {
public:
	Mesh(const char* objFile, const char* texFile, int meshId);
	~Mesh();
	
	void Animate(float deltaTime);
	Tri* tris;
	TriVerts* vertData;

	//TriVerts vertData[1024];
private:
	Tri* bindPoseTris;
public:
	Surface* texture;
	//Tri bindPoseTris[1024];

	float rotation = 0;
	float rotationSpeed = 5.0f;
public:
	int numTris = 0;

	int id;
};