#include "MeshInstance.h"
#include "BVH.h"
#include "Mesh.h"
void MeshInstance::SetTransform(Mat4 transform)
{
	transform = transform;
	OnTransformSet(transform);
}
