#pragma once
#include "Math.h"
#include "AABB.h"
#include <functional>
class Mesh;

class MeshInstance {
public:
	MeshInstance() = default;
	MeshInstance(Mesh* mesh) {
		meshRef = mesh;
	}

	void SetTransform(Mat4 transform);
	std::function<void(Mat4 transform)> OnTransformSet;
	Mat4 invTransform;
	Mesh* meshRef;
};