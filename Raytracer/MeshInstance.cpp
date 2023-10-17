#include "MeshInstance.h"
#include "BVH.h"
#include "Mesh.h"
void MeshInstance::SetTransform(Mat4 transform)
{
	invTransform = transform;
	invTransform.Invert();
	OnTransformSet(transform);
}
