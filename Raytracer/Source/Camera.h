#pragma once
#include "Math.h"
#include "RayCast.h"
#include "Constants.h"

class Buffer;
class RenderQuad;

class Camera {
public:
	static Camera& Get() {
		static Camera instance;
		return instance;
	}

	~Camera();

	void Init(GLFWwindow* window);
	void Render();
	void RenderCPU();
	Ray GetRay(unsigned int x, unsigned int y);
	void SetTransform(Vec3 position, Mat4 rotation);
	Vec3 GetPosition() { return pos; }

	void UpdateRays();

private:
	Ray rays[WIDTH][HEIGHT];

public:
	unsigned int pixels[HEIGHT * WIDTH * 4];
	GLubyte pixelData[HEIGHT][WIDTH][4];
private:
	Mat4 rot;
	Vec3 pos;

public:
	Vec3 bottomLeft;
	Vec3 lengthY;
	Vec3 lengthX;

	Buffer* renderTargetBuffer;
	RenderQuad* renderQuad;

private:
	Camera();
public:
	Camera(Camera const&) = delete;
	void operator=(Camera const&) = delete;
};