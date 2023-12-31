#pragma once
#include "Light.h"

class Camera;
class BVHInstance;
class MeshInstance;
class BVH;
class Mesh;
class TLAS;
class RenderQuad;
class Kernel;
class Buffer;
struct Vec3;
struct Ray;
struct HitInfo;
struct Vertex;
struct GLFWwindow;

class Application {

private:
	Application() = default;
public:
	Application(Application const&) = delete;
	void operator=(Application const&) = delete;

	static Application& Get() {
		static Application instance;
		return instance;
	}

	bool Init(GLFWwindow* window);
	void Tick(float deltaTime);
	void Shutdown();

	// input handling
	void MouseUp(int button) {}
	void MouseDown(int button) {}
	void MouseMove(int x, int y) {}
	void MouseWheel(float y) {}
	void KeyUp(int key) {}
	void KeyDown(int key);

private:
	//scene updates
	void AnimateScene(float deltaTime);
	void DrawScene();

	/// <summary>
	/// check if ray intersects with triangle or sphere, returns vertex at closest intersection point and id of hit object
	/// </summary>
	bool RayCast(Ray& ray, Vertex& outVertex, HitInfo& hit, int ignoreID = 0);

	/// <summary>
	/// bounce rays for a pure mirror effect
	/// </summary>
	Vec3 CastMirrorRays(Ray& ray, Vertex& vertex);

	/// <summary>
	/// raycast from a point to each light source, return phong shading color with shadows
	/// </summary>
	Vec3 CastShadowRays(const Vertex& vertex);

	Vec3 DrawSkybox(Ray& ray);

	inline Vec3 RGB8toRGB32F(unsigned int c)
	{
		float s = 1 / 256.0f;
		int r = (c >> 16) & 255;
		int g = (c >> 8) & 255;
		int b = c & 255;
		return Vec3(r * s, g * s, b * s);
	}

	//scene
	Camera* camera;
	TLAS* tlas;
	
public:
	BVH* bvh[NUM_MESHES];
	Mesh* meshes[NUM_MESHES];
	
private:
	BVHInstance* bvhInstances;
	MeshInstance* meshInstances;

	Light lights[MAX_LIGHTS];
	Vec3 ambientLight;

	int skyWidth, skyHeight, skyBpp;
	float* skyPixels;

	//rendering
	unsigned int pixelData[HEIGHT][WIDTH][4];

	//mouse
	Vec2 mousePos;

	//opencl kernel
	Kernel* tracer;

	//gpu buffers
	Buffer* textureBuffer;
	Buffer* skyboxBuffer;
	Buffer* triBuffer;
	Buffer* vertDataBuffer;
	Buffer* bvhBuffer;
	Buffer* bvhTriIdxBuffer;
	Buffer* bvhInstBuffer;
	Buffer* meshInstBuffer;
	Buffer* tlasBuffer;
};