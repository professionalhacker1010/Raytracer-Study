#pragma once
#include "Math.h"
#include "AABB.h"
class Mesh;

class MeshInstance {
public:
	MeshInstance() = default;
	MeshInstance(Mesh* mesh, int instanceId) {
		Set(mesh, instanceId);
	}
	void Set(Mesh* mesh, int instanceId) {
		meshRef = mesh;
		id = instanceId;
	}
	void SetTransform(Mat4 transform);
	Mat4 GetTransform() { return transform; }

private:
	Mat4 transform; //64 bytes
public:
	Mesh* meshRef; //4
	int id; //4
	float pad0 = 0, pad1 = 0;
};