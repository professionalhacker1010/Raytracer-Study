#pragma once
#include "AABB.h"
#include "Constants.h"
#include <xmmintrin.h>
#include <ctime>
#include <atomic>

struct Ray;
struct HitInfo;

class BVH {
public:
	BVH(class Tri* triangles, unsigned int numTris);
	~BVH();
	void CalculateIntersection(Ray& ray, HitInfo& out, unsigned int nodeIdx = 0);

	//old
	void CalculateIntersection_Recursive(Ray& ray, HitInfo& out, unsigned int nodeIdx = 0);

	//debug
	clock_t constructionTime;
	std::atomic_int falseBranch = 0;
private:
	struct Bin {
		Bin() {
			bounds.min = Vec3(FLT_MAX);
			bounds.max = Vec3(-FLT_MAX);
		}
		AABB bounds;
		int numTris = 0;
	};

	struct BVHNode {
		union {
			struct { Vec3 min; unsigned int leftFirst; };
			__m128 min4;
		};
		union {
			struct { Vec3 max; unsigned int numTris; };
			__m128 max4;
		};
		BVHNode() { min4 = max4 = _mm_set1_ps(0); }
		bool IsLeaf() const { return numTris > 0; }
	};

	//look through tris in the node and update the AABB
	void UpdateNodeBounds(unsigned int index);

	//sort tris into subdivided spaces and create child BVHNodes
	void Subdivide(unsigned int parentIdx);
	void CalculateBestSplit(const BVHNode& parent, float& bestCost, float& bestSplitPos, int& bestAxis);

	float SurfaceAreaHeuristic(const BVHNode& node, int axis, double splitPos);

	int SortAlongAxis(const BVHNode& node, int axis, double splitPos);

	void DebugTraversal(unsigned int idx);

	//old
	int MidpointSplit(unsigned int parentIdx);

	BVHNode* nodes;
	class Tri* tris;
	int nodeCounter;
	unsigned int* triIndices; //proxy for sorted tris, to avoid sorting the actual array
};