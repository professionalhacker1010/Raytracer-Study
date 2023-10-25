#pragma once
#include "Math.h"
#include <smmintrin.h>

struct HitInfo {
	float distance = FLT_MAX;
	float u, v; //barycentric coords
	short triId = -1;
	short meshInstId = -1;
	//class Tri* hit = nullptr;
};

struct Ray {
	Ray() {
		maxDist = FLT_MAX;
		origin4 = direction4 = dInv4 = _mm_set1_ps(1);
	}
	Ray(const Ray& ray) {
		origin4 = ray.origin4;
		direction4 = ray.direction4;
		dInv4 = ray.dInv4;
		maxDist = ray.maxDist;
	}
	Ray(Vec3 origin, Vec3 direction, float distance = FLT_MAX) {
		this->origin = origin;
		this->direction = direction;
		this->maxDist = distance;
		dInv = Vec3::One() / direction; //Vec3(1.0f / direction[0], 1.0f / direction[1], 1.0f / direction[2]);
	}

	void Set(Vec3 origin, Vec3 direction, float distance = FLT_MAX) {
		this->origin = origin;
		this->direction = direction;
		this->maxDist = distance;
		dInv = Vec3::One() / direction; //Vec3(1.0f / direction[0], 1.0f / direction[1], 1.0f / direction[2]);
	}

	static float CalculatePlaneIntersection(const Vec3& norm, const Ray& ray, const float d);
	static bool IntersectAABB(const Ray& ray, Vec3 minBounds, Vec3 maxBounds, float& out);
	static bool IntersectAABB_SIMD(const Ray& ray, const __m128 bmin4, const __m128 bmax4, float& out);

	union { Vec3 origin; struct {  float pad0, pad1, pad2, maxDist; }; __m128 origin4; };
	union { Vec3 direction; __m128 direction4; };
	union { Vec3 dInv;  __m128 dInv4; };
	
};