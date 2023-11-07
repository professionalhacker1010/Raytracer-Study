#pragma once


struct HitInfo {
	float distance;
	float u, v; 
	short triId;
	short meshInstId;
};

struct Ray {
	float3 origin;
	float3 direction;
	float3 dInv;
	struct HitInfo hit;
};

struct Vertex
{
	float3 position;
	float3 albedo;
	float3 normal;
};

struct Tri {
	float3 vert0, vert1, vert2;
	float3 centroid;
};

struct TriVerts {
	float3 norm0, norm1, norm2; //48
	float2 uv0, uv1, uv2, pad0; //80
};

struct MeshInstance {
	float16 transform; //64 bytes
	int meshId;
	int id;
	float pad0, pad1; //80
};

struct BVHNode {
	float minX, minY, minZ;
	uint leftFirst; //16
	float maxX, maxY, maxZ;
	uint numTris; //32
};

struct BVHInstance {
	float16 invTransform; //64
	float3 min, max; //96
	int bvhId;
	int meshInstId;
	int pad[6]; //124
};

struct TLASNode {
	float minX, minY, minZ;
	uint leftRight;
	float maxX, maxY, maxZ;
	uint BLAS;
};


inline uint RGB32FtoRGB8(float3 c)
{
	int r = (int)(min(c.x, 1.f) * 255);
	int g = (int)(min(c.y, 1.f) * 255);
	int b = (int)(min(c.z, 1.f) * 255);
	return (b << 16) + (g << 8) + r;
}

inline float3 RGB8toRGB32F(uint c)
{
	float s = 1 / 256.0f;
	int r = (c >> 16) & 255;
	int g = (c >> 8) & 255;
	int b = c & 255;
	return (float3)(r * s, g * s, b * s);
}

float3 Transform(float3 vec, float16 mat, float w) {
	//float4 v = (float4)(w, vec[2], vec[1], vec[0]);
	float4 v = (float4)(vec[0], vec[1], vec[2], w);

	return (float3)(
		dot(v, (float4)(mat[0], mat[4], mat[8], mat[12])),
		dot(v, (float4)(mat[1], mat[5], mat[9], mat[13])),
		dot(v, (float4)(mat[2], mat[6], mat[10], mat[14]))
		);
}

float IntersectAABB(struct Ray* ray, float3 minBounds, float3 maxBounds) {
	float tx1 = (minBounds[0] - ray->origin[0]) * ray->dInv[0];
	float tx2 = (maxBounds[0] - ray->origin[0]) * ray->dInv[0];
	float tmin = (float)min(tx1, tx2);
	float tmax = (float)max(tx1, tx2);
	float ty1 = (minBounds[1] - ray->origin[1]) * ray->dInv[1];
	float ty2 = (maxBounds[1] - ray->origin[1]) * ray->dInv[1];
	tmin = (float)max(tmin, min(ty1, ty2));
	tmax = (float)min(tmax, max(ty1, ty2));
	float tz1 = (minBounds[2] - ray->origin[2]) * ray->dInv[2];
	float tz2 = (maxBounds[2] - ray->origin[2]) * ray->dInv[2];
	tmin = (float)max(tmin, min(tz1, tz2));
	tmax = (float)min(tmax, max(tz1, tz2));
	if (tmax >= tmin && tmin < ray->hit.distance && tmax > 0.0f) {
		return tmin;
	}
	return FLT_MAX;
}

bool IntersectTri(
	struct Ray* ray,
	__global struct Tri* tri
)
{
	const float3 edge1 = tri->vert1 - tri->vert0;
	const float3 edge2 = tri->vert2 - tri->vert0;
	const float3 h = cross(ray->direction, edge2);
	const float a = dot(edge1, h);
	if (a > -0.00001f && a < 0.00001f) return false; // ray parallel to triangle
	const float f = 1 / a;
	const float3 s = ray->origin - tri->vert0;
	const float u = f * dot(s, h);
	if (u < 0 || u > 1) return false;
	const float3 q = cross(s, edge1);
	const float v = f * dot(ray->direction, q);
	if (v < 0 || u + v > 1) return false;
	const float t = f * dot(edge2, q);
	if (t > ray->hit.distance) return false;
	if (t > 0.0001f) {
		ray->hit.distance = t;
		ray->hit.u = u;
		ray->hit.v = v;
		return true;
	}
	return false;
}

bool IntersectBVH(
	struct Ray* ray,
	__global struct BVHNode* bvh, __global uint* bvhTriIndices,
	__global struct Tri* tris)
{
	__global struct BVHNode* node = &bvh[0], * left, * right, * stack[MAX_BVH_STACK];
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
				if (IntersectTri(ray, &tris[bvhTriIndices[i]])) {
					ray->hit.triId = (short)bvhTriIndices[i];
					//ray.maxDist = out.distance;
					hasHit = true;
				}
				//	else falseBranch++;
			}
			if (stackIdx == 0) break;
			else node = stack[--stackIdx];
			continue;
		}
		left = &bvh[node->leftFirst];
		right = &bvh[node->leftFirst + 1];

		float leftDist = FLT_MAX;
		float rightDist = FLT_MAX;
		leftDist = IntersectAABB(ray, (float3)(left->minX, left->minY, left->minZ), (float3)(left->maxX, left->maxY, left->maxZ));
		rightDist = IntersectAABB(ray, (float3)(right->minX, right->minY, right->minZ), (float3)(right->maxX, right->maxY, right->maxZ));

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

bool IntersectBVHInstance(
	struct Ray* ray,
	__global struct BVHInstance* blas,
	__global struct BVHNode* bvh, __global uint* bvhTriIndices,
	__global struct Tri* tris)
{
	//transform the ray to do intersection on the un-transformed bvh
	struct Ray backupRay;
	backupRay.origin = ray->origin;
	backupRay.direction = ray->direction;
	backupRay.dInv = ray->dInv;

	ray->origin = Transform(ray->origin, blas->invTransform, 1.0f);
	ray->direction = Transform(ray->direction, blas->invTransform, 0.0f);
	ray->dInv = (float3)(1, 1, 1) / ray->direction;

	bool hit = IntersectBVH(ray, bvh, bvhTriIndices, tris);

	// restore ray origin and direction
	ray->dInv = backupRay.dInv;
	ray->direction = backupRay.direction;
	ray->origin = backupRay.origin;

	if (hit) ray->hit.meshInstId = (short)blas->meshInstId;

	return hit;
}

bool IntersectTLAS(
	struct Ray* ray,
	__global struct TLASNode* tlas,
	__global struct BVHInstance* blas,
	__global struct BVHNode* bvh, __global uint* bvhTriIndices,
	__global struct Tri* tris)
{
	__global struct TLASNode* node = &tlas[0], * stack[64], * left, * right;
	unsigned int stackIdx = 0;
	bool hasHit = false;
	while (true)
	{
		if (node->leftRight == 0)
		{
			if (IntersectBVHInstance(ray, &blas[node->BLAS], bvh, bvhTriIndices, tris)) {
				hasHit = true;
			}
			if (stackIdx == 0) break;
			else node = stack[--stackIdx];
			continue;
		}
		left = &tlas[(node->leftRight & 0xffff)];
		right = &tlas[(node->leftRight >> 16)];

		float leftDist = FLT_MAX, rightDist = FLT_MAX;
		leftDist = IntersectAABB(ray, (float3)(left->minX, left->minY, left->minZ), (float3)(left->maxX, left->maxY, left->maxZ));
		rightDist = IntersectAABB(ray, (float3)(right->minX, right->minY, right->minZ), (float3)(right->maxX, right->maxY, right->maxZ));

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