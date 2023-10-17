#include "BVH.h"
#include "util.h"
#include "RayCast.h"
#include "MeshInstance.h"
#include "Mesh.h"
#include "Camera.h"

bool debugPrint = true;
bool debugColor = false;
bool debugSAH = false;
Vec3 colors[] = {
	Vec3(1, 0, 0), //red
	Vec3(0, 1, 0), //green
	Vec3(0, 0, 1), //blue
	Vec3(1, 1, 0), //yellow
	Vec3(0, 1, 1), //teal
	Vec3(1, 0, 1), //pink
};
int colorIdx = 0;

void BVH::DebugTraversal(unsigned int idx) {
	const BVHNode& node = nodes[idx];

	if (node.IsLeaf()) {
		Util::Print("Leaf node with num tris = " + std::to_string(node.numTris) + " start idx = " + std::to_string(node.leftFirst));
		return;
	}
	else {
		Util::Print("node min " + (std::string)node.min + " node max " + (std::string)node.max);
	}

	DebugTraversal(node.leftFirst);
	DebugTraversal(node.leftFirst + 1);
}

BVH::BVH(Tri* triangles, unsigned int numTris)
{
	Set(triangles, numTris);
}

void BVH::Set(Tri* triangles, unsigned int numTris)
{
	tris = triangles;
	nodes = (BVHNode*)_aligned_malloc(sizeof(BVHNode) * numTris * 2, 64);
	triIndices = new unsigned int[numTris];
	this->numTris = numTris;

	if (debugPrint) Util::Print("Total tris in scene = " + std::to_string(numTris));
	if (debugPrint) Util::Print("Sizeof BVH node = " + std::to_string(sizeof(Tri)));
}

void BVH::Rebuild()
{
	clock_t startTime = clock();
	for (unsigned int i = 0; i < numTris; i++) triIndices[i] = i;

	BVHNode& root = nodes[0];
	root.numTris = numTris;
	root.leftFirst = 0;

	numNodes = 0;
	UpdateNodeBounds(numNodes);
	Subdivide(numNodes);

	if (debugPrint) Util::Print("Nodes generated = " + std::to_string(numNodes + 1));
	//if (debugPrint) DebugTraversal(0);
}

void BVH::Refit()
{
	for (int i = numNodes - 1; i >= 0; i--) {
		BVHNode& node = nodes[i];
		if (node.numTris > 0) {
			UpdateNodeBounds(i);
			continue;
		}
		BVHNode& left = nodes[node.leftFirst];
		BVHNode& right = nodes[node.leftFirst + 1];
		unsigned int leftFirst = node.leftFirst, numTris = node.numTris;
		node.min = Vec3::Min(left.min, right.min);
		node.max = Vec3::Max(left.max, right.max);
		node.leftFirst = leftFirst;
		node.numTris = numTris;
	}
}

bool BVH::CalculateIntersection(Ray& ray, HitInfo& out, unsigned int nodeIdx)
{
	BVHNode* node = &nodes[nodeIdx], * left, * right, * stack[MAX_BVH_STACK];
	unsigned int stackIdx = 0;
	bool hasHit = false;

	while (true)
	{
		if (node->numTris > 0)
		{
			//find the closest triangle hit
			unsigned int maxIdx = node->numTris + node->leftFirst;
			for (unsigned int i = node->leftFirst; i < maxIdx; i++)
			{
				if (tris[triIndices[i]].CalculateIntersection(ray, out)) {
					out.hit = &tris[triIndices[i]];
					ray.maxDist = out.distance;
					hasHit = true;
				}
				else falseBranch++;
			}
			if (stackIdx == 0) break;
			else node = stack[--stackIdx];
			continue;
		}
		left = &nodes[node->leftFirst];
		right = &nodes[node->leftFirst + 1];

		float leftDist = FLT_MAX;
		Ray::IntersectAABB_SIMD(ray, left->min4, left->max4, leftDist);
		float rightDist = FLT_MAX;
		Ray::IntersectAABB_SIMD(ray, right->min4, right->max4, rightDist);

		//intersect with neither 
		if (leftDist == FLT_MAX && rightDist == FLT_MAX) {
			if (stackIdx == 0) break;
			else node = stack[--stackIdx];
		}
		//intersection found, keep going down the tree
		//if intersected with the other child too, push it to the stack for later
		else {
			if (leftDist > rightDist) {
				node = right;
				if (leftDist != FLT_MAX) stack[stackIdx++] = left;
			}
			else {
				node = left;
				if (rightDist != FLT_MAX) stack[stackIdx++] = right;
			}
			
		}
	}
	return hasHit;
}

void BVH::UpdateNodeBounds(unsigned int index)
{
	BVHNode& node = nodes[index];
	unsigned int leftFirst = node.leftFirst, numTris = node.numTris;
	unsigned int maxIdx = leftFirst + numTris;
	node.min = Vec3(FLT_MAX);
	node.max = Vec3(-FLT_MAX);

	for (unsigned int i = leftFirst; i < maxIdx; i++) {
		Tri& tri = tris[triIndices[i]];
		for (unsigned int j = 0; j < 3; j++) {
			node.min = Vec3::Min(node.min, tri.verts[j].position);
			node.max = Vec3::Max(node.max, tri.verts[j].position);
		}
	}
	node.leftFirst = leftFirst;
	node.numTris = numTris;
	//if (debug) Util::Print("Node " + std::to_string(index) + " resized to min = " + (std::string)node.bounds.min + ", max = " + (std::string)node.bounds.max);
}

void BVH::Subdivide(unsigned int parentIdx)
{
	BVHNode& parent = nodes[parentIdx];

	if (parentIdx == 0) numNodes++; //to fit left and right nodes on 64 byte cache line

	//calculate split resulting in smallest AABB surface areas
	float bestCost = FLT_MAX;
	float bestSplitPos = 0;
	int bestAxis = -1;
	CalculateBestSplit(parent, bestCost, bestSplitPos, bestAxis);

	//if the split doesn't subdivide into smaller/better boxes than the parent, stop subdividing
	float parentCost = parent.numTris * AABB(parent.min, parent.max).Area();
	if (bestCost >= parentCost) {
		if (debugColor) {
			unsigned int maxTriIdx = parent.leftFirst + parent.numTris;
			for (unsigned int i = parent.leftFirst; i < maxTriIdx; i++) {
				Tri& tri = tris[triIndices[i]];
				for (int j = 0; j < 3; j++) tri.verts[j].color_diffuse = colors[colorIdx];
			}
			colorIdx++;
			if (colorIdx >= 6) colorIdx = 0;
		}
		return;
	}

	int splitIdx = SortAlongAxis(parent, bestAxis, bestSplitPos);

	//create child nodes
	int numLeftTris = splitIdx - parent.leftFirst;
	if (numLeftTris == 0 || numLeftTris == parent.numTris) return;

	int leftChildIdx = ++numNodes;
	int rightChildIdx = ++numNodes;
	
	//here leftFirst is interpreted as first index of tris
	nodes[leftChildIdx].leftFirst = parent.leftFirst;
	nodes[leftChildIdx].numTris = numLeftTris;
	nodes[rightChildIdx].leftFirst = splitIdx;
	nodes[rightChildIdx].numTris = parent.numTris - numLeftTris;

	//switch leftFirst to being interpreted as left node index
	parent.numTris = 0;
	parent.leftFirst = leftChildIdx;

	//update bounds
	UpdateNodeBounds(leftChildIdx);
	UpdateNodeBounds(rightChildIdx);

	//subdivide
	Subdivide(leftChildIdx);
	Subdivide(rightChildIdx);
}

void BVH::CalculateBestSplit(const BVHNode& parent, float& bestCost, float& bestSplitPos, int& bestAxis)
{
	unsigned int maxTriIdx = parent.leftFirst + parent.numTris;
	const int SPLIT_PLANES = 4;

	//split along longest axis
	Vec3 extents = Vec3(parent.max) - Vec3(parent.min);
	int axis = 0;
	if (extents[1] > extents[0]) axis = 1;
	if (extents[3] > extents[axis]) axis = 2;
	for (int axis = 0; axis < 3; axis++) {

		//fit the min and max bounds on the selected axis
		float minBound = parent.max[axis], maxBound = parent.min[axis];
		for (unsigned int i = parent.leftFirst; i < maxTriIdx; i++) {
			Tri& tri = tris[triIndices[i]];
			minBound = (float)fmin(minBound, tri.centroid[axis]);
			maxBound = (float)fmax(maxBound, tri.centroid[axis]);
		}
		if (maxBound == minBound) return;
		float boundSize = maxBound - minBound;

		struct Bin {
			Bin() {
				bounds.min = Vec3(FLT_MAX);
				bounds.max = Vec3(-FLT_MAX);
			}
			AABB bounds;
			int numTris = 0;
		};

		//populate bins
		const int BINS = SPLIT_PLANES + 1;
		Bin bin[BINS]; //one bin for each interval (split planes + 1)
		float step = boundSize / (float)BINS; //size of each bin relative to bound size
		for (unsigned int i = parent.leftFirst; i < maxTriIdx; i++) {
			Tri& tri = tris[triIndices[i]];
			int binIdx = (int)fmin(BINS - 1, (int)((tri.centroid[axis] - minBound) / step)); //the tri affects the bounds of whichever bin the centroid is in
			bin[binIdx].numTris++;
			for (unsigned int j = 0; j < 3; j++) bin[binIdx].bounds.Grow(tri.verts[j].position);
		}

		//precalculate area and tri counts
		float leftArea[SPLIT_PLANES], rightArea[SPLIT_PLANES];
		int leftTris[SPLIT_PLANES], rightTris[SPLIT_PLANES];
		AABB leftBin = AABB(Vec3(FLT_MAX), Vec3(-FLT_MAX));
		AABB rightBin = AABB(Vec3(FLT_MAX), Vec3(-FLT_MAX));
		int leftSum = 0, rightSum = 0;
		for (unsigned int i = 0; i < SPLIT_PLANES; i++) {
			leftSum += bin[i].numTris;
			leftBin.Grow(bin[i].bounds);
			leftTris[i] = leftSum;
			leftArea[i] = leftBin.Area();
			rightSum += bin[BINS - 1 - i].numTris;
			rightBin.Grow(bin[BINS - 1 - i].bounds);
			rightTris[SPLIT_PLANES - 1 - i] = rightSum;
			rightArea[SPLIT_PLANES - 1 - i] = rightBin.Area();
		}

		//find the best SA heuristic
		for (unsigned int i = 0; i < SPLIT_PLANES; i++) {
			float splitPos = minBound + (float)(i + 1) * step;
			//based on splitPos and axis, calculate SA of resulting AABB's
			float cost = (float)leftTris[i] * leftArea[i] + (float)rightTris[i] * rightArea[i];
			if (cost < bestCost) {
				bestCost = cost;
				bestSplitPos = splitPos;
				bestAxis = axis;
			}
		}
	}
}

int BVH::SortAlongAxis(const BVHNode& node, int axis, double splitPos)
{
	//split the tris into two groups, and sort along the way. Works like QuickSort
	int splitIdx = node.leftFirst;
	int maxIdx = node.leftFirst + node.numTris;
	while (splitIdx < maxIdx) {
		Vec3 centroid = tris[triIndices[splitIdx]].centroid;
		if (centroid[axis] < splitPos) {
			splitIdx++;
		}
		else {
			unsigned int temp = triIndices[splitIdx];
			triIndices[splitIdx] = triIndices[maxIdx - 1];
			triIndices[maxIdx - 1] = temp;
			maxIdx--;
		}
	}

	return splitIdx;
}

BVH::~BVH()
{
	delete[] nodes;
	delete[] triIndices;
	delete[] tris;
}

BVHInstance::BVHInstance(BVH* bvHeirarchy, MeshInstance* meshInstance) {
	Set(bvHeirarchy, meshInstance);
}

void BVHInstance::Set(BVH* bvHeirarchy, MeshInstance* meshInstance)
{
	bvh = bvHeirarchy;
	mesh = meshInstance;
	mesh->OnTransformSet = [this](Mat4 transform) {
		worldSpaceBounds = AABB(Vec3(FLT_MAX), Vec3(-FLT_MAX));
		for (int i = 0; i < 8; i++) {
			worldSpaceBounds.Grow(Mat4::Transform(
				Vec3(
					i & 1 ? bvh->GetBounds().max[0] : bvh->GetBounds().min[0],
					i & 2 ? bvh->GetBounds().max[1] : bvh->GetBounds().min[1],
					i & 4 ? bvh->GetBounds().max[2] : bvh->GetBounds().min[2]
				),
				transform
			));
		}
	};
}

TLAS::TLAS(BVH** bvhList, MeshInstance** meshInstances, int numMeshInstances)
{
	blas = new BVHInstance[numMeshInstances];
	for (int i = 0; i < numMeshInstances; i++) {
		blas[i].Set(bvhList[meshInstances[i]->meshRef->id], meshInstances[i]);
	}
	//meshes = meshInstances;
	//meshIndices = new int[numMeshInstances];
	//for (int i = 0; i < numBVH; i++) {
	//	blas[i].bvh = &(meshes[i]->meshRef->id);
	//	blas[i].meshInstance = meshes[i];
	//}
	//blas = bvhList;
	numBLAS = numMeshInstances;
	nodes = (TLASNode*)_aligned_malloc(sizeof(TLASNode) * 2 * numMeshInstances, 64);
	numNodes = numMeshInstances * 2; //for now
}

void TLAS::Rebuild() {
	nodes[0].min = Vec3(-100.0f);
	nodes[0].max = Vec3(100.0f);
	nodes[0].leftBLAS = 2;
	nodes[0].isLeaf = false;

	for (int i = 2; i < numBLAS * 2; i++) {
		nodes[i].min = Vec3(-100.0f);
		nodes[i].max = Vec3(100.0f);
		nodes[i].leftBLAS = i - 2;
		nodes[i].isLeaf = true;
	}
}

bool TLAS::CalculateIntersection(Ray& ray, HitInfo& out, unsigned int nodeIdx) {
	TLASNode* node = &nodes[0], * stack[64], * left, * right;
	unsigned int stackIdx = 0;
	bool hasHit = false;
	while (true)
	{
		if (node->isLeaf)
		{
			BVHInstance& bvhInstance = blas[node->leftBLAS];
			//MeshInstance& mesh = *meshes[node->leftBLAS];
			ray.origin = Camera::Get().position + bvhInstance.mesh->invTransform.GetTranslation();
			if (bvhInstance.bvh->CalculateIntersection(ray, out, 0)) {
				ray.maxDist = out.distance;
				hasHit = true;
			}
			if (stackIdx == 0) break; 
			else node = stack[--stackIdx];
			continue;
		}
		left = &nodes[node->leftBLAS];
		right = &nodes[node->leftBLAS + 1];

		float leftDist = FLT_MAX, rightDist = FLT_MAX;
		Ray::IntersectAABB(ray, left->min, left->max, leftDist);
		Ray::IntersectAABB(ray, right->min, right->max, rightDist);

		//intersect with neither 
		if (leftDist == FLT_MAX && rightDist == FLT_MAX)
		{
			if (stackIdx == 0) break;
			else node = stack[--stackIdx];
		}
		//intersection found, keep going down the tree
		//if intersected with the other child too, push it to the stack for later
		else {
			if (leftDist > rightDist) {
				node = right;
				if (leftDist != FLT_MAX) stack[stackIdx++] = left;
			}
			else {
				node = left;
				if (rightDist != FLT_MAX) stack[stackIdx++] = right;
			}
		}
	}
	return hasHit;
}


