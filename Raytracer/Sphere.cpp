#include "Sphere.h"
#include "RayCast.h"
#include "Vertex.h"
#include "util.h"

bool Sphere::CalculateIntersection(const Ray& ray, HitInfo* out) const
{
	Vec3 diffVec = Vec3(ray.origin[0] - position[0], ray.origin[1] - position[1], ray.origin[2] - position[2]);
	double a = 1.;
	double b = 2 * (Vec3::Dot(ray.direction, diffVec));
	double c = pow(diffVec[0], 2) + pow(diffVec[1], 2) + pow(diffVec[2], 2) - pow(radius, 2);

	double determinant = pow(b, 2) - 4 * a * c;

	if (determinant < 0.) return false;

	//quadratic formula
	double t0, t1;
	double sqrtDeterminant = sqrt(determinant);
	t0 = (-b + sqrtDeterminant) / 2.;
	t1 = (-b - sqrtDeterminant) / 2.;

	if (t0 < 0. || t1 < 0.) return false;

	//get min t
	double t = fmin(t0, t1);

	//get intersection point, store distnace
	Vec3 distVec;
	distVec = ray.direction * t;
	out->position = ray.origin + distVec;
	out->distance = t;

	return true;
}

void Sphere::CalculateVertex(const Vec3& position, Vertex& outVertex)
{
	outVertex.normal.Set((position[0] - this->position[0]) / radius,
		(position[1] - this->position[1]) / radius,
		(position[2] - this->position[2]) / radius);
	outVertex.color_diffuse = color_diffuse;
	outVertex.color_specular = color_specular;
	outVertex.position = position;
	outVertex.shininess = shininess;
	if (debug < 10) {
		//Util::Print(position);
		debug++;
	}
}

void Sphere::ParseFromFile(FILE* file, int id)
{
	Util::parse_doubles(file, "pos:", position);
	Util::parse_rad(file, &radius);
	Util::parse_doubles(file, "dif:", color_diffuse);
	Util::parse_doubles(file, "spe:", color_specular);
	Util::parse_shi(file, &shininess);

	this->id = id;
}