#include "Source/Constants.h"
#include "UtilCL.cl"

bool RayCast(
	struct Ray* ray, struct Vertex* outVertex, 
	__global struct TLASNode* tlas, 
	__global struct BVHInstance* blas, 
	__global struct BVHNode* bvh, __global uint* bvhTriIndices, 
	__global struct Tri* tris) 
{
	
	bool hit = IntersectTLAS(ray, tlas, blas, bvh, bvhTriIndices, tris);

	if (hit) {

		//position
		outVertex->position = ray->origin + ray->hit.distance * ray->direction;

		return true;
	}

	return false;
}

__constant float3 colors[] = {
	(float3)(1, 0, 0), //red
	(float3)(0, 1, 0), //green
	(float3)(0, 0, 1), //blue
	(float3)(1, 1, 0), //yellow
	(float3)(0, 1, 1), //teal
	(float3)(1, 0, 1), //pink
};

__kernel void render( 
	__global uint* target,
	__global float* skyPixels, 
	__global struct TLASNode* tlas,
	__global struct Tri* tris, __global struct TriVerts*  vertData, __global struct BVHNode* bvh, __global uint* bvhTriIndices,
	__global struct MeshInstance* meshInstances, __global struct BVHInstance* bvhInstances,
	float3 camPos, float3 camBottomLeft, float3 camLengthX, float3 camLengthY, 
	uint skyWidth, uint skyHeight
)
{

	int threadIdx = get_global_id(0);
	if (threadIdx >= WIDTH * HEIGHT) return;
	int x = threadIdx % WIDTH;
	int y = threadIdx / WIDTH;
	
	//calculate ray
	struct Ray ray;
	ray.hit.distance = FLT_MAX;
	ray.hit.triId = -1;
	ray.hit.meshInstId = -1;
	ray.origin = camPos;
	ray.direction = normalize(camBottomLeft + (camLengthY * ((float)y / (float)HEIGHT)) + (camLengthX * ((float)x / (float)WIDTH))); //get interpolated direction
	ray.dInv = (float3)(1, 1, 1) / ray.direction;

	struct Vertex vert;

	//raycast
	bool hit = RayCast(&ray, &vert, tlas, bvhInstances, bvh, bvhTriIndices, tris);

	float3 color = (float3)(1, 1, 1);
	if (hit) {
		color = colors[ray.hit.meshInstId % 6];
		if (ray.hit.meshInstId % 3 != 0) {
			//color = CastShadowRays(vert) * 255.0f;
		}
		else {
			//color = CastMirrorRays(ray, vert) * 255.0f;
		}
	}
	else {
		//draw skybox
		float phi = atan2(ray.direction.z, ray.direction.x);
		uint u = (uint)(skyWidth * (phi > 0 ? phi : (phi + 2 * PI)) * INV2PI - 0.5f);
		uint v = (uint)(skyHeight * acos(ray.direction.y) * INVPI - 0.5f);
		uint skyIdx = (u + v * skyWidth) % (skyWidth * skyHeight);

		color = 0.65f * (float3)(skyPixels[skyIdx * 3], skyPixels[skyIdx * 3 + 1], skyPixels[skyIdx * 3 + 2]);
	}


    // plot a pixel into the target array in GPU memory
	target[y * WIDTH + x] = RGB32FtoRGB8(color);
}