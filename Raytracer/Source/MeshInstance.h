#pragma once
#include "Math.h"
#include "AABB.h"
class Mesh;

class MeshInstance {
public:
	MeshInstance() = default;
	MeshInstance(int meshId, int instanceId) {
		Set(meshId, instanceId);
	}
	void Set(int meshId, int instanceId) {
		this->meshId = meshId;
		id = instanceId;
	}
	void SetTransform(Mat4 transform);
	Mat4 GetTransform() { return transform; }

private:
	Mat4 transform; //64 bytes
public:
	int meshId; //4
	int id; //4
	float pad[14];
};