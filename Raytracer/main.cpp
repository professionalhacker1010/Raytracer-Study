#include <windows.h>
#include <stdlib.h>

#include <gl/glew.h>
#include <gl/glu.h>
#include <gl/glut.h>

#include <stdio.h>
#include <string>
#include <math.h>
#include <ctime>

#include "Util.h"
#include "Tri.h"
#include "Math.h"
#include "Vertex.h"
#include "Sphere.h"
#include "RayCast.h"
#include "tests.h"
#include "Constants.h"
#include "Camera.h"
#include "Light.h"
#include "BVH.h"
#include "RenderQuad.h"
#include "Mesh.h"
#include "MeshInstance.h"

char* filename = 0;

int mode = MODE_DISPLAY;

Tri tris[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
Vec3 ambient_light;

int numTriangles = 0;
int numSpheres= 0;
int numLights = 0;

//scene
Camera* camera;
TLAS* tlas;

constexpr int NUM_MESHES = 1;
BVH* bvh[NUM_MESHES];
Mesh* meshes[NUM_MESHES];

constexpr int NUM_MESH_INST = 4;
MeshInstance* meshInstances[NUM_MESH_INST];

RenderQuad* renderQuad;
GLubyte pixelData[HEIGHT][WIDTH][3];

clock_t startFrameTime = 0;

//debug
int frames = 0;
long int triIntersections = 0;
clock_t totalTime = 0;
clock_t raycastTime = 0;
clock_t drawTime = 0;
clock_t buildTime = 0;

bool Init()
{
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
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

	for (int i = 0; i < NUM_MESHES; i++) {
		meshes[i] = new Mesh(tris, numTriangles, i);
		bvh[i] = new BVH(meshes[i]->tris, numTriangles);
		bvh[i]->Rebuild();
	}

	for (int i = 0; i < NUM_MESH_INST; i++) {
		meshInstances[i] = new MeshInstance(meshes[0]);
	}

	tlas = new TLAS(bvh, meshInstances, NUM_MESH_INST);

	for (int i = 0; i < NUM_MESH_INST; i++) {
		if (i == 0) meshInstances[i]->SetTransform(Mat4::CreateTranslation(Vec3(0.0f, 1.0f, 0.0f)));
		else if (i == 1) meshInstances[i]->SetTransform(Mat4::CreateTranslation(Vec3(0.0f, -1.0f, 0.0f)));
		else if (i == 2) meshInstances[i]->SetTransform(Mat4::CreateTranslation(Vec3(0.0f, -2.0f, 0.0f)));
		else if (i == 3) meshInstances[i]->SetTransform(Mat4::CreateTranslation(Vec3(0.0f, 2.0f, 0.0f)));
	}
	tlas->Rebuild();

	camera = &Camera::Get();
	renderQuad = new RenderQuad();
	return true;
}

/// <summary>
/// check if ray intersects with triangle or sphere, returns vertex at closest intersection point and id of hit object
/// </summary>
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
	tlas->CalculateIntersection(ray, closestTriHit);
	closestTri = closestTriHit.hit;

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
		outVertex.color_diffuse = closestTri->verts[0].color_diffuse;
		//outVertex.color_diffuse = Vec3::One();
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
	Vec3 toCamera = Camera::Get().position - vertex.position;
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
	clock_t c = clock();

	float deltaTime = (float)(c - startFrameTime) / 1000.0f;
	startFrameTime = c;

	for (int i = 0; i < NUM_MESHES; i++) {
		meshes[i]->Animate(deltaTime);
	}

	// animate the scene
	static float a[16] = { 0 }, h[16] = { 5, 4, 3, 2, 1, 5, 4, 3 }, s[16] = { 0 };
	for (int i = 0, x = 0; x < 2; x++) for (int y = 0; y < 2; y++, i++)
	{
		Mat4 R, T = Mat4::CreateTranslation(Vec3((x - 1.5f) * 2.5f, 0, (y - 1.5f) * 2.5f));
		if ((x + y) & 1) R = Mat4::CreateRotationX(a[i]) * Mat4::CreateRotationZ(a[i]);
		else R = Mat4::CreateTranslation(Vec3(0, h[i / 2], 0));
		if ((a[i] += (((i * 13) & 7) + 2) * 0.005f) > 2 * PI) a[i] -= 2 * PI;
		if ((s[i] -= 0.01f, h[i] += s[i]) < 0) s[i] = 0.2f;
		meshInstances[i]->SetTransform( Mat4::CreateScale(0.75f) * R * T * Mat4::CreateTranslation(Vec3(5.0f, 0.0f, 0.0f)));
	}

	clock_t startBuildTime = clock();
	for (int i = 0; i < NUM_MESHES; i++) {
		bvh[i]->Refit();
	}
	tlas->Rebuild();
	c = clock();
	buildTime += (c - startBuildTime);

	clock_t startDrawTime = c;

	Vertex vert;
	const int tileSize = 4;
	int x, y, u, v, i;

#pragma omp parallel for schedule(dynamic) reduction(+:raycastTime), private(y, u, v, i, vert)
	for (x = 0; x < WIDTH; x+=tileSize) {
		for (y = 0; y < HEIGHT; y += tileSize) {
			for (u = x; u < x + tileSize; u++) for (v = y; v < y + tileSize; v++) {
				//if i used x and y here it gave a kinda cool pixellated effect lol
				clock_t startRayTime = clock();
				int hit = RayCast(Camera::Get().GetRay(u, v), vert);
				raycastTime += (clock() - startRayTime);
				if (hit) {
					// cast shadow ray
					//Vec3 color = CastShadowRays(vert, hit);
					Vec3 color = vert.color_diffuse * 255.0f;
					for (i = 0; i < 3; i++) {
						pixelData[v][u][i] = (GLubyte)(color[i]);
					}
				}
				else {
					for (i = 0; i < 3; i++) {
						pixelData[v][u][i] = (GLubyte)0;
					}
				}
				
			}
		}
	}

	drawTime += (clock() - startDrawTime);

	glutPostRedisplay();

	frames++;

	int intersections = tris[0].debug();
	triIntersections += intersections;
	//Util::Print("Avg ms per raycast = " + std::to_string(raycastTime / (double)(WIDTH * HEIGHT)));
	//Util::Print("Total triangle intersections = " + std::to_string(intersections));
	//Util::Print("False intersections = " + std::to_string(meshes[0]->bvh->falseBranch));
	bvh[0]->falseBranch = 0;
	//Util::Print("Total draw secs = " + std::to_string(drawTime/1000.0f));
	//Util::Print("BVH construction secs = " + std::to_string(buildTime/1000.0f));
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

			//if (numTriangles == MAX_TRIANGLES)
			//{
			//	printf("too many triangles, you should increase MAX_TRIANGLES!\n");
			//	return false;
			//}
			tris[numTriangles++] = t;
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
	renderQuad->Draw(pixelData);
	glutSwapBuffers();
}

void Idle()
{
	clock_t startAnimTime = clock();
	//hack to make it only draw once
	static int once = 0;
	//if (!once)
	{
		DrawScene();
	}
	once = 1;
	totalTime += clock() - startAnimTime;
}

void OnKeyDown(unsigned char key, int x, int y) {
	if (key == 'Q' || key == 'q') exit(0);
}

void OnExit() {
	Util::Print("Avg FPS = " + std::to_string((float)frames / ((float)totalTime / 1000.0f)));
	Util::Print("Avg BVH construction secs = " + std::to_string(buildTime / ((float)frames * 1000.0f)));
	Util::Print("Avg tri intersections = " + std::to_string(triIntersections / ((float)frames)));
	Util::Print("Avg ms per raycast = " + std::to_string(raycastTime / ((float)(WIDTH * HEIGHT))));
	Util::Print("Avg draw secs = " + std::to_string(drawTime / ((float)frames * 1000.0f)));
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
  atexit(OnExit);
  glutKeyboardFunc(OnKeyDown);
  startFrameTime = clock();
  glutMainLoop();
}
