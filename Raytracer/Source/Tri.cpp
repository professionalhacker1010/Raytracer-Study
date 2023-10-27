#include "Tri.h"
#include "RayCast.h"
#include "Util.h"
#include <atomic>

std::atomic_int intersectionChecks;

bool Tri::CalculateIntersection(const Ray& ray, HitInfo& out) const
{
	intersectionChecks++;

	//const Vec3 edge1 = verts[1].position - verts[0].position;
	//const Vec3 edge2 = verts[2].position - verts[0].position;
	const Vec3 h = Vec3::Cross(ray.direction, edge2);
	const float a = Vec3::Dot(edge1, h);
	if (abs(a) < 0.0001f) return false; // ray parallel to triangle
	const float f = 1 / a;
	const Vec3 s = ray.origin - verts[0];
	const float u = f * Vec3::Dot(s, h);
	if (u < 0 || u > 1) return false;
	const Vec3 q = Vec3::Cross(s, edge1);
	const float v = f * Vec3::Dot(ray.direction, q);
	if (v < 0 || u + v > 1) return false;
	const float t = f * Vec3::Dot(edge2, q);
	if (t > ray.maxDist) return false;
	if (t > 0.0001f) {
		out.distance = t;
		out.u = u;
		out.v = v;
		return true;
	}
	return false;
}

void Tri::CalculateVertex(const Vec3& coord, Vertex& outVertex)
{
	//interpolate vertex based on area
	//outVertex.color_diffuse =
	//	Vec3::BaryCoord(verts[2].color_diffuse, verts[1].color_diffuse, verts[0].color_diffuse, coord);
	//outVertex.color_specular =
	//	Vec3::BaryCoord(verts[2].color_specular, verts[1].color_specular, verts[0].color_specular, coord);
	//outVertex.normal =
	//	Vec3::BaryCoord(verts[2].normal, verts[1].normal, verts[0].normal, coord);
	//outVertex.position =
	//	Vec3::BaryCoord(verts[2].position, verts[1].position, verts[0].position, coord);
	//outVertex.shininess = Vec3::Dot(Vec3(verts[2].shininess, verts[1].shininess, verts[0].shininess), coord);
}

void Tri::ParseFromFile(FILE* file)
{
	for (int i = 0; i < 3; i++)
	{
		//Util::parse_doubles(file, "pos:", verts[i]);
		//Vec3 dummy;
		//Util::parse_doubles(file, "nor:", dummy);
		//Util::parse_doubles(file, "dif:", dummy);
		//Util::parse_doubles(file, "spe:", dummy);
		//double tempShi;
		//Util::parse_shi(file, &tempShi);
		//verts[i].shininess = (float)tempShi;
	}

	CachedCalculations();
	intersectionChecks = 0;
}

int Tri::debug()
{
	int retVal = intersectionChecks;
	intersectionChecks = 0;
	return retVal;
}

void Tri::CachedCalculations()
{
	edge1 = verts[1] - verts[0];
	edge2 = verts[2] - verts[0];

	//plane normal
	//normal = Vec3::Cross(edge1, edge2);
	//normal.Normalize();

	centroid = (verts[0] + verts[1] + verts[2]) / 3.0f;

	//	// perpendicular distance from origin to plane
	//	d = -Vec3::Dot(centroid - camera.position, normal);

	//if (Util::doubleCompare(verts[0].position[0], verts[1].position[0]) && Util::doubleCompare(verts[0].position[0], verts[2].position[0])) {
	//	dimOne = 2;
	//}
	//else if (Util::doubleCompare(verts[0].position[1], verts[1].position[1]) && Util::doubleCompare(verts[0].position[1], verts[2].position[1])) {
	//	dimTwo = 2;
	//}
	//proj1[0] = verts[0].position[dimOne]; proj1[1] = verts[0].position[dimTwo];
	//proj2[0] = verts[1].position[dimOne]; proj2[1] = verts[1].position[dimTwo];
	//proj3[0] = verts[2].position[dimOne]; proj3[1] = verts[2].position[dimTwo];

	//perpendicular distance from origin to plane
	//d = -Vec3::Dot(verts[0].position - cameraPos, normal);
}
