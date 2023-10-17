#include "MeshInstance.h"
#include "BVH.h"
#include "Mesh.h"
void MeshInstance::SetTransform(Mat4 transform)
{
	this->transform = transform;
	if (OnTransformSet) OnTransformSet(transform);
}
