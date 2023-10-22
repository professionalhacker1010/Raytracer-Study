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

	std::function<void(Mat4 transform)> OnTransformSet;
	Mesh* meshRef;
	int id;

	//temp
	Vec3 color;
private:
	Mat4 transform;

};