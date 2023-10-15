#include <windows.h>
#include <stdlib.h>

#include <gl/glew.h>
#include <gl/glu.h>
#include <gl/glut.h>

#include <stdio.h>
#include <string>
#include <math.h>
#include <ctime>

#include <map>
#include "Util.h"
#include "Tri.h"
#include "Vector.h"
#include "Vertex.h"
#include "Sphere.h"
#include "RayCast.h"
#include "tests.h"
#include "Constants.h"
#include "Camera.h"
#include "Light.h"
#include "BVH.h"
#include "RenderQuad.h"

char* filename = 0;

int mode = MODE_DISPLAY;

unsigned char buffer[HEIGHT][WIDTH][3];

Tri triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
Vec3 ambient_light;

int numTriangles = 0;
int numSpheres= 0;
int numLights = 0;

Camera camera;
RenderQuad* renderQuad;
BVH* BVHeirarchy;

struct Pixel {
	Pixel() {}
	Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
		this->r = r; this->g = g; this->b = b; this->a = a;
	}
	uint8_t r{ 0 }, g{ 0 }, b{ 0 }, a{ 0 };
};
GLubyte pixelData[HEIGHT][WIDTH][3];

bool Init()
{
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(500, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	int window = glutCreateWindow("Ray Tracer");

	if (glewInit() != GLEW_OK) {
		return false;
	}

	glMatrixMode(GL_PROJECTION);
	glOrtho(0, WIDTH, 0, HEIGHT, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// precalculate triangle constants
	Vec3 cameraDir(0., 0., -1.);
	for (int i = 0; i < numTriangles; i++) {
		// perpendicular distance from origin to plane
		triangles[i].d = -Vec3::Dot(triangles[i].centroid - camera.position, triangles[i].normal);
	}

	BVHeirarchy = new BVH(triangles, numTriangles);
	renderQuad = new RenderQuad();
	return true;
}

Tri* BruteForceTris(Ray ray, HitInfo& closestTriHit, int ignoreID = 0) {
	Tri* closestTri = nullptr;
	for (int i = 0; i < numTriangles; i++) {
		if (triangles[i].id == ignoreID) continue;
		HitInfo hitInfo;
		//if (octree->CalculateIntersection(ray, &hitInfo, -1)){
		if (triangles[i].CalculateIntersection(ray, hitInfo)) {
			//check if within raycast length
			if (hitInfo.distance > ray.maxDist) continue;
			//check if new closest sphere
			if (!closestTri || hitInfo.distance < closestTriHit.distance) {
				closestTri = &triangles[i];
				closestTriHit.position = hitInfo.position;
				closestTriHit.distance = hitInfo.distance;
			}
		}
	}
	return closestTri;
}

/// <summary>
/// check if ray intersects with triangle or sphere, returns vertex at closest intersection point and id of hit object
/// </summary>
int debugRayCast = 0;
int RayCast(Ray ray, Vertex& outVertex, int ignoreID = 0) {
	// loop through every sphere
	//Sphere* closestSphere = nullptr;
	//HitInfo closestSphereHit;
	//for (int i = 0; i < numSpheres; i++) {
	//	if (spheres[i].id == ignoreID) continue;
	//	HitInfo hitInfo;
	//	if (spheres[i].CalculateIntersection(ray, &hitInfo)) {
	//		//check if within raycast length
	//		if (hitInfo.distance > ray.maxDist) continue;
	//		//check if new closest sphere
	//		if (!closestSphere || hitInfo.distance < closestSphereHit.distance) {
	//			closestSphere = &spheres[i];
	//			closestSphereHit.position = hitInfo.position;
	//			closestSphereHit.distance = hitInfo.distance;
	//		}
	//	}
	//}

	//step through bvh for triangles
	Tri* closestTri = nullptr;
	HitInfo closestTriHit;
	BVHeirarchy->CalculateIntersection(ray, closestTriHit);
	if (closestTriHit.triId != -1) closestTri = &triangles[closestTriHit.triId];

	//calculate normal for closest intersection (sphere or triangle)
	//if (closestSphere && closestTri) {
	//	if (closestSphereHit.distance < closestTriHit.distance) closestTri = nullptr;
	//	else closestSphere = nullptr;
	//}
	//if (closestSphere) {
	//	closestSphere->CalculateVertex(closestSphereHit.position, outVertex);
	//	return closestSphere->id;
	//}
	if (closestTri) {
		outVertex.color_diffuse = Vec3::One();
		//closestTri->CalculateVertex(closestTriHit.position, outVertex);
		return closestTri->id;
	}

	return false;
}

/// <summary>
/// raycast from a point to each light source, return phong shading color with shadows
/// </summary>
Vec3 CastShadowRays(const Vertex& vertex, int ignoreID = 0) {
	Vec3 color = ambient_light;

	//specular light setup
	Vec3 toCamera = camera.position - vertex.position;
	toCamera.Normalize();

	Ray ray;
	ray.origin = vertex.position;
	for (int i = 0; i < numLights; i++) {
		//set direction and max dist of raycast
		//Vec3 dir = lights[i].position - vertex.position;
		//ray.maxDist = dir.Magnitude();
		//ray.direction = dir * (1. / ray.maxDist);
		//ray.dInv = Vec3(1. / ray.direction[0], 1. / ray.direction[1], 1. / ray.direction[2]);

		//if the raycast hit something don't calculate lighting
		//if (RayCast(ray, nullptr, ignoreID)) {
		//	continue;
		//}

		Vec3 diffuse;
		Vec3 specular;

		//diffuse light
		double diffuseStrength = Vec3::Dot(ray.direction, vertex.normal);
		diffuseStrength = fmax(diffuseStrength, 0.);
		diffuse = vertex.color_diffuse * diffuseStrength;

		//specular light
		Vec3 reflector = vertex.normal * (2. * diffuseStrength);
		Vec3 reflectedLightVector = reflector - ray.direction;
		double specularStrength = pow(Vec3::Dot(toCamera, reflectedLightVector), vertex.shininess);
		specularStrength = fmax(specularStrength, 0.);
		specular = vertex.color_specular * specularStrength;

		//TODO add attenuation factor

		//final color
		Vec3 colorContribution = (diffuse + specular) * lights[i].color;
		color = color + colorContribution;
		
	}

	//clamp color
	color.Set(fmin(color[0], 1.), fmin(color[1], 1.), fmin(color[2], 1.));

	//return final color
	return color;
}


void DrawScene()
{
	clock_t totalTime = 0;
	clock_t startDrawTime = clock();

	clock_t raycastTime = 0;

	Vertex vert;
	const int tileSize = 16;
	int x, y, u, v, i;

#pragma omp parallel for schedule(dynamic) reduction(+:raycastTime), private(y, u, v, i, vert)
	for (x = 0; x < WIDTH; x+=tileSize) {
//#pragma omp parallel for schedule(dynamic), private(u, v, i, vert)
		for (y = 0; y < HEIGHT; y += tileSize) {
			for (u = x; u < x + tileSize; u++) {
//#pragma omp parallel for schedule(dynamic, 10), private(i, vert)
				for (v = y; v < y + tileSize; v++) {
					//if i used x and y here it gave a kinda cool pixellated effect lol
					clock_t startRayTime = clock();
					if (RayCast(camera.GetRay(u, v), vert)) {
						// cast shadow ray
						//Vec3 color = CastShadowRays(vert, hit);
						Vec3 color = vert.color_diffuse * 255.0f;
						for (i = 0; i < 3; i++) {
							pixelData[v][u][i] = (GLubyte)(color[i]);
						}
					}
					raycastTime += (clock() - startRayTime);
				}
			}
		}
	}

	totalTime = (clock() - startDrawTime);
	renderQuad->Draw(pixelData);

	glFlush();

	Util::Print("Avg ms per raycast = " + std::to_string(raycastTime / (double)(WIDTH * HEIGHT)));
	Util::Print("Total triangle intersections = " + std::to_string(triangles[0].debug()));
	Util::Print("False intersections = " + std::to_string(BVHeirarchy->falseBranch));
	Util::Print("Total draw secs = " + std::to_string((clock() - startDrawTime)/1000.0f));
	Util::Print("BVH construction secs = " + std::to_string(BVHeirarchy->constructionTime/1000.0f));
}

bool LoadScene(char* argv)
{
	FILE* file = fopen("examples/SIGGRAPH2.txt", "r");
	int number_of_objects;
	char type[50];
	int i;
	Tri t;
	Sphere s;
	Light l;
	fscanf(file, "%i", &number_of_objects);

	// printf("number of objects: %i\n",number_of_objects);

	Util::parse_doubles(file, "amb:", ambient_light);

	for (i = 0; i < number_of_objects; i++)
	{
		fscanf(file, "%s\n", type);
		//printf("%s\n", type);
		if (stricmp(type, "triangle") == 0)
		{
			//printf("found triangle\n");
			t.ParseFromFile(file, i + 1);

			if (numTriangles == MAX_TRIANGLES)
			{
				printf("too many triangles, you should increase MAX_TRIANGLES!\n");
				return false;
			}
			triangles[numTriangles++] = t;
		}
		else if (stricmp(type, "sphere") == 0)
		{
			//printf("found sphere\n");

			s.ParseFromFile(file, i + 1);

			if (numSpheres == MAX_SPHERES)
			{
				printf("too many spheres, you should increase MAX_SPHERES!\n");
				return false;
			}
			spheres[numSpheres++] = s;
		}
		else if (stricmp(type, "light") == 0)
		{
			// printf("found light\n");
			Util::parse_doubles(file, "pos:", l.position);
			Util::parse_doubles(file, "col:", l.color);

			if (numLights == MAX_LIGHTS)
			{
				printf("too many lights, you should increase MAX_LIGHTS!\n");
				return false;
			}
			lights[numLights++] = l;
		}
		else
		{
			printf("unknown type in scene description:\n%s\n", type);
			return false;
		}
	}
	return true;
}

void Display()
{
	// Clear the screen
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);


	//	// Draw the fullscreen quad
	//glUseProgram(shaderProgram);
	//glBindVertexArray(VAO);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//glBindVertexArray(0);

	//glutSwapBuffers();
}

void Idle()
{
	//hack to make it only draw once
	static int once = 0;
	if (!once)
	{
		DrawScene();
	}
	once = 1;
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
  if (!LoadScene(argv[1])) exit(0);
  if (!Init()) exit(0);;

  glutDisplayFunc(Display);
  glutIdleFunc(Idle);
  glutMainLoop();
}
