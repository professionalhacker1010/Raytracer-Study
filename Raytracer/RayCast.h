#pragma once
#include "Vector.h"
#include <smmintrin.h>

struct HitInfo {
	Vec3 position;
	float distance = FLT_MAX;
	int triId = -1;
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

	union { struct { Vec3 origin; float maxDist; }; __m128 origin4; };
	union { struct { Vec3 direction; float pad1; }; __m128 direction4; };
	union { struct { Vec3 dInv; float pad2; }; __m128 dInv4; };
	
};