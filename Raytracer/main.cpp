#include <windows.h>
#include <stdlib.h>

#include <gl/glew.h>
#include <gl/glu.h>
#include <gl/glut.h>

#include <stdio.h>
#include <string>
#include <math.h>
#include <ctime>
#include "stb_image.h"

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
#include "Surface.h"
int mode = MODE_DISPLAY;

//scene
Camera* camera;
TLAS* tlas;

constexpr int NUM_MESHES = 1;
BVH* bvh[NUM_MESHES];
Mesh* meshes[NUM_MESHES];

constexpr int NUM_MESH_INST = 4;
BVHInstance* bvhInstances;
MeshInstance* meshInstances;

constexpr int NUM_LIGHTS = 1;
Light lights[MAX_LIGHTS];
Vec3 ambientLight;

//rendering
RenderQuad* renderQuad;
GLubyte pixelData[HEIGHT][WIDTH][3];

//debug
int frames = 0;
long int triIntersections = 0;
clock_t startFrameTime = 0;
clock_t totalTime = 0;
clock_t raycastTime = 0;
clock_t drawTime = 0;
clock_t buildTime = 0;
Vec3 colors[] = {
	Vec3(1, 0, 0), //red
	Vec3(0, 1, 0), //green
	Vec3(0, 0, 1), //blue
	Vec3(1, 1, 0), //yellow
	Vec3(0, 1, 1), //teal
	Vec3(1, 0, 1), //pink
};

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
		//int dummy = 0;
		meshes[i] = new Mesh("Assets/teapot.obj", "Assets/bricks.png", i);
		bvh[i] = new BVH(meshes[i]->tris, meshes[i]->numTris);
		bvh[i]->Rebuild();
	}

	meshInstances = (MeshInstance*)_aligned_malloc(sizeof(MeshInstance) * NUM_MESH_INST, ALIGN);
	bvhInstances = (BVHInstance*)_aligned_malloc(sizeof(BVHInstance) * NUM_MESH_INST, ALIGN); //new BVHInstance[numMeshInstances];//

	for (int i = 0; i < NUM_MESH_INST; i++) {
		meshInstances[i].Set(meshes[0], i); //= (MeshInstance*)_aligned_malloc(sizeof(MeshInstance), ALIGN); //new MeshInstance(meshes[0], i);
		bvhInstances[i].Set(bvh[meshInstances[i].meshRef->id], &meshInstances[i]);
	}

	tlas = new TLAS(bvhInstances, NUM_MESH_INST);
	int size = sizeof(MeshInstance);
	int size2 = sizeof(Mesh);
	int size3 = sizeof(BVH);
	int size4 = sizeof(TLAS);
	int size5 = sizeof(RenderQuad);
	int size6 = sizeof(BVHInstance);
	int size7 = sizeof(BVHNode);
	Util::Print("Sizeof BVH node = " + std::to_string(size7));

	
	// load HDR sky
	int skyWidth, skyHeight, skyBpp;
	int bpp = 0;
	float* skyPixels = stbi_loadf("assets/sky_19.hdr", &skyWidth, &skyHeight, &skyBpp, 0);
	for (int i = 0; i < skyWidth * skyHeight * 3; i++) skyPixels[i] = sqrtf(skyPixels[i]);

	tlas->Rebuild();

	lights[0].position = Vec3(3, 10, 2);
	lights[0].color = Vec3(255, 255, 191) / 255.0f;
	ambientLight = Vec3(40, 36, 59) / 255.0f;

	renderQuad = new RenderQuad();
	camera = &Camera::Get();
	camera->SetPosition(Vec3(-2.0f, 0.0f, 2.0f));
	return true;
}

inline Vec3 RGB8toRGB32F(unsigned int c)
{
	float s = 1 / 256.0f;
	int r = (c >> 16) & 255;
	int g = (c >> 8) & 255;
	int b = c & 255;
	return Vec3(r * s, g * s, b * s);
}

/// <summary>
/// check if ray intersects with triangle or sphere, returns vertex at closest intersection point and id of hit object
/// </summary>
std::atomic_int zeroes;
bool RayCast(Ray ray, Vertex& outVertex, HitInfo& hit, int ignoreID = 0) {
	//step through bvh for triangles
	tlas->CalculateIntersection(ray, hit);
	
	if (hit.triId != -1) {
		TriVerts& vertData = meshInstances[hit.meshInstId].meshRef->vertData[hit.triId];
		Surface& tex = *(meshInstances[hit.meshInstId].meshRef->texture);

		Vec3 coord = Vec3(hit.u, hit.v, 1 - (hit.u + hit.v));

		//normal
		Vec3 norm = Vec3::BaryCoord(vertData.norm[1], vertData.norm[2], vertData.norm[0], coord);
		norm = Mat4::Transform(norm, meshInstances[hit.meshInstId].GetTransform(), 0.0f).Normalized();
		outVertex.normal = norm;

		//texture
		Vec2 uv = Vec2::BaryCoord(vertData.uv[1], vertData.uv[2], vertData.uv[0], coord);
		int iu = (int)(uv[0] * (float)tex.width) % tex.width;
		int iv = (int)(uv[1] * (float)tex.height) % tex.height;
		unsigned int texel = tex.pixels[iu + iv * tex.width];
		outVertex.albedo = RGB8toRGB32F(texel);

		//position
		outVertex.position = ray.origin + hit.distance * ray.direction;

		return true;
	}

	return false;
}

/// <summary>
/// raycast from a point to each light source, return phong shading color with shadows
/// </summary>
Vec3 CastShadowRays(const Vertex& vertex, int ignoreID = 0) {
	Vec3 color = ambientLight;

	//specular light setup
	//Vec3 toCamera = Camera::Get().position - vertex.position;
	//toCamera.Normalize();
	Ray ray;
	ray.origin = vertex.position;
	for (int i = 0; i < NUM_LIGHTS; i++) {
		//set direction and max dist of raycast
		Vec3 dir = (lights[i].position + camera->GetInvPosition()) - vertex.position;
		float dist = dir.Length();
		dir *= 1.0f / dist;
		ray.Set(vertex.position, dir, dist);

		//if the raycast hit something don't calculate lighting
		//if (RayCast(ray, nullptr, ignoreID)) {
		//	continue;
		//}

		float diffuse = 0;
		float specular = 0;

		//diffuse light
		diffuse = Vec3::Dot(vertex.normal, ray.direction);
		diffuse = fmaxf(diffuse, 0.0f);

		//specular light
		//Vec3 reflector = vertex.normal * (2. * diffuseStrength);
		//Vec3 reflectedLightVector = reflector - ray.direction;
		//double specularStrength = pow(Vec3::Dot(toCamera, reflectedLightVector), vertex.shininess);
		//specularStrength = fmax(specularStrength, 0.);
		//specular = vertex.color_specular * specularStrength;

		//TODO add attenuation factor

		//final color
		color += (diffuse + specular) * lights[i].color; //* (1.0f / (dist * dist));
	}
	color = color * vertex.albedo;

	//clamp color
	color = Vec3::Min(color, Vec3::One());

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
		Mat4 transform = Mat4::CreateScale(0.75f) * R * T * Mat4::CreateTranslation(camera->GetInvPosition());
		meshInstances[i].SetTransform(transform);
		bvhInstances[i].SetTransform(transform);
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
				HitInfo hitInfo;
				int hit = RayCast(Camera::Get().GetRay(u, v), vert, hitInfo);
				raycastTime += (clock() - startRayTime);
				if (hit) {
					// cast shadow ray
					Vec3 color = CastShadowRays(vert, hitInfo.triId) * 255.0f;
					//Vec3 color = vert.color_diffuse * 255.0f;
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
	triIntersections += meshes[0]->tris[0].debug();

	//system("pause");
	glutPostRedisplay();

	frames++;
}

void Display()
{
	renderQuad->Draw(pixelData);
	glutSwapBuffers();
}

void Idle()
{
	clock_t startAnimTime = clock();
	DrawScene();
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
	
	_aligned_free(bvhInstances);
	_aligned_free(meshInstances);

	for (int i = 0; i < NUM_MESHES; i++) {
		delete meshes[i];
		delete bvh[i];
	}

	delete tlas;
	delete renderQuad;
	system("pause");
}

int main (int argc, char ** argv)
{
  if (argc<2 || argc > 3)
  {  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  
  FILE* fp;

  AllocConsole();
  freopen_s(&fp, "CONIN$", "r", stdin);
  freopen_s(&fp, "CONOUT$", "w", stdout);
  freopen_s(&fp, "CONOUT$", "w", stderr);

  glutInit(&argc,argv);
  if (!Init()) exit(0);;

  glutDisplayFunc(Display);
  glutIdleFunc(Idle);
  atexit(OnExit);
  glutKeyboardFunc(OnKeyDown);
  startFrameTime = clock();
  glutMainLoop();
}
