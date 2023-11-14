#include "Source/Constants.h"
#include "Source/cl/UtilCL.cl"

__constant struct Light lights[2] = {
	((float3)(3, 10, 2), (float3)(1, 1, 1)),
	((float3)(3, 10, 2), (float3)(1, 1, 1))
};
__constant float3 ambientLight = (float3)(60 / 255.0f, 56 / 255.0f, 79 / 255.0f) ;

__constant float3 colors[] = {
	(float3)(1, 0, 0), //red
	(float3)(0, 1, 0), //green
	(float3)(0, 0, 1), //blue
	(float3)(1, 1, 0), //yellow
	(float3)(0, 1, 1), //teal
	(float3)(1, 0, 1), //pink
};

bool RayCast(
	__global uint* texture, uint texWidth, uint texHeight,
	struct Ray* ray, struct Vertex* outVertex,
	__global struct TLASNode* tlas,
	__global struct BVHInstance* blas,
	__global struct BVHNode* bvh, __global uint* bvhTriIndices,
	__global struct Tri* tris, __global struct TriVerts* vertData,
	__global struct MeshInstance* meshInstances
	)
{

	bool hit = IntersectTLAS(ray, tlas, blas, bvh, bvhTriIndices, tris);

	if (hit) {
		__global struct MeshInstance* mesh = &meshInstances[ray->hit.meshInstId];
		__global struct TriVerts* triVerts = &vertData[ray->hit.triId];

		float3 coord = (float3)(ray->hit.u, ray->hit.v, 1 - (ray->hit.u + ray->hit.v));

		//normal
		float3 norm = BaryCoord3(
			triVerts->norm1_uv2y.xyz,
			triVerts->norm2.xyz,
			triVerts->norm0_uv2x.xyz,
			coord);
		norm = normalize(Transform(norm, &mesh->transform, 0.0f));
		outVertex->normal = norm;

		//texture
		float2 uv = BaryCoord2(triVerts->uv1, (float2)(triVerts->norm0_uv2x.w, triVerts->norm1_uv2y.w), triVerts->uv0, coord);
		int iu = (int)(uv[0] * (float)texWidth) % texWidth;
		int iv = (int)(uv[1] * (float)texHeight) % texHeight;
		unsigned int texel = texture[iu + iv * texWidth];
		outVertex->albedo = RGB8toRGB32F(texel);

		//position
		outVertex->position = ray->origin + ray->hit.distance * ray->direction + mesh->transform.sCDE; //transform to world space

		return true;
	}

	return false;
}

float3 CastShadowRays(
	struct Ray* ray, struct Vertex* vertex,
	__global struct TLASNode* tlas,
	__global struct BVHInstance* blas,
	__global struct BVHNode* bvh, __global uint* bvhTriIndices,
	__global struct Tri* tris)
{
	float3 color = ambientLight;

	struct Ray secondary;
	secondary.origin = vertex->position + vertex->normal * 0.001f;
	for (int i = 0; i < NUM_LIGHTS; i++) {
		//set direction and max dist of raycast
		float3 dir = lights[i].position - vertex->position;
		float dist = length(dir);
		dir *= 1.0f / dist;
		secondary.direction = dir;
		secondary.dInv = (float3)(1,1,1) / dir;

		//if the raycast hit something don't calculate lighting
		if (IntersectTLAS(&secondary, tlas, blas, bvh, bvhTriIndices, tris)) {
			continue;
		}

		//diffuse light
		float diffuse = max(dot(vertex->normal, secondary.direction), 0.0f);

		//final color
		color += (diffuse) * lights[i].color; //* (1.0f / (dist * dist));
	}
	color = color * vertex->albedo;

	//clamp color
	color = min(color, (float3)(1,1,1));

	//return final color
	return color;
}

float3 CastMirrorRays(
	__global float* skyPixels, __global uint* texture,
	struct Ray* ray, struct Vertex* vertex, 
	__global struct TLASNode* tlas,
	__global struct BVHInstance* blas,
	__global struct BVHNode* bvh, __global uint* bvhTriIndices,
	__global struct Tri* tris, __global struct TriVerts* vertData,
	__global struct MeshInstance* meshInstances,
	uint skyWidth, uint skyHeight, uint texWidth, uint texHeight)
{
	int depth = 0;
	struct Ray primary = *ray;
	while (depth < MAX_RAY_DEPTH) {
		struct Ray secondary;
		secondary.direction = normalize(primary.direction - 2 * vertex->normal * dot(vertex->normal, primary.direction));
		secondary.dInv = (float3)(1, 1, 1) / secondary.direction;
		secondary.origin = vertex->position + secondary.direction * 0.001f;
		secondary.hit.distance = FLT_MAX;

		int hit = RayCast(texture, texWidth, texHeight, &secondary, vertex, tlas, blas, bvh, bvhTriIndices, tris, vertData, meshInstances);

		if (hit) {
			if (secondary.hit.meshInstId % 3 != 0) {
				return CastShadowRays(&ray, &vertex, tlas, blas, bvh, bvhTriIndices, tris);
			}
			else {
				depth++;
				primary = secondary;
				continue;
			}
		}
		else {
			return DrawSkybox(skyPixels, &secondary, skyWidth, skyHeight);
		}
	}

	return (float3)(0, 0, 0);
}

__kernel void render( 
	__global uint* target, __global uint* texture,
	__global float* skyPixels, 
	__global struct TLASNode* tlas,
	__global struct Tri* tris, __global struct TriVerts*  vertData, __global struct BVHNode* bvh, __global uint* bvhTriIndices,
	__global struct MeshInstance* meshInstances, __global struct BVHInstance* bvhInstances,
	float3 camPos, float3 camBottomLeft, float3 camLengthX, float3 camLengthY, 
	uint skyWidth, uint skyHeight, uint texWidth, uint texHeight
)
{

	int threadIdx = get_global_id(0);
	if (threadIdx >= WIDTH * HEIGHT) return;
	int x = threadIdx % WIDTH;
	int y = threadIdx / WIDTH;
	
	//jittered sampling
	float3 color = (float3)(0, 0, 0);
	int dimension = 2; //4 samples in 2x2 grid
	int samples = 4;
	uint seed = WangHash(threadIdx * 17 + 1);
	struct Ray ray;

	for (int s = 0; s < dimension; s++) {
		for (int t = 0; t < dimension; t++) {
			float u = ((float)s + RandomFloat(&seed)) / (float)dimension;
			float v = ((float)t + RandomFloat(&seed)) / (float)dimension;

			//calculate ray
			ray.hit.distance = FLT_MAX;
			ray.hit.triId = -1;
			ray.hit.meshInstId = -1;
			ray.origin = camPos;
			ray.direction = normalize(
				camBottomLeft +
				(camLengthY * (((float)y + u) / (float)HEIGHT)) +
				(camLengthX * (((float)x + v) / (float)WIDTH))); //get interpolated direction
			ray.dInv = (float3)(1, 1, 1) / ray.direction;

			struct Vertex vert;

			//raycast
			bool hit = RayCast(texture, texWidth, texHeight, &ray, &vert, tlas, bvhInstances, bvh, bvhTriIndices, tris, vertData, meshInstances);


			if (hit) {
				if (ray.hit.meshInstId % 3 != 0) {
					color += CastShadowRays(&ray, &vert, tlas, bvhInstances, bvh, bvhTriIndices, tris);
				}
				else {
					color += CastMirrorRays(skyPixels, texture, &ray, &vert, tlas, bvhInstances, bvh, bvhTriIndices, tris, vertData, meshInstances, skyWidth, skyHeight, texWidth, texHeight);
				}
			}
			else {
				color += DrawSkybox(skyPixels, &ray, skyWidth, skyHeight);
			}
		}
	}

	color /= samples;


    // plot a pixel into the target array in GPU memory
	target[y * WIDTH + x] = RGB32FtoRGB8(color);
}