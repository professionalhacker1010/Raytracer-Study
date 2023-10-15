#pragma once
#include "Vertex.h"

struct Ray;
struct HitInfo;

class Tri {
public:
	Tri() { }

	/// <summary>
	/// whether the ray hits the triangle, at what distance and barycentric coords
	/// </summary>
	bool CalculateIntersection(const Ray& ray, HitInfo& out) const;

	/// <summary>
	/// interpolate vertex values based on triangle and barycentric coords
	/// </summary>
	void CalculateVertex(const Vec3& coord, Vertex& outVertex);

	void ParseFromFile(FILE* file, int id);

	int debug();

	Vertex verts[3];
	Vec3 centroid;
	Vec3 normal;
	float d;
	int id;

	unsigned int dimOne = 0, dimTwo = 1;
	float proj1[2], proj2[2], proj3[2];

private:
	void CachedCalculations();
};
