#pragma once
#include "Math.h"
#include <smmintrin.h>

struct AABB {
	AABB() {}
	AABB(Vec3 minBounds, Vec3 maxBounds) {
		min = minBounds;
		max = maxBounds;
	}

	void Grow(Vec3 point) {
		max = Vec3::Max(max, point);
		min = Vec3::Min(min, point);
	}

	void Grow(AABB& aabb) {
		max = Vec3::Max(max, aabb.max);
		min = Vec3::Min(min, aabb.min);
	}

	float Area() {
		Vec3 diff = max - min;
		Vec3 abs = diff.Abs();
		return _mm_mul_ps(_mm_dp_ps(_mm_set_ps(abs[0], abs[0], abs[1], 0.0f), _mm_set_ps(abs[1], abs[2], abs[2], 0.0f), 0x71), _mm_set_ps1(2.0f)).m128_f32[0];
		//float edge1 = (float)fabs(diff[0]);
		//float edge2 = (float)fabs(diff[1]);
		//float edge3 = (float)fabs(diff[2]);
		//return 2.0f * ((edge1 * edge2) + (edge1 * edge3) + (edge2 * edge3));
	}

	Vec3 min, max;
};