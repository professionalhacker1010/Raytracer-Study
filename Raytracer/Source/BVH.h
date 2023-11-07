#pragma once
#include "AABB.h"
#include "Tri.h"

struct Ray;
struct HitInfo;
class Tri;
class Mesh;
class MeshInstance;
class Camera;

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
	BVH(int meshId, unsigned int numTris);
	~BVH();

	void Set(int meshId, unsigned int numTris);
	bool CalculateIntersection(Ray& ray, HitInfo& out, unsigned int nodeIdx = 0);
	void Rebuild();
	void Refit();
	AABB GetBounds() { return AABB(nodes[0].min, nodes[0].max); }

	//debug
	void DebugTraversal(unsigned int idx);
	//std::atomic_int falseBranch = 0;

private:	
	void UpdateNodeBounds(unsigned int index); //look through tris in the node and update the AABB
	void Subdivide(unsigned int parentIdx); //sort tris into subdivided spaces and create child BVHNodes
	void CalculateBestSplit(Tri* tris, const BVHNode& parent, float& bestCost, float& bestSplitPos, int& bestAxis);
	int SortAlongAxis(Tri* tris, const BVHNode& node, int axis, double splitPos);

public:
	BVHNode* nodes; //nodes created in order s.t. right and left nodes next to each other, top->bottom
	int meshId = -1, pad0;
	unsigned int* triIndices; //proxy for sorted tris, to avoid sorting the actual array
	int numNodes = 0, numTris;
};

struct BVHInstance {
	BVHInstance() = default;
	BVHInstance(int bvHeirarchy, int meshInstance);
	void Set(int bvHeirarchy, int meshInstance);
	bool CalculateIntersection(Ray& ray, HitInfo& out, unsigned int nodeIdx = 0);
	void SetTransform(Mat4 transform);
	Mat4 invTransform; //64
	AABB worldSpaceBounds;
	int bvhId = -1;
	int meshInstId = -1;
	float pad[6];
};

struct TLASNode {
	TLASNode() {}
	union {
		Vec3 min;
		struct { float pad0, pad1, pad2; unsigned int leftRight; };
		__m128 min4;
	};
	union {
		Vec3 max;
		struct { float pad3, pad4, pad5; unsigned int BLAS; };
		__m128 max4;
	};
};

class TLAS {
public:
	TLAS() = default;
	TLAS(BVHInstance* bvhInstances, int numInstances);
	~TLAS();

	void Rebuild();
	bool CalculateIntersection(Ray& ray, HitInfo& out, unsigned int nodeIdx = 0);
	
private:
	int FindBestMatch(unsigned int* indices, int numNodes, int a);
	
public:
	TLASNode* nodes; //nodes created in order s.t. all leaf nodes come first -> go towards root node
	BVHInstance* blas; //bottom-level acceleration structures, not sorted
	unsigned int numNodes = 0, numBLAS;
};