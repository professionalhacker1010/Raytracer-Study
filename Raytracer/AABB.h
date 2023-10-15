#pragma once
#include "Vector.h"
#include <smmintrin.h>

struct AABB {
	AABB() {}
	AABB(Vec3 minBounds, Vec3 maxBounds) {
		min = minBounds;
		max = maxBounds;
	}

	void Grow(Vec3 point) {
		max = Vec3::Max(point, max);
		min = Vec3::Min(point, min);
	}

	void Grow(AABB& aabb) {
		max = Vec3::Max(max, aabb.max);
		min = Vec3::Min(min, aabb.min);
	}

	float Area() {
		Vec3 diff = max - min;
		float edge1 = fabs(diff[0]);
		float edge2 = fabs(diff[1]);
		float edge3 = fabs(diff[2]);
		//return _mm_dp_ps(_mm_set_ps(edge1, edge1, edge2, 0.0f), _mm_set_ps(edge2, edge3, edge3, 0.0f), 0x71).m128_f32[0] * 2.0f;
		return 2.0f * ((edge1 * edge2) + (edge1 * edge3) + (edge2 * edge3));
	}

	Vec3 min, max;
};