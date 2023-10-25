#pragma once
#include "Math.h"
struct Ray;
struct Vertex;
struct HitInfo;

class Sphere {
public:
	Sphere() {}

	/// <summary>
	/// Return true if intersection between Ray and Sphere found
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="out"></param>
	/// <returns></returns>
	bool CalculateIntersection(const Ray& ray, HitInfo* out) const;

	/// <summary>
	/// Calculate vertex values based on position on sphere
	/// </summary>
	/// <param name="position"></param>
	/// <param name="outVertex"></param>
	void CalculateVertex(const Vec3& position, Vertex& outVertex);

	void ParseFromFile(FILE* file, int id);

	Vec3 position;
	Vec3 color_diffuse;
	Vec3 color_specular;
	double shininess = 0;
	double radius = 0;
	int id = 0;

	int debug = 0;
};