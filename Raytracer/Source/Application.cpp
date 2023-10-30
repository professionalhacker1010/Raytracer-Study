#include "stdafx.h"
#include "Application.h"
#include "BVH.h"
#include "Mesh.h"
#include "MeshInstance.h"
#include "Camera.h"
#include "RayCast.h"
#include "RenderQuad.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "External/stb_image.h"


//debug
int frames = 0;
long int triIntersections = 0;
clock_t totalTime;
clock_t startFrameTime = 0;
clock_t totalDrawTime = 0;
clock_t raycastTime = 0;
clock_t drawTime = 0;
clock_t buildTime = 0;
Vec3 colors[] = {
	Vec3(1, 0, 0), //red
	Vec3(0, 1, 0), //green
	Vec3(0, 0, 1), //blue
	Vec3(1, 1, 0), //yellow
	Vec3(0, 1, 1), //teal
	Vec3(1, 0, 1), //pink
};

bool Application::Init()
{
	for (int i = 0; i < NUM_MESHES; i++) {
		//int dummy = 0;
		meshes[i] = new Mesh("Assets/teapot.obj", "Assets/bricks.png", i);
		bvh[i] = new BVH(meshes[i]->tris, meshes[i]->numTris);
		bvh[i]->Rebuild();
	}

	meshInstances = (MeshInstance*)MALLOC64(sizeof(MeshInstance) * NUM_MESH_INST);
	bvhInstances = (BVHInstance*)MALLOC64(sizeof(BVHInstance) * NUM_MESH_INST); //new BVHInstance[numMeshInstances];//

	for (int i = 0; i < NUM_MESH_INST; i++) {
		meshInstances[i].Set(meshes[0], i); //= (MeshInstance*)_aligned_malloc(sizeof(MeshInstance), ALIGN); //new MeshInstance(meshes[0], i);
		bvhInstances[i].Set(bvh[meshInstances[i].meshRef->id], &meshInstances[i]);
	}

	tlas = new TLAS(bvhInstances, NUM_MESH_INST);
	int size = sizeof(MeshInstance);
	int size2 = sizeof(Mesh);
	int size3 = sizeof(BVH);
	int size4 = sizeof(TLAS);
	int size5 = sizeof(RenderQuad);
	int size6 = sizeof(BVHInstance);
	int size7 = sizeof(BVHNode);
	Util::Print("Sizeof BVH node = " + std::to_string(size7));

	// load HDR sky
	int bpp = 0;
	skyPixels = stbi_loadf("Assets/sky_19.hdr", &skyWidth, &skyHeight, &skyBpp, 0);
	for (int i = 0; i < skyWidth * skyHeight * 3; i++) skyPixels[i] = sqrtf(skyPixels[i]);

	tlas->Rebuild();

	lights[0].position = Vec3(3, 10, 2);
	lights[0].color = Vec3(255, 255, 255) / 255.0f;
	lights[1].position = Vec3(3, 10, -2);
	lights[1].color = Vec3(255, 255, 191) / 255.0f;
	ambientLight = Vec3(60, 56, 79) / 255.0f;

	renderQuad = new RenderQuad();
	camera = &Camera::Get();
	//->SetTransform(Vec3(-2.0f, 0.0f, 2.0f), Mat4::Identity());

	startFrameTime = clock();

	return true;
}

void Application::Tick(float deltaTime)
{
	clock_t startAnimTime = clock();
	AnimateScene(deltaTime);
	DrawScene();
	renderQuad->Draw(pixelData);
	totalDrawTime += clock() - startAnimTime;
}


bool Application::RayCast(Ray& ray, Vertex& outVertex, HitInfo& hit, int ignoreID) {
	//step through bvh for triangles
	tlas->CalculateIntersection(ray, hit);

	if (hit.triId != -1) {
		TriVerts& vertData = meshInstances[hit.meshInstId].meshRef->vertData[hit.triId];
		Surface& tex = *(meshInstances[hit.meshInstId].meshRef->texture);

		Vec3 coord = Vec3(hit.u, hit.v, 1 - (hit.u + hit.v));

		//normal
		Vec3 norm = Vec3::BaryCoord(vertData.norm[1], vertData.norm[2], vertData.norm[0], coord);
		norm = Mat4::Transform(norm, meshInstances[hit.meshInstId].GetTransform(), 0.0f).Normalized();
		outVertex.normal = norm;

		//texture
		Vec2 uv = Vec2::BaryCoord(vertData.uv[1], vertData.uv[2], vertData.uv[0], coord);
		int iu = (int)(uv[0] * (float)tex.width) % tex.width;
		int iv = (int)(uv[1] * (float)tex.height) % tex.height;
		unsigned int texel = tex.pixels[iu + iv * tex.width];
		outVertex.albedo = RGB8toRGB32F(texel);

		//position
		outVertex.position = ray.origin + hit.distance * ray.direction;

		return true;
	}

	return false;
}

Vec3 Application::CastShadowRays(const Vertex& vertex) {
	Vec3 color = ambientLight;

	//specular light setup
	//Vec3 toCamera = Camera::Get().position - vertex.position;
	//toCamera.Normalize();
	Ray ray;
	ray.origin = vertex.position + vertex.normal * 0.001f;
	for (int i = 0; i < NUM_LIGHTS; i++) {
		//set direction and max dist of raycast
		Vec3 dir = (lights[i].position + camera->GetInvPosition()) - vertex.position;
		float dist = dir.Length();
		dir *= 1.0f / dist;
		ray.direction = dir;
		ray.dInv = Vec3::One() / dir;

		//if the raycast hit something don't calculate lighting
		HitInfo hitInfo;
		if (tlas->CalculateIntersection(ray, hitInfo)) {
			continue;
		}

		float specular = 0;

		//diffuse light
		float diffuse = fmaxf(Vec3::Dot(vertex.normal, ray.direction), 0.0f);

		//specular light
		//Vec3 reflector = vertex.normal * (2. * diffuseStrength);
		//Vec3 reflectedLightVector = reflector - ray.direction;
		//double specularStrength = pow(Vec3::Dot(toCamera, reflectedLightVector), vertex.shininess);
		//specularStrength = fmax(specularStrength, 0.);
		//specular = vertex.color_specular * specularStrength;

		//final color
		color += (diffuse + specular) * lights[i].color; //* (1.0f / (dist * dist));
	}
	color = color * vertex.albedo;

	//clamp color
	color = Vec3::Min(color, Vec3::One());

	//return final color
	return color;
}

Vec3 Application::CastMirrorRays(Ray& ray, Vertex& vertex, int rayDepth) {
	if (rayDepth >= MAX_RAY_DEPTH) {
		return Vec3::Zero();
	}

	Ray secondary;
	Vec3 dir = (ray.direction - 2 * vertex.normal * Vec3::Dot(vertex.normal, ray.direction)).Normalized();
	Vec3 pos = vertex.position + dir * 0.001f;
	secondary.Set(pos, dir);


	HitInfo hitInfo;
	int hit = RayCast(secondary, vertex, hitInfo);

	if (hit) {
		if (hitInfo.meshInstId % 3 != 0)
			return CastShadowRays(vertex);
		else
			return CastMirrorRays(ray, vertex, rayDepth + 1);
	}
	else {
		//draw skybox
		unsigned int u = skyWidth * atan2f(secondary.direction[2], secondary.direction[0]) * INV2PI - 0.5f;
		unsigned int v = skyHeight * acosf(secondary.direction[1]) * INVPI - 0.5f;
		unsigned int skyIdx = u + v * skyWidth;
		if (skyIdx < skyWidth * skyHeight)
			return Vec3::Min(Vec3::One(), 0.65f * Vec3(skyPixels[skyIdx * 3], skyPixels[skyIdx * 3 + 1], skyPixels[skyIdx * 3 + 2]));
	}

	return Vec3::Zero();
}

void Application::AnimateScene(float deltaTime) {
	clock_t c = clock();
	totalTime += (c - startFrameTime);
	startFrameTime = c;

	//mesh animation
	for (int i = 0; i < NUM_MESHES; i++) {
		//meshes[i]->Animate(deltaTime);
	}

	//update camera transform
	static float angle = 0; angle += 0.5f * deltaTime;
	Mat4 M1 = Mat4::CreateRotationY(angle);// M2 = Mat4::CreateRotationX(-0.65f) * M1;
	Vec3 camPos = Mat4::Transform(Vec3(0.0f, 1.0f, 6.5f), M1);
	camera->SetTransform(camPos, M1);

	//mesh instance transformations
	static float a[16] = { 0 }, h[16] = { 5, 4, 3, 2, 1, 5, 4, 3 }, s[16] = { 0 };
	for (int i = 0, x = 0; x < 3; x++) for (int y = 0; y < 3; y++, i++)
	{
		Mat4 R, T = Mat4::CreateTranslation(Vec3((x - 1.5f) * 2.5f, 0, (y - 1.5f) * 2.5f));
		if ((x + y) & 1) R = Mat4::CreateRotationX(a[i]) * Mat4::CreateRotationZ(a[i]);
		else R = Mat4::CreateTranslation(Vec3(0, h[i / 2], 0));
		if ((a[i] += (((i * 13) & 7) + 2) * 0.005f) > 2 * PI) a[i] -= 2 * PI;
		if ((s[i] -= 0.01f, h[i] += s[i]) < 0) s[i] = 0.2f;
		Mat4 transform = Mat4::CreateScale(0.75f) * R * T * Mat4::CreateTranslation(Vec3(2.0f, 0.0f, 0.0f));
		meshInstances[i].SetTransform(transform);
		bvhInstances[i].SetTransform(transform);
	}

	clock_t startBuildTime = clock();
	for (int i = 0; i < NUM_MESHES; i++) {
		//bvh[i]->Refit();
	}
	tlas->Rebuild();
	c = clock();
	buildTime += (c - startBuildTime);
}

void Application::DrawScene()
{
	clock_t startDrawTime = clock();

	Vertex vert;
	const int tileSize = 4;
	int x, y, u, v, i;

#pragma omp parallel for schedule(dynamic) reduction(+:raycastTime), private(y, u, v, i, vert)
	for (x = 0; x < WIDTH; x += tileSize) {

		Ray ray;
		Vec3 color;
		for (y = 0; y < HEIGHT; y += tileSize) {
			for (u = x; u < x + tileSize; u++) for (v = y; v < y + tileSize; v++) {
				//if i used x and y here it gave a kinda cool pixellated effect lol
				clock_t startRayTime = clock();
				for (i = 0; i < 4; i++) {
					ray = Camera::Get().GetRay(u, v);


					HitInfo hitInfo;
					int hit = RayCast(ray, vert, hitInfo);

					color = Vec3::Zero();
					if (hit) {
						if (hitInfo.meshInstId % 3 != 0)
							color = CastShadowRays(vert) * 255.0f;
						else
							color = CastMirrorRays(ray, vert) * 255.0f;
					}
					else {
						//draw skybox
						unsigned int u = skyWidth * atan2f(ray.direction[2], ray.direction[0]) * INV2PI - 0.5f;
						unsigned int v = skyHeight * acosf(ray.direction[1]) * INVPI - 0.5f;
						unsigned int skyIdx = u + v * skyWidth;
						if (skyIdx < skyWidth * skyHeight)
							color = Vec3::Min(Vec3::One(), 0.65f * Vec3(skyPixels[skyIdx * 3], skyPixels[skyIdx * 3 + 1], skyPixels[skyIdx * 3 + 2])) * 255.0f;
					}
				}

				raycastTime += (clock() - startRayTime);

				for (i = 0; i < 3; i++) {
					pixelData[v][u][i] = (GLubyte)color[i];
				}
			}
		}
	}

	drawTime += (clock() - startDrawTime);
	triIntersections += meshes[0]->tris[0].debug();

	frames++;
}

void Application::KeyDown(int key) {
	if (key == 'Q' || key == 'q') exit(0);
	if (key == 'p') system("pause");
}

void Application::Shutdown() {
	Util::Print("Avg FPS = " + std::to_string((float)frames / ((float)totalDrawTime / 1000.0f)));
	Util::Print("Avg BVH construction secs = " + std::to_string(buildTime / ((float)frames * 1000.0f)));
	Util::Print("Avg tri intersections = " + std::to_string(triIntersections / ((float)frames)));
	Util::Print("Avg ms per raycast = " + std::to_string(raycastTime / ((float)(WIDTH * HEIGHT))));
	Util::Print("Avg draw secs = " + std::to_string(drawTime / ((float)frames * 1000.0f)));

	FREE64(bvhInstances);
	FREE64(meshInstances);

	for (int i = 0; i < NUM_MESHES; i++) {
		delete meshes[i];
		delete bvh[i];
	}

	delete tlas;
	delete renderQuad;
	system("pause");
}