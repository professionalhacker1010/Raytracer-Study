#pragma once
#include "Vertex.h"

struct Ray;
struct HitInfo;

class Tri {
public:
	Tri() = default;

	/// <summary>
	/// whether the ray hits the triangle, at what distance and barycentric coords
	/// </summary>
	bool CalculateIntersection(const Ray& ray, HitInfo& out) const;

	/// <summary>
	/// interpolate vertex values based on triangle and barycentric coords
	/// </summary>
	void CalculateVertex(const Vec3& coord, Vertex& outVertex);

	void ParseFromFile(FILE* file);

	int debug();

	Vec3 verts[3];
	Vec3 centroid;
	//Vec3 normal;

	void CachedCalculations();
};
