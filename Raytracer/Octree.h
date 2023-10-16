#pragma once
#include <map>
#include "util.h"
#include "Math.h"
#include "Tri.h"
#include "Constants.h"


enum NodePos {
	FRONT_LEFT_TOP, FRONT_RIGHT_TOP,
	FRONT_LEFT_BOT, FRONT_RIGHT_BOT,
	BACK_LEFT_TOP, BACK_RIGHT_TOP,
	BACK_LEFT_BOT, BACK_RIGHT_BOT,
};

typedef struct Node {
	Node* children[8];
	Node* parent;
	std::map<int, class Tri*> tris;
	double d[6];
	Vec3 maxBounds;
	Vec3 minBounds;
	Vec3 center;
	NodePos id;
};

class Octree {
public:
	Octree() {}
	Octree(Vec3 maxBounds, Vec3 minBounds, Tri triangles[MAX_TRIANGLES], int numTris);
	
	void Clean(Node* root);
	Node* FindNode(Node* root, const Vec3& pos);
	bool CalculateIntersection(const struct Ray& ray, struct HitInfo* out, int ignoreID = 0);

	enum CubeFace {
		FRONT, LEFT, BOTTOM,
		BACK, RIGHT, TOP,
		NONE
	};

	Vec3 cubeNormals[6] = {
		Vec3(0, 0, 1),
		Vec3(-1, 0, 0),
		Vec3(0, -1, 0),
		Vec3(0, 0, -1),
		Vec3(1, 0, 0),
		Vec3(0, 1, 0)
	};

private:
	void Init(Node* root);
	void CalculateVoxelIntersection(const Vec3& min, const Vec3& max, const double d[6], const Ray& ray, HitInfo* out);

	const double MIN_SIZE = 0.625;
	const double EPSILON = 0.01;

	Node* root;
};