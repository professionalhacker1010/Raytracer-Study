#include "Octree.h"
#include "RayCast.h"


Octree::Octree(Vec3 maxBounds, Vec3 minBounds, Tri triangles[MAX_TRIANGLES], int numTris)
{
	root = new Node();
	root->maxBounds = maxBounds;
	root->minBounds = minBounds;
	for (int i = 0; i < numTris; i++) {
		//root->tris[triangles[i].id] = &triangles[i];
	}

	Init(root);
	Clean(root);
}

void Octree::Clean(Node* root)
{
	if (root->children[0] == nullptr) return;

	int emptyCount = 0;
	for (int i = 0; i < 8; i++) {
		if (root->children[i]->tris.empty()) emptyCount++;
	}

	//if (emptyCount == 8) {
	//	for (int i = 0; i < 8; i++) {
	//		delete root->children[i];
	//		root->children[i] = nullptr;
	//	}
	//}
	//else {

	//}

	//precalculate d
	for (int i = 0; i < 6; i++) {
		if (i < 3) root->d[i] = -Vec3::Dot(root->minBounds /* - cameraPos*/, cubeNormals[i]);
		else root->d[i] = -Vec3::Dot(root->maxBounds /* - cameraPos*/, cubeNormals[i]);
	}

	for (int i = 0; i < 8; i++) {
		Clean(root->children[i]);
	}
}

Node* Octree::FindNode(Node* root, const Vec3& pos)
{
	if (root->children[0] == nullptr) return root;
	//if (root->tris.empty()) return root;

	if (Util::withinBounds(pos, root->maxBounds, root->minBounds)) {
		if (pos[0] > root->center[0]) {
			if (pos[1] > root->center[1]) {
				if (pos[2] > root->center[2]) return FindNode(root->children[(int)BACK_RIGHT_TOP], pos);
				else return FindNode(root->children[(int)FRONT_RIGHT_TOP], pos);
			}
			else {
				if (pos[2] > root->center[2]) return FindNode(root->children[(int)BACK_RIGHT_BOT], pos);
				else return FindNode(root->children[(int)FRONT_RIGHT_BOT], pos);
			}
		}
		else {
			if (pos[1] > root->center[1]) {
				if (pos[2] > root->center[2]) return FindNode(root->children[(int)BACK_LEFT_TOP], pos);
				else return FindNode(root->children[(int)FRONT_LEFT_TOP], pos);
			}
			else {
				if (pos[2] > root->center[2]) return FindNode(root->children[(int)BACK_LEFT_BOT], pos);
				else return FindNode(root->children[(int)FRONT_LEFT_BOT], pos);
			}
		}
	}
	else {
		//printf("not in bounds\n");
		//printVec3(pos, "pos");
		//printVec3(root->minBounds, "min");
		//printVec3(root->maxBounds, "max");
		//printf("\n");
	}

	return nullptr;
}

bool Octree::CalculateIntersection(const Ray& ray, HitInfo* out, int ignoreID)
{
	Vec3 epsilon = Vec3(ray.direction[0] * (float)EPSILON, ray.direction[1] * (float)EPSILON, ray.direction[2] * (float)EPSILON);
	Ray testRay(ray.origin, ray.direction);
	Node* closestNode = root;
	Tri* closestTri = nullptr;

	//find start and end intersection with root cube
	HitInfo frontHit, backHit;
	if (!Util::withinBounds(ray.origin, root->maxBounds, root->minBounds)) {
		CalculateVoxelIntersection(root->minBounds, root->maxBounds, root->d, ray, &frontHit);
	}
	else {
		//frontHit.position.Set(ray.origin);
		frontHit.distance = 0;
	}
	//testRay.origin = epsilon + frontHit.position;
	CalculateVoxelIntersection(root->minBounds, root->maxBounds, root->d, testRay, &backHit);

	//step through octree
	while (closestTri == nullptr && !Util::doubleCompare(frontHit.distance, backHit.distance) && frontHit.distance < ray.maxDist) {

		//find closest intersection in tree
		//frontHit.position = epsilon + frontHit.position;

		//closestNode = FindNode(root, frontHit.position);
		if (!closestNode) return false;
		//printVec3(frontHit.position, "test pos");
		//printVec3(closestNode->minBounds, "min");
		//printVec3(closestNode->maxBounds, "max");


		//find closest triangle intersection
		std::map<int, Tri*>::iterator it;
		for (it = closestNode->tris.begin(); it != closestNode->tris.end(); ++it) {

			if (it->first == ignoreID) continue;

			HitInfo hitInfo; //barycentric coords + distance from ray origin
			if (it->second->CalculateIntersection(ray, hitInfo)) {

				//check if within raycast length
				if (hitInfo.distance > ray.maxDist) continue;

				//check if new closest triangle
				if (!closestTri || hitInfo.distance < out->distance) {

					closestTri = it->second;
					//out->position = hitInfo.position;
					out->distance = hitInfo.distance;
				}
			}

		}

		//if no found continue to next octree node
		double tempDist = frontHit.distance;
		//testRay.origin = frontHit.position;
		CalculateVoxelIntersection(closestNode->minBounds, closestNode->maxBounds, closestNode->d, testRay, &frontHit);
		//printVec3(frontHit.position);
		//printDouble(frontHit.distance);
		//printf("\n");
		frontHit.distance += tempDist;
	}

	return closestTri;
}

void Octree::CalculateVoxelIntersection(const Vec3& min, const Vec3& max, const double d[6], const Ray& ray, HitInfo* out)
{
	out->distance = 10000;

	for (int i = 0; i < 6; i++) {
		// get distance from ray origin to plane
		double t;
		double d;
		if (i < 3) d = -Vec3::Dot(min /* - cameraPos*/, cubeNormals[i]);
		else d = -Vec3::Dot(max /* - cameraPos */ , cubeNormals[i]);

		t = Ray::CalculatePlaneIntersection(cubeNormals[i], ray, d);
		if (t < 0.) continue; //intersection behind ray origin

		// determine if new closest intersection
		if (t < out->distance) {
			out->distance = t;
			//out->position = ray.direction * t;
			//out->position = out->position + ray.origin;
		}
	}
}

void Octree::Init(Node* root)
{
	Vec3 center = Vec3(
		root->minBounds[0] + 0.5 * (root->maxBounds[0] - root->minBounds[0]),
		root->minBounds[1] + 0.5 * (root->maxBounds[1] - root->minBounds[1]),
		root->minBounds[2] + 0.5 * (root->maxBounds[2] - root->minBounds[2])
	);
	root->center = center;

	if (root->maxBounds[0] - root->minBounds[0] < (MIN_SIZE)) {
		return;
	}

	//create child nodes
	for (int i = 0; i < 8; i++) {
		Node* child = new Node();

#pragma region set bounds
		switch (i) {
		case FRONT_LEFT_TOP:
			child->maxBounds.Set(center[0], root->maxBounds[1], center[2]);
			child->minBounds.Set(root->minBounds[0], center[1], root->minBounds[2]);
			break;
		case FRONT_RIGHT_TOP:
			child->maxBounds.Set(root->maxBounds[0], root->maxBounds[1], center[2]);
			child->minBounds.Set(center[0], center[1], root->minBounds[2]);
			break;
		case FRONT_LEFT_BOT:
			child->maxBounds = center;
			child->minBounds = root->minBounds;
			break;
		case FRONT_RIGHT_BOT:
			child->maxBounds.Set(root->maxBounds[0], center[1], center[2]);
			child->minBounds.Set(center[0], root->minBounds[1], root->minBounds[2]);
			break;
		case BACK_LEFT_TOP:
			child->maxBounds.Set(center[0], root->maxBounds[1], root->maxBounds[2]);
			child->minBounds.Set(root->minBounds[0], center[1], center[2]);
			break;
		case BACK_RIGHT_TOP:
			child->maxBounds = root->maxBounds;
			child->minBounds = center;
			break;
		case BACK_LEFT_BOT:
			child->maxBounds.Set(center[0], center[1], root->maxBounds[2]);
			child->minBounds.Set(root->minBounds[0], root->minBounds[1], center[2]);
			break;
		case BACK_RIGHT_BOT:
			child->maxBounds.Set(root->maxBounds[0], center[1], root->maxBounds[2]);
			child->minBounds.Set(center[0], root->minBounds[1], center[2]);
			break;
		}
#pragma endregion

		//assign all geometry within region
		std::map<int, Tri*>::iterator it = root->tris.begin();
		while (it != root->tris.end()) {

			int inBoundsCount = 0;
			for (int v = 0; v < 3; v++) {
				//if (Util::withinBounds(it->second->verts[v].position, child->maxBounds, child->minBounds)) {
					inBoundsCount++;
				//}
			}

			if (inBoundsCount > 0)	child->tris.insert(std::pair<int, Tri*>(it->first, it->second));

			++it;
		}

		//set pointers
		root->children[i] = child;
		child->parent = root;

		//if (Util::doubleCompare(0.625, root->maxBounds[0] - root->minBounds[0])) printf("leaf node %i\n", child->tris.size());
		//else printf("node%i %i\n", i, child->tris.size());
		//if (child->tris.size() > 0) {
		//	printf("node%i %i\n", i, child->tris.size());
		//	printVec3(child->minBounds, "min");
		//	printVec3(child->maxBounds, "max");
		//}


		//recurse
		Init(child);
	}
}
