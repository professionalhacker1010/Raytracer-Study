/*
CSCI 420
Assignment 3 Raytracer

Name: <Your name here>
*/

#include <windows.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdio.h>
#include <string>
#include <math.h>

#include "opencv2/core/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"

#include <map>
#include "util.h"

#define PI 3.14159265358979323
#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10

char *filename=0;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode=MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0

unsigned char buffer[HEIGHT][WIDTH][3];
const double Vector0[3] = { 0., 0., 0. };
const double Vector1[3] = { 1., 1., 1. };

struct Vertex
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double normal[3];
	double shininess;
};


typedef struct _Triangle
{
  struct Vertex v[3];
  double normal[3];
  double d;
  int id;
} Triangle;

typedef struct _Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
  int id;
} Sphere;

typedef struct _Light
{
  double position[3];
  double color[3];
} Light;

typedef struct _Ray
{
	void set(const double o[3], const double d[3], const double dist = 100) {
		Util::setVec3(origin, o);
		Util::setVec3(direction, d);
		maxDist = dist;
	}
	double origin[3];
	double direction[3];
	double maxDist;
} Ray;

typedef struct _HitInfo
{
	double position[3];
	double distance;
} HitInfo;

enum NodePos {
	FRONT_LEFT_TOP, FRONT_RIGHT_TOP,
	FRONT_LEFT_BOT, FRONT_RIGHT_BOT,
	BACK_LEFT_TOP, BACK_RIGHT_TOP,
	BACK_LEFT_BOT, BACK_RIGHT_BOT,
};

enum CubeFace {
	FRONT, LEFT, BOTTOM,
	BACK, RIGHT, TOP,
	NONE
};

double cubeNormals[6][3] = {
	{0, 0, 1 }, // Z
	{-1, 0, 0}, // -X
	{0, -1, 0}, // -Y
	{0, 0, -1 }, // -Z
	{1, 0, 0}, // X
	{0, 1, 0} // Y
};

typedef struct Node {
	Node* children[8];
	Node* parent;
	std::map<int, Triangle*> tris;
	double d[6];
	double maxBounds[3];
	double minBounds[3];
	double center[3];
	NodePos id;
};

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles=0;
int num_spheres=0;
int num_lights=0;

double cameraRayDirections[WIDTH][HEIGHT][3];
Ray cameraRay;
double cameraPos[3] = { 0., 0., 0. };

Node* octreeRoot;
double maxBounds[3] = { 10., 10., -1. };
double minBounds[3] = { -10., -10., -21. };
const double minSize = 0.625;

double eps = 0.01;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);
bool calculateTriangleIntersection(const Triangle& tri, const Ray& ray, HitInfo* out);
void calculateCubeIntersection(const double min[3], const double max[3], const double d[6], const Ray& ray, HitInfo* out);

void initOctree(Node* root) {
	double center[3] = {
	root->minBounds[0] + 0.5 * (root->maxBounds[0] - root->minBounds[0]),
	root->minBounds[1] + 0.5 * (root->maxBounds[1] - root->minBounds[1]),
	root->minBounds[2] + 0.5 * (root->maxBounds[2] - root->minBounds[2])
	};
	Util::setVec3(root->center, center);

	if (root->maxBounds[0] - root->minBounds[0] < minSize) {
		return;
	}

	//create child nodes
	for (int i = 0; i < 8; i++) {
		Node* child = new Node();

#pragma region set bounds
		switch (i) {
		case FRONT_LEFT_TOP:
			Util::setVec3(child->maxBounds, center[0], root->maxBounds[1], center[2]);
			Util::setVec3(child->minBounds, root->minBounds[0], center[1], root->minBounds[2]);
			break;
		case FRONT_RIGHT_TOP:
			Util::setVec3(child->maxBounds, root->maxBounds[0], root->maxBounds[1], center[2]);
			Util::setVec3(child->minBounds, center[0], center[1], root->minBounds[2]);
			break;
		case FRONT_LEFT_BOT:
			Util::setVec3(child->maxBounds, center);
			Util::setVec3(child->minBounds, root->minBounds);
			break;
		case FRONT_RIGHT_BOT:
			Util::setVec3(child->maxBounds, root->maxBounds[0], center[1], center[2]);
			Util::setVec3(child->minBounds, center[0], root->minBounds[1], root->minBounds[2]);
			break;
		case BACK_LEFT_TOP:
			Util::setVec3(child->maxBounds, center[0], root->maxBounds[1], root->maxBounds[2]);
			Util::setVec3(child->minBounds, root->minBounds[0], center[1], center[2]);
			break;
		case BACK_RIGHT_TOP:
			Util::setVec3(child->maxBounds, root->maxBounds);
			Util::setVec3(child->minBounds, center);
			break;
		case BACK_LEFT_BOT:
			Util::setVec3(child->maxBounds, center[0], center[1], root->maxBounds[2]);
			Util::setVec3(child->minBounds, root->minBounds[0], root->minBounds[1], center[2]);
			break;
		case BACK_RIGHT_BOT:
			Util::setVec3(child->maxBounds, root->maxBounds[0], center[1], root->maxBounds[2]);
			Util::setVec3(child->minBounds, center[0], root->minBounds[1], center[2]);
			break;
		}
#pragma endregion

		//assign all geometry within region
		std::map<int, Triangle*>::iterator it = root->tris.begin();
		while (it != root->tris.end()) {

			int inBoundsCount = 0;
			for (int v = 0; v < 3; v++) {
				if (Util::withinBounds(it->second->v[v].position, child->maxBounds, child->minBounds)) {
					inBoundsCount++;
				}
			}

			if (inBoundsCount > 0)	child->tris.insert(std::pair<int, Triangle*>(it->first, it->second));

			++it;
		}

		//set pointers
		root->children[i] = child;
		child->parent = root;

		//if (Util::doubleCompare(0.625, root->maxBounds[0] - root->minBounds[0])) printf("leaf node %i\n", child->tris.size());
		//else printf("node%i %i\n", i, child->tris.size());
		//if (child->tris.size() > 0) {
		//	printf("node%i %i\n", i, child->tris.size());
		//	printVec3(child->minBounds, "min");
		//	printVec3(child->maxBounds, "max");
		//}


		//recurse
		initOctree(child);
	}
}

void cleanOctree(Node* root) {
	if (root->children[0] == nullptr) return;

	int emptyCount = 0;
	for (int i = 0; i < 8; i++) {
		if (root->children[i]->tris.empty()) emptyCount++;
	}

	//if (emptyCount == 8) {
	//	for (int i = 0; i < 8; i++) {
	//		delete root->children[i];
	//		root->children[i] = nullptr;
	//	}
	//}
	//else {

	//}

	//precalculate d
	for (int i = 0; i < 6; i++) {
		if (i < 3) root->d[i] = -Util::dotVec3(Util::subVec3(root->minBounds, cameraPos), cubeNormals[i]);
		else root->d[i] = -Util::dotVec3(Util::subVec3(root->maxBounds, cameraPos), cubeNormals[i]);
	}

	for (int i = 0; i < 8; i++) {
		cleanOctree(root->children[i]);
	}
}


Node* findOctreeNode(Node* root, double pos[3]) {
	if (root->children[0] == nullptr) return root;
	//if (root->tris.empty()) return root;

	if (Util::withinBounds(pos, root->maxBounds, root->minBounds)) {
		if (pos[0] > root->center[0]) {
			if (pos[1] > root->center[1]) {
				if (pos[2] > root->center[2]) return findOctreeNode(root->children[(int)BACK_RIGHT_TOP], pos);
				else return findOctreeNode(root->children[(int)FRONT_RIGHT_TOP], pos);
			}
			else {
				if (pos[2] > root->center[2]) return findOctreeNode(root->children[(int)BACK_RIGHT_BOT], pos);
				else return findOctreeNode(root->children[(int)FRONT_RIGHT_BOT], pos);
			}
		}
		else {
			if (pos[1] > root->center[1]) {
				if (pos[2] > root->center[2]) return findOctreeNode(root->children[(int)BACK_LEFT_TOP], pos);
				else return findOctreeNode(root->children[(int)FRONT_LEFT_TOP], pos);
			}
			else {
				if (pos[2] > root->center[2]) return findOctreeNode(root->children[(int)BACK_LEFT_BOT], pos);
				else return findOctreeNode(root->children[(int)FRONT_LEFT_BOT], pos);
			}
		}
	}
	else {
		//printf("not in bounds\n");
		//printVec3(pos, "pos");
		//printVec3(root->minBounds, "min");
		//printVec3(root->maxBounds, "max");
		//printf("\n");
	}


	return nullptr;
}

Triangle* rayCastOctree(Node* root, const Ray& ray, HitInfo* out, int ignoreID = 0) {
	double epsilon[3] = { ray.direction[0] * eps, ray.direction[1] * eps, ray.direction[2] * eps };
	Ray testRay;
	testRay.set(ray.origin, ray.direction);
	Node* closestNode = root;
	Triangle* closestTri = nullptr;

	//find start and end intersection with root cube
	HitInfo frontHit, backHit;
	if (!Util::withinBounds(ray.origin, root->maxBounds, root->minBounds)) {
		calculateCubeIntersection(root->minBounds, root->maxBounds, root->d, ray, &frontHit);
	}
	else {
		Util::setVec3(frontHit.position, ray.origin);
		frontHit.distance = 0;
	}
	Util::setVec3(testRay.origin, Util::addVec3(epsilon, frontHit.position));
	calculateCubeIntersection(root->minBounds, root->maxBounds, root->d, testRay, &backHit);

	//step through octree
	while (closestTri == nullptr && !Util::doubleCompare(frontHit.distance, backHit.distance) && frontHit.distance < ray.maxDist) {

		//find closest intersection in tree
		Util::setVec3(frontHit.position, Util::addVec3(epsilon, frontHit.position));
		
		closestNode = findOctreeNode(root, frontHit.position);
		if (!closestNode) return false;
		//printVec3(frontHit.position, "test pos");
		//printVec3(closestNode->minBounds, "min");
		//printVec3(closestNode->maxBounds, "max");


		//find closest triangle intersection
		std::map<int, Triangle*>::iterator it;
		for (it = closestNode->tris.begin(); it != closestNode->tris.end(); ++it) {

			if (it->first == ignoreID) continue;

			HitInfo hitInfo; //barycentric coords + distance from ray origin
			if (calculateTriangleIntersection(*(it->second), ray, &hitInfo)) {

				//check if within raycast length
				if (hitInfo.distance > ray.maxDist) continue;

				//check if new closest triangle
				if (!closestTri || hitInfo.distance < out->distance) {
					
					closestTri = it->second;
					Util::setVec3(out->position, hitInfo.position);
					out->distance = hitInfo.distance;
				}
			}

		}

		//if no found continue to next octree node
		double tempDist = frontHit.distance;
		Util::setVec3(testRay.origin, frontHit.position);
		calculateCubeIntersection(closestNode->minBounds, closestNode->maxBounds, closestNode->d, testRay, &frontHit);
		//printVec3(frontHit.position);
		//printDouble(frontHit.distance);
		//printf("\n");
		frontHit.distance += tempDist;
	}

	return closestTri;
}

double calculatePlaneIntersection(const double norm[3], const Ray& ray, const double d) {
	double denom = Util::dotVec3(norm, ray.direction);
	if (Util::doubleCompare(denom, 0.)) {
		return -1.; //ray parallel to plane
	}

	// distance from ray origin to plane
	return -(Util::dotVec3(norm, ray.origin) + d) / denom;
}

void calculateCubeIntersection(const double min[3], const double max[3], const double d[6], const Ray& ray, HitInfo* out) {
	out->distance = 10000;

	for (int i = 0; i < 6; i++) {
		// get distance from ray origin to plane
		double t;
		double d;
		if (i < 3) d = -Util::dotVec3(Util::subVec3(min, cameraPos), cubeNormals[i]);
		else d = -Util::dotVec3(Util::subVec3(max, cameraPos), cubeNormals[i]);

		t = calculatePlaneIntersection(cubeNormals[i], ray, d);
		if (t < 0.) continue; //intersection behind ray origin

		// determine if new closest intersection
		if (t < out->distance) {
			out->distance = t;
			Util::setVec3(out->position, Util::mulVec3(ray.direction, t));
			Util::setVec3(out->position, Util::addVec3(out->position, ray.origin));
		}
	}
}

void init()
{
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, WIDTH, 0, HEIGHT, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// precalculate triangle constants
	double cameraDir[3] = { 0., 0., -1. };
	for (int i = 0; i < num_triangles; i++) {

		double planeVec1[3], planeVec2[3];
		Util::setVec3(planeVec1, Util::subVec3(triangles[i].v[1].position, triangles[i].v[0].position));
		Util::setVec3(planeVec2, Util::subVec3(triangles[i].v[2].position, triangles[i].v[1].position));
		Util::setVec3(triangles[i].normal, Util::crossVec3(planeVec1, planeVec2));
		Util::setVec3(triangles[i].normal, Util::normalizeVec3(triangles[i].normal));

		// perpendicular distance from origin to plane
		triangles[i].d = -Util::dotVec3(Util::subVec3(triangles[i].v[0].position, cameraPos), triangles[i].normal);
	}

	// precalculate camera ray directions
	double aspectRatio = (double) WIDTH / (double) HEIGHT;
	double tanHalfFOV = tan(((double) fov / 2.) * ((double) PI / 180.));
	double xMax = aspectRatio * tanHalfFOV;
	double xMin = -aspectRatio * tanHalfFOV;
	double yMax = tanHalfFOV;
	double yMin = -tanHalfFOV;

	//treat top left as 0,0
	for (int h = 0; h < HEIGHT; h++) {
		double y = yMin + (2 * yMax * h / (double) HEIGHT);
		for (int w = 0; w < WIDTH; w++) {
			double x = xMin + (2 * xMax * w / (double) WIDTH);
			double dir[3] = { x - cameraPos[0], y - cameraPos[1], -1. - cameraPos[2] };
			Util::setVec3(cameraRayDirections[w][h], Util::normalizeVec3(dir));
		}
	}

	//set up octree
	octreeRoot = new Node();
	octreeRoot->parent = nullptr;
	Util::setVec3(octreeRoot->maxBounds, maxBounds);
	Util::setVec3(octreeRoot->minBounds, minBounds);
	for (int i = 0; i < num_triangles; i++) {
		octreeRoot->tris[triangles[i].id] = &triangles[i];
	}
	initOctree(octreeRoot);
	cleanOctree(octreeRoot);
}

/// <summary>
/// return true + the position and distance of the ray intersection, false if no intersection found
/// </summary>
/// <param name="sphere"></param>
/// <returns></returns>
bool calculateSphereIntersection(const Sphere& sphere, const Ray& ray, HitInfo* out) {
	double diffVec[3] = { ray.origin[0] - sphere.position[0], ray.origin[1] - sphere.position[1], ray.origin[2] - sphere.position[2] };
	double a = 1.;
	double b = 2 * (Util::dotVec3(ray.direction, diffVec));
	double c = pow(diffVec[0], 2) + pow(diffVec[1], 2) + pow(diffVec[2], 2) - pow(sphere.radius, 2);

	double determinant = pow(b, 2) - 4 * a * c;

	if (determinant < 0.) return false;

	//quadratic formula
	double t0, t1;
	double sqrtDeterminant = sqrt(determinant);
	t0 = (-b + sqrtDeterminant) / 2.;
	t1 = (-b - sqrtDeterminant) / 2.;

	if (t0 < 0. || t1 < 0.) return false;

	//get min t
	double t = Util::min(t0, t1);

	//get intersection point, store distnace
	double distVec[3];
	Util::setVec3(distVec, Util::mulVec3(ray.direction, t));
	Util::setVec3(out->position, Util::addVec3(ray.origin, distVec));
	out->distance = t;

	return true;
}

/// <summary>
/// calculate vertex values based on position on sphere
/// </summary>
/// <param name="sphere"></param>
/// <param name="position"></param>
/// <param name="outVertex"></param>
int debug = 0;
void calculateSphereVertex(const Sphere& sphere, const double position[3], Vertex* outVertex) {
	double normal[3];
	Util::setVec3(normal,
		(position[0] - sphere.position[0]) / sphere.radius, 
		(position[1] - sphere.position[1]) / sphere.radius, 
		(position[2] - sphere.position[2]) / sphere.radius);
	Util::setVec3(outVertex->normal, normal);
	Util::setVec3(outVertex->color_diffuse, sphere.color_diffuse);
	Util::setVec3(outVertex->color_specular, sphere.color_specular);
	Util::setVec3(outVertex->position, position);
	outVertex->shininess = sphere.shininess;
	if (debug < 10) {
		Util::printVec3(outVertex->color_diffuse);
		debug++;
	}
}

/// <summary>
/// return true + the barycentric coords and distance of the ray intersection, false if no intersection found
/// </summary>
/// <param name="tri"></param>
/// <param name="ray"></param>
/// <param name="hitInfo"></param>
/// <returns></returns>
bool calculateTriangleIntersection(const Triangle& tri, const Ray& ray, HitInfo* out) {

	// distance from ray origin to plane
	double t = calculatePlaneIntersection(tri.normal, ray, tri.d);

	if (t < 0.) return false; // intersection behind ray origin

	//project onto plane not perpendicular to triangle
	double cross[3];
	double dot1[3];
	double dot2[3];
	Util::setVec3(dot1, Util::subVec3(tri.v[0].position, tri.v[1].position));
	Util::setVec3(dot2, Util::subVec3(tri.v[1].position, tri.v[2].position));
	Util::setVec3(cross, Util::crossVec3(dot1, dot2));

	int dimOne = 0, dimTwo = 1;
	if (Util::doubleCompare(tri.v[0].position[0], tri.v[1].position[0]) && Util::doubleCompare(tri.v[0].position[0], tri.v[2].position[0])) {
		dimOne = 2;
	}
	else if (Util::doubleCompare(tri.v[0].position[1], tri.v[1].position[1]) && Util::doubleCompare(tri.v[0].position[1], tri.v[2].position[1])) {
		dimTwo = 2;
	}
	double proj1[2] = { tri.v[0].position[dimOne], tri.v[0].position[dimTwo] };
	double proj2[2] = { tri.v[1].position[dimOne], tri.v[1].position[dimTwo] };
	double proj3[2] = { tri.v[2].position[dimOne], tri.v[2].position[dimTwo] };
	double projRay[2] = { ray.origin[dimOne] + (ray.direction[dimOne] * t), ray.origin[dimTwo] + (ray.direction[dimTwo] * t) };

	//calc areas with barycentric coordinates
	double areaTri, areaA, areaB, areaC;
	areaTri = Util::calcTriArea2D(proj1, proj2, proj3);
	areaA = Util::calcTriArea2D(proj1, proj2, projRay);
	areaB = Util::calcTriArea2D(proj1, projRay, proj3);
	areaC = Util::calcTriArea2D(projRay, proj2, proj3);

	// point not within triangle
	if (!Util::doubleCompare(areaA + areaB + areaC, areaTri)) {
		return false;
	}

	//store coords and distance
	out->position[0] = areaA / areaTri;
	out->position[1] = areaB / areaTri;
	out->position[2] = areaC / areaTri;
	out->distance = t;

	return true;
}

/// <summary>
/// interpolate vertex values based on triangle and barycentric coords
/// </summary>
/// <param name="tri"></param>
/// <param name="coord"></param>
/// <param name="outVertex"></param>
void calculateTriVertex(const Triangle& tri, const double coord[3], Vertex* outVertex) {
	//interpolate vertex based on area
	Util::setVec3(outVertex->color_diffuse, 
		Util::lerpVec3_3Point(tri.v[2].color_diffuse, tri.v[1].color_diffuse, tri.v[0].color_diffuse, coord[0], coord[1], coord[2]));
	Util::setVec3(outVertex->color_specular,
		Util::lerpVec3_3Point(tri.v[2].color_specular, tri.v[1].color_specular, tri.v[0].color_specular, coord[0], coord[1], coord[2]));
	Util::setVec3(outVertex->normal,
		Util::lerpVec3_3Point(tri.v[2].normal, tri.v[1].normal, tri.v[0].normal, coord[0], coord[1], coord[2]));
	Util::setVec3(outVertex->position,
		Util::lerpVec3_3Point(tri.v[2].position, tri.v[1].position, tri.v[0].position, coord[0], coord[1], coord[2]));
	outVertex->shininess = Util::lerpFloat_3Point(tri.v[2].shininess, tri.v[1].shininess, tri.v[0].shininess, coord[0], coord[1], coord[2]);
}

/// <summary>
/// check if ray intersects with triangle or sphere, returns vertex at closest intersection point and id of hit object
/// </summary>
/// <param name="ray"></param>
/// <param name="outVertex"></param>
/// <returns></returns>
int rayCast(Ray ray, Vertex* outVertex, int ignoreID = 0) {
	// loop through every sphere
	Sphere* closestSphere = nullptr;
	HitInfo closestSphereHit;
	for (int i = 0; i < num_spheres; i++) {
		if (spheres[i].id == ignoreID) continue;

		HitInfo hitInfo;
		if (calculateSphereIntersection(spheres[i], ray, &hitInfo)) {
			//check if within raycast length
			if (hitInfo.distance > ray.maxDist) continue;

			//check if new closest sphere
			if (!closestSphere || hitInfo.distance < closestSphereHit.distance) {
				closestSphere = &spheres[i];
				Util::setVec3(closestSphereHit.position, hitInfo.position);
				closestSphereHit.distance = hitInfo.distance;
			}
		}
	}

	//step through octree for triangles
	Triangle* closestTri = nullptr;
	HitInfo closestTriHit;
	for (int i = 0; i < num_triangles; i++) {
		if (triangles[i].id == ignoreID) continue;

		HitInfo hitInfo;
		if (calculateTriangleIntersection(triangles[i], ray, &hitInfo)) {
			//check if within raycast length
			if (hitInfo.distance > ray.maxDist) continue;

			//check if new closest sphere
			if (!closestTri || hitInfo.distance < closestTriHit.distance) {
				closestTri = &triangles[i];
				Util::setVec3(closestTriHit.position, hitInfo.position);
				closestTriHit.distance = hitInfo.distance;
			}
		}
	}

	//calculate normal for closest intersection (sphere or triangle)
	if (closestSphere && closestTri) {
		if (closestSphereHit.distance < closestTriHit.distance) closestTri = nullptr;
		else closestSphere = nullptr;
	}

	if (closestSphere) {
		if (outVertex) calculateSphereVertex(*closestSphere, closestSphereHit.position, outVertex);
		return closestSphere->id;
	}

	else if (closestTri) {
		if (outVertex) calculateTriVertex(*closestTri, closestTriHit.position, outVertex);
		return closestTri->id;
	}

	return false;
}

/// <summary>
/// raycast from a point to each light source, return phong shading color with shadows
/// </summary>
/// <param name="vertex"></param>
/// <returns></returns>
int debugShadow = 0;
const double* castShadowRays(const Vertex& vertex, int ignoreID = 0) {
	double color[3];
	Util::setVec3(color, ambient_light);

	//specular light setup
	double toCamera[3] = {
		cameraPos[0]-vertex.position[0], cameraPos[1]-vertex.position[1], cameraPos[2]-vertex.position[2]
	};
	Util::setVec3(toCamera, Util::normalizeVec3(toCamera));

	Ray ray;
	Util::setVec3(ray.origin, vertex.position);
	for (int i = 0; i < num_lights; i++) {
		//set direction and max dist of raycast
		double dir[3] = {
			lights[i].position[0] - vertex.position[0],
			lights[i].position[1] - vertex.position[1],
			lights[i].position[2] - vertex.position[2]
		};
		double maxDist = Util::magVec3(dir);
		Util::setVec3(ray.direction, Util::normalizeVec3(dir));
		ray.maxDist = maxDist;

		//if the raycast hit something don't calculate lighting
		if (rayCast(ray, nullptr, ignoreID)) {
			continue;
		}

		double diffuse[3];
		Util::setVec3(diffuse, Vector0);

		double specular[3];
		Util::setVec3(specular, Vector0);

		//diffuse light
		double diffuseStrength = Util::dotVec3(ray.direction, vertex.normal);
		diffuseStrength = Util::max(diffuseStrength, 0.);
		Util::setVec3(diffuse, Util::mulVec3(vertex.color_diffuse, diffuseStrength));

		//specular light
		double reflector[3];
		Util::setVec3(reflector, Util::mulVec3(vertex.normal, 2. * diffuseStrength));
		double reflectedLightVector[3];
		Util::setVec3(reflectedLightVector, Util::subVec3(reflector, ray.direction));
		double specularStrength = pow(Util::dotVec3(toCamera, reflectedLightVector), vertex.shininess);
		specularStrength = Util::max(specularStrength, 0.);
		Util::setVec3(specular, Util::mulVec3(vertex.color_specular, specularStrength));

		if (debugShadow < 10) {
			Util::printVec3(diffuse, "shadow ");
			debugShadow++;
		}

		//TODO add attenuation factor

		//final color
		double colorContribution[3];
		Util::setVec3(colorContribution, Util::addVec3(diffuse, specular));
		Util::setVec3(colorContribution, Util::mulVec3(colorContribution, lights[i].color));
		Util::setVec3(color, Util::addVec3(color, colorContribution));
	}

	//clamp color
	color[0] = Util::min(color[0], 1.);
	color[1] = Util::min(color[1], 1.);
	color[2] = Util::min(color[2], 1.);
	
	//return final color
	return color;
}

void draw_scene()
{
	int debugScene = 0;
  unsigned int x,y;
  Vertex* vert = new Vertex();

  //simple output
  for(x=0; x<WIDTH; x++)
  {

    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(y=0;y < HEIGHT;y++)
    {
		//reset vertex
		Util::setVec3(vert->color_diffuse, Vector0);
		Util::setVec3(vert->color_specular, Vector0);
		Util::setVec3(vert->normal, Vector0);
		Util::setVec3(vert->position, Vector0);
		vert->shininess = 0.;

		// cast ray
		Ray ray;
		ray.set(Vector0, cameraRayDirections[x][y]);
		int hit = rayCast(ray, vert);
		if (hit) {
			// cast shadow ray
			double color[3];
			Util::setVec3(color, castShadowRays(*vert, hit));

			// draw
			plot_pixel(x, y, Util::min(color[0] * 255, 255), Util::min(color[1] * 255, 255), Util::min(color[2] * 255, 255));

			if (debugScene < 10) {
				Util::setVec3(color, Util::mulVec3(color, 255.));
				//Util::printVec3(color);
				debugScene++;
			}
		}
    }
    glEnd();
    glFlush();
  }

  delete vert;
  printf("Done!\n"); fflush(stdout);
}

#pragma region plot pixels
void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	glColor3f(((double)r) / 256.f, ((double)g) / 256.f, ((double)b) / 256.f);
	glVertex2i(x, y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	buffer[HEIGHT - y - 1][x][0] = r;
	buffer[HEIGHT - y - 1][x][1] = g;
	buffer[HEIGHT - y - 1][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	plot_pixel_display(x, y, r, g, b);
	if (mode == MODE_JPEG)
		plot_pixel_jpeg(x, y, r, g, b);
}
#pragma endregion

/* Write a jpg image from buffer*/
void save_jpg()
{
	if (filename == NULL)
		return;

	// Allocate a picture buffer // 
	cv::Mat3b bufferBGR = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3); //rows, cols, 3-channel 8-bit.
	printf("File to save to: %s\n", filename);

	// unsigned char buffer[HEIGHT][WIDTH][3];
	for (int r = 0; r < HEIGHT; r++) {
		for (int c = 0; c < WIDTH; c++) {
			for (int chan = 0; chan < 3; chan++) {
				unsigned char red = buffer[r][c][0];
				unsigned char green = buffer[r][c][1];
				unsigned char blue = buffer[r][c][2];
				bufferBGR.at<cv::Vec3b>(r,c) = cv::Vec3b(blue, green, red);
			}
		}
	}
	if (cv::imwrite(filename, bufferBGR)) {
		printf("File saved Successfully\n");
	}
	else {
		printf("Error in Saving\n");
	}
}

int loadScene(char *argv)
{
  FILE *file = fopen("examples/test2.txt", "r");
  int number_of_objects;
  char type[50];
  int i;
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i",&number_of_objects);

 // printf("number of objects: %i\n",number_of_objects);
  char str[200];

  Util::parse_doubles(file,"amb:",ambient_light);

  for (i = 0; i < number_of_objects; i++)
  {
	  fscanf(file, "%s\n", type);
	  //printf("%s\n", type);
	  if (stricmp(type, "triangle") == 0)
	  {

		  //printf("found triangle\n");
		  int j;

		  for (j = 0; j < 3; j++)
		  {
			  Util::parse_doubles(file, "pos:", t.v[j].position);
			  Util::parse_doubles(file, "nor:", t.v[j].normal);
			  Util::parse_doubles(file, "dif:", t.v[j].color_diffuse);
			  Util::parse_doubles(file, "spe:", t.v[j].color_specular);
			  Util::parse_shi(file, &t.v[j].shininess);
		  }

		  t.id = i + 1;

		  if (num_triangles == MAX_TRIANGLES)
		  {
			  printf("too many triangles, you should increase MAX_TRIANGLES!\n");
			  exit(0);
		  }
		  triangles[num_triangles++] = t;
	  }
	  else if (stricmp(type, "sphere") == 0)
	  {
		 // printf("found sphere\n");

		  Util::parse_doubles(file, "pos:", s.position);
		  Util::parse_rad(file, &s.radius);
		  Util::parse_doubles(file, "dif:", s.color_diffuse);
		  Util::parse_doubles(file, "spe:", s.color_specular);
		  Util::parse_shi(file, &s.shininess);

		  s.id = i + 1;

		  if (num_spheres == MAX_SPHERES)
		  {
			  printf("too many spheres, you should increase MAX_SPHERES!\n");
			  exit(0);
		  }
		  spheres[num_spheres++] = s;
	  }
	  else if (stricmp(type, "light") == 0)
	  {
		 // printf("found light\n");
		  Util::parse_doubles(file, "pos:", l.position);
		  Util::parse_doubles(file, "col:", l.color);

		  if (num_lights == MAX_LIGHTS)
		  {
			  printf("too many lights, you should increase MAX_LIGHTS!\n");
			  exit(0);
		  }
		  lights[num_lights++] = l;
	  }
	  else
	  {
		  printf("unknown type in scene description:\n%s\n", type);
		  exit(0);
	  }
  }
  return 0;
}

void display()
{

}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
      draw_scene();
      if(mode == MODE_JPEG)
	save_jpg();
    }
  once=1;
}

int main (int argc, char ** argv)
{
  if (argc<2 || argc > 3)
  {  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
    {
      mode = MODE_JPEG;
      filename = argv[2];
    }
  else if(argc == 2)
    mode = MODE_DISPLAY;
  
  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}
