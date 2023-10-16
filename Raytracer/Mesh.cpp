#include "Mesh.h"
#include "BVH.h"

Mesh::Mesh(Tri* triangles, int numTriangles) {
	bindPoseTris = triangles;
	numTris = numTriangles;
	bvh = new BVH(triangles, numTriangles);
}

Mesh::~Mesh()
{
	//delete[] tris;
}

void Mesh::Animate(float deltaTime)
{
	if ((rotation += (rotationSpeed * deltaTime)) > 2.0f * PI) rotation -= 2.0f * PI;
	float a = sinf(rotation) * 0.5f;
	for (int i = 0; i < numTris; i++) {
		for (int j = 0; j < 3; j++) {
			Vec3 original = bindPoseTris[i].verts[j].position;
			float step = a * (original[1] - 0.2f) * 0.2f;
			float x = original[0] * cosf(step) - original[1] * sinf(step);
			float y = original[0] * sinf(step) + original[1] * cosf(step);
			bvh->tris[i].verts[j].position = Vec3(x, y, original[2]);
			bvh->tris[i].CachedCalculations();
		}
	}
}
