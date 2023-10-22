#pragma once
#include "Math.h"
#include "AABB.h"
#include <functional>
class Mesh;

class MeshInstance {
public:
	MeshInstance() = default;
	MeshInstance(Mesh* mesh, int instanceId) {
		meshRef = mesh;
		id = instanceId;
	}

	void SetTransform(Mat4 transform);
	Mat4 GetTransform() { return transform; }

private:
	Mat4 transform; //64 bytes
public:
	std::function<void(Mat4 transform)> OnTransformSet; //40 bytes
	Mesh* meshRef; //4
	int id; //4

	//temp
	//Vec3 color; 


};