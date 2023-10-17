#pragma once
#include "AABB.h"
#include "Constants.h"
#include "Tri.h"
#include <xmmintrin.h>
#include <ctime>
#include <atomic>

struct Ray;
struct HitInfo;
class Tri;
class MeshInstance;

struct BVHNode {
	union {
		Vec3 min;
		struct { float pad0, pad1, pad2; unsigned int leftFirst; };
		__m128 min4;
	};
	union {
		Vec3 max;
		struct { float pad3, pad4, pad5; unsigned int numTris; };
		__m128 max4;
	};
	BVHNode() { min4 = max4 = _mm_set1_ps(0); }
	bool IsLeaf() const { return numTris > 0; }
};

class BVH {
public:
	BVH() = default;
	BVH(Tri* triangles, unsigned int numTris);
	~BVH();

	void Set(Tri* triangles, unsigned int numTris);
	bool CalculateIntersection(Ray& ray, HitInfo& out, unsigned int nodeIdx = 0);
	void Rebuild();
	void Refit();
	AABB GetBounds() { return AABB(nodes[0].min, nodes[0].max); }

	//debug
	void DebugTraversal(unsigned int idx);
	std::atomic_int falseBranch = 0;

private:	
	void UpdateNodeBounds(unsigned int index); //look through tris in the node and update the AABB
	void Subdivide(unsigned int parentIdx); //sort tris into subdivided spaces and create child BVHNodes
	void CalculateBestSplit(const BVHNode& parent, float& bestCost, float& bestSplitPos, int& bestAxis);
	int SortAlongAxis(const BVHNode& node, int axis, double splitPos);

	BVHNode* nodes;
	Tri* tris;
	unsigned int* triIndices; //proxy for sorted tris, to avoid sorting the actual array
	int numNodes = 0;
	int numTris;
};

struct BVHInstance {
	BVHInstance() {}
	BVHInstance(BVH* bvHeirarchy, MeshInstance* meshInstance);
	void Set(BVH* bvHeirarchy, MeshInstance* meshInstance);
	BVH* bvh;
	MeshInstance* mesh;
	AABB worldSpaceBounds;
};

struct TLASNode {
	union {
		struct { Vec3 min; unsigned int leftBLAS; };
		__m128 min4;
	};
	union {
		struct { Vec3 max; unsigned int isLeaf; };
		__m128 max4;
	};
};

class TLAS {
public:
	TLAS() = default;
	TLAS(BVH** bvhList, MeshInstance** meshInstances, int numMeshInstances);
	void Rebuild();
	bool CalculateIntersection(Ray& ray, HitInfo& out, unsigned int nodeIdx = 0);

private:
	TLASNode* nodes;
	BVHInstance* blas;
	//MeshInstance** meshes;
	//BVH** blas;
	unsigned int numNodes, numBLAS;
};