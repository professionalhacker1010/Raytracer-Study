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

bool Application::Init(GLFWwindow* window)
{
	//set up tlas, blas, meshes, and mesh instances
	for (int i = 0; i < NUM_MESHES; i++) {
		//int dummy = 0;
		meshes[i] = new Mesh("Assets/dragon.obj", "Assets/bricks.png", i);
		bvh[i] = new BVH(i, meshes[i]->numTris);
		bvh[i]->Rebuild();
	}

	meshInstances = (MeshInstance*)MALLOC64(sizeof(MeshInstance) * NUM_MESH_INST);
	bvhInstances = (BVHInstance*)MALLOC64(sizeof(BVHInstance) * NUM_MESH_INST); //new BVHInstance[numMeshInstances];//

	for (int i = 0; i < NUM_MESH_INST; i++) {
		meshInstances[i].Set(0, i); //= (MeshInstance*)_aligned_malloc(sizeof(MeshInstance), ALIGN); //new MeshInstance(meshes[0], i);
		bvhInstances[i].Set(meshInstances[i].meshId, i);
	}

	tlas = new TLAS(bvhInstances, NUM_MESH_INST);
	tlas->Rebuild();

	//lights
	lights[0].position = Vec3(3, 10, 2);
	lights[0].color = Vec3(255, 255, 255) / 255.0f;
	lights[1].position = Vec3(3, 10, -2);
	lights[1].color = Vec3(255, 255, 191) / 255.0f;
	ambientLight = Vec3(60, 56, 79) / 255.0f;

	camera = &Camera::Get();
	camera->Init(window);

	// load HDR sky
	int bpp = 0;
	skyPixels = stbi_loadf("Assets/sky_19.hdr", &skyWidth, &skyHeight, &skyBpp, 0);
	for (int i = 0; i < skyWidth * skyHeight * 3; i++) skyPixels[i] = sqrtf(skyPixels[i]);

	// prepare OpenCL and buffers
	tracer = new Kernel("Source/cl/kernels.cl", "render", window); //load and compile file, load render function

	textureBuffer = new Buffer(window, meshes[0]->texture->width * meshes[0]->texture->height * sizeof(unsigned int), meshes[0]->texture->pixels);
	skyboxBuffer = new Buffer(window, skyWidth * skyHeight * 3 * sizeof(float), skyPixels); //3 color channels per texel
	triBuffer = new Buffer(window, meshes[0]->numTris * sizeof(Tri), meshes[0]->tris);
	vertDataBuffer = new Buffer(window, MAX_TRIANGLES * sizeof(TriVerts), meshes[0]->vertData);
	bvhBuffer = new Buffer(window, bvh[0]->numNodes * sizeof(BVHNode), bvh[0]->nodes);
	bvhTriIdxBuffer = new Buffer(window, bvh[0]->numTris * sizeof(uint), bvh[0]->triIndices);
	bvhInstBuffer = new Buffer(window, NUM_MESH_INST * sizeof(BVHInstance), bvhInstances);
	meshInstBuffer = new Buffer(window, NUM_MESH_INST * sizeof(MeshInstance), meshInstances);
	tlasBuffer = new Buffer(window, sizeof(TLASNode) * tlas->numNodes, tlas->nodes);

	//send to gpu once
	textureBuffer->CopyToDevice();
	skyboxBuffer->CopyToDevice(); 
	triBuffer->CopyToDevice(); //no mesh animation, so mesh and bvh are static
	vertDataBuffer->CopyToDevice();
	bvhBuffer->CopyToDevice();
	bvhTriIdxBuffer->CopyToDevice();

	startFrameTime = clock();

	return true;
}

void Application::Tick(float deltaTime)
{
	clock_t startAnimTime = clock();

	AnimateScene(deltaTime);
	//send to gpu every frame
	bvhInstBuffer->CopyToDevice();
	meshInstBuffer->CopyToDevice();
	tlasBuffer->CopyToDevice();
	tracer->SetArguments(
		camera->renderTargetBuffer, textureBuffer,
		skyboxBuffer, 
		tlasBuffer,
		triBuffer, vertDataBuffer, bvhBuffer, bvhTriIdxBuffer,
		meshInstBuffer, bvhInstBuffer,
		camera->GetPosition(), camera->bottomLeft, camera->lengthX, camera->lengthY, 
		(int)skyWidth, (int)skyHeight,
		(int)meshes[0]->texture->width, (int)meshes[0]->texture->height);
	tracer->Run(WIDTH * HEIGHT); //use one thread per pixel
	camera->Render();
	totalDrawTime += clock() - startAnimTime;
	frames++;
	return;

	//AnimateScene(deltaTime);
	//DrawScene();
	//camera->Render();
	//totalDrawTime += clock() - startAnimTime;
}

Vec3 Application::DrawSkybox(Ray& ray) {
	unsigned int u = skyWidth * atan2f(ray.direction[2], ray.direction[0]) * INV2PI - 0.5f;
	unsigned int v = skyHeight * acosf(ray.direction[1]) * INVPI - 0.5f;
	unsigned int skyIdx = (u + v * skyWidth) % ((uint)skyWidth * (uint)skyHeight);
	return Vec3::Min(Vec3::One(), 0.65f * Vec3(skyPixels[skyIdx * 3], skyPixels[skyIdx * 3 + 1], skyPixels[skyIdx * 3 + 2]));
}

bool Application::RayCast(Ray& ray, Vertex& outVertex, HitInfo& hit, int ignoreID) {
	//step through bvh for triangles
	tlas->CalculateIntersection(ray, hit);

	if (hit.triId != -1) {
		MeshInstance& meshInst = meshInstances[hit.meshInstId];
		Mesh& mesh = *meshes[meshInst.meshId];
		TriVerts& vertData = mesh.vertData[hit.triId];
		Surface& tex = *(mesh.texture);

		Vec3 coord = Vec3(hit.u, hit.v, 1 - (hit.u + hit.v));

		//normal
		Vec3 norm = Vec3::BaryCoord(vertData.norm1, vertData.norm2, vertData.norm0, coord);
		norm = Mat4::Transform(norm, meshInst.GetTransform(), 0.0f).Normalized();
		outVertex.normal = norm;

		//texture
		Vec2 uv = Vec2::BaryCoord(vertData.uv1, Vec2(vertData.uv2x, vertData.uv2y), vertData.uv0, coord);
		int iu = (int)(uv[0] * (float)tex.width) % tex.width;
		int iv = (int)(uv[1] * (float)tex.height) % tex.height;
		unsigned int texel = tex.pixels[iu + iv * tex.width];
		outVertex.albedo = RGB8toRGB32F(texel);

		//position
		outVertex.position = ray.origin + hit.distance * ray.direction + meshInst.GetTransform().GetTranslation();

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
		Vec3 dir = lights[i].position - vertex.position;
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

Vec3 Application::CastMirrorRays(Ray& ray, Vertex& vertex) {
	int depth = 0;
	Ray primary = ray;
	while (depth < MAX_RAY_DEPTH) {
		Ray secondary;
		Vec3 dir = (primary.direction - 2 * vertex.normal * Vec3::Dot(vertex.normal, primary.direction)).Normalized();
		Vec3 pos = vertex.position + dir * 0.001f;
		secondary.Set(pos, dir);

		HitInfo hitInfo;
		int hit = RayCast(secondary, vertex, hitInfo);

		if (hit) {
			if (hitInfo.meshInstId % 3 != 0) {
				return CastShadowRays(vertex);
			}
			else {
				depth++;
				primary = secondary;
				continue;
			}
		}
		else {
			return DrawSkybox(secondary);
		}
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
	static bool once = false;
	if (!once) {
		static float angle = 0; angle += 0.5f * deltaTime;
		Mat4 M1 = Mat4::CreateRotationY(angle); //Mat4 M2 = Mat4::CreateRotationX(0.65f) * M1;
		Vec3 camPos = Mat4::Transform(Vec3(0.0f, 1.0f, 6.5f), M1);
		camera->SetTransform(camPos, M1);

		for (int i = 0; i < NUM_MESH_INST; i++) {
			meshInstances[i].SetTransform(Mat4::CreateTranslation(Vec3(bvhInstances[i].meshInstId, 0.0f, 0.0f)));
			bvhInstances[i].SetTransform(Mat4::CreateTranslation(Vec3(bvhInstances[i].meshInstId, 0.0f, 0.0f)));
		}
		//once = true;
	}

	
	//mesh instance transformations
	static float a[16] = { 0 }, h[16] = { 5, 4, 3, 2, 1, 5, 4, 3 }, s[16] = { 0 };
	for (int i = 0, x = 0; x < 3; x++) for (int y = 0; y < 3; y++, i++)
	{
		Mat4 R, T = Mat4::CreateTranslation(Vec3((x - 1.5f) * 2.5f, 0, (y - 1.5f) * 2.5f));
		if ((x + y) & 1) R = Mat4::CreateRotationX(a[i]) * Mat4::CreateRotationZ(a[i]);
		else R = Mat4::CreateTranslation(Vec3(0, h[i / 2], 0));
		if ((a[i] += (((i * 13) & 7) + 2) * 0.005f) > 2 * PI) a[i] -= 2 * PI;
		if ((s[i] -= 0.01f, h[i] += s[i]) < 0) s[i] = 0.2f;
		Mat4 transform = Mat4::CreateScale(0.01f) * R * T * Mat4::CreateTranslation(Vec3(2.0f, 0.0f, 0.0f));
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
							color = CastShadowRays(vert);
						else
							color = CastMirrorRays(ray, vert);
					}
					else {
						//draw skybox
						color = DrawSkybox(ray);
					}
				}

				raycastTime += (clock() - startRayTime);

				color *= 255.0f;
				for (i = 0; i < 3; i++) {
					camera->pixelData[v][u][i] = (GLubyte)color[i];
				}
				camera->pixelData[v][u][3] = (GLubyte)255.0f;
			}
		}
	}

	drawTime += (clock() - startDrawTime);
	triIntersections += meshes[0]->tris[0].debug();

	frames++;
}

void Application::KeyDown(int key) {
	if (key == 'Q' || key == 'q') {
		exit(0);
		system("pause");
	}
	if (key == 'p' || key == 'P') system("pause");
}

void Application::Shutdown() {
	Util::Print("Avg FPS = " + std::to_string((float)frames / ((float)totalDrawTime / 1000.0f)));
	Util::Print("Avg BVH construction secs = " + std::to_string(buildTime / ((float)frames * 1000.0f)));
	Util::Print("Avg tri intersections = " + std::to_string(triIntersections / ((float)frames)));
	Util::Print("Avg ms per raycast = " + std::to_string(raycastTime / ((float)(WIDTH * HEIGHT))));
	Util::Print("Avg draw secs = " + std::to_string(drawTime / ((float)frames * 1000.0f)));

	delete tlas;

	for (int i = 0; i < NUM_MESHES; i++) {
		delete meshes[i];
		delete bvh[i];
	}

	FREE64(bvhInstances);
	FREE64(meshInstances);

	delete skyPixels;
	delete skyboxBuffer;
	delete tracer;
	system("pause");
}