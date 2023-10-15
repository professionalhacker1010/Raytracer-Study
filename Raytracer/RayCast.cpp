#include "RayCast.h"
#include "Util.h"

float Ray::CalculatePlaneIntersection(const Vec3& norm, const Ray& ray, const float d)
{
	float denom = Vec3::Dot(norm, ray.direction);
	if (Util::FloatCompare(denom, 0.)) {
		return -1.; //ray parallel to plane
	}

	// distance from ray origin to plane
	return -(Vec3::Dot(norm, ray.origin) + d) / denom;
}

float Ray::IntersectAABB(const Ray& ray, Vec3 minBounds, Vec3 maxBounds)
{
	double tx1 = (minBounds[0] - ray.origin[0]) * ray.dInv[0], tx2 = (maxBounds[0] - ray.origin[0]) * ray.dInv[0];
	double tmin = fmin(tx1, tx2);
	double tmax = fmax(tx1, tx2);
	double ty1 = (minBounds[1] - ray.origin[1]) * ray.dInv[1], ty2 = (maxBounds[1] - ray.origin[1]) * ray.dInv[1];
	tmin = fmax(tmin, fmin(ty1, ty2)), tmax = fmin(tmax, fmax(ty1, ty2));
	double tz1 = (minBounds[2] - ray.origin[2]) * ray.dInv[2], tz2 = (maxBounds[2] - ray.origin[2]) * ray.dInv[2];
	tmin = fmax(tmin, fmin(tz1, tz2)), tmax = fmin(tmax, fmax(tz1, tz2));
	if (tmax >= tmin && tmin < ray.maxDist && tmax > 0.) return tmin;
	return -1;
}

bool Ray::IntersectAABB_SIMD(const Ray& ray, const __m128 bmin4, const __m128 bmax4, float& out)
{
	//mask out the uint bc calculations on it are slow
	static __m128 mask4 = _mm_cmpeq_ps(_mm_setzero_ps(), _mm_set_ps(1, 0, 0, 0)); 
	__m128 t1 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmin4, mask4), ray.origin4), ray.dInv4);
	__m128 t2 = _mm_mul_ps(_mm_sub_ps(_mm_and_ps(bmax4, mask4), ray.origin4), ray.dInv4);
	__m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
	float tmax = fmin(vmax4.m128_f32[0], fmin(vmax4.m128_f32[1], vmax4.m128_f32[2]));
	float tmin = fmax(vmin4.m128_f32[0], fmax(vmin4.m128_f32[1], vmin4.m128_f32[2]));
	if (tmax >= tmin && tmin < ray.maxDist && tmax > 0) {
		out = tmin;
		return true;
	}
	else return false;
}
