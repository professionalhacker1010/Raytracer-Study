#include "Mesh.h"
#include "Surface.h"
#include "Util.h"

Mesh::Mesh(const char* objFile, const char* texFile, int meshId)
{
    bindPoseTris = (Tri*)_aligned_malloc(sizeof(Tri) * MAX_TRIANGLES, ALIGN); //new Tri[MAX_TRIANGLES];//
    vertData = (TriVerts*)_aligned_malloc(sizeof(TriVerts) * MAX_TRIANGLES, ALIGN);// new TriVerts[MAX_TRIANGLES];//

    texture = new Surface(texFile);
    Vec2 UV[1024];
    Vec3 N[1024], P[1024];
    int UVs = 0, Ns = 0, Ps = 0, a, b, c, d, e, f, g, h, i, idx = 0;
    bool init = false;
    FILE* file = fopen(objFile, "r");

    int debug = 0;
    while (!feof(file))
    {
        char line[1024] = { 0 };
        fgets(line, 1023, file);
        float norm[3] = { 0 }, pos[3] = { 0 }, uv[2] = { 0 };
        if (line == strstr(line, "vt ")) {
            UVs++;
            sscanf(line + 3, "%f %f", &uv[0], &uv[1]);
            UV[UVs].Set(uv[0], uv[1]);
        }
        else if (line == strstr(line, "vn ")) {
            Ns++;
            sscanf(line + 3, "%f %f %f", &norm[0], &norm[1], &norm[2]);
            N[Ns].Set(norm[0], norm[1], norm[2]);
        }
        else if (line[0] == 'v') {
           Ps++;
           sscanf(line + 2, "%f %f %f", &pos[0], &pos[1], &pos[2]);
           P[Ps].Set(pos[0], pos[1], pos[2]);
        }

        if (line[0] != 'f') {
            continue;
        }
        else {
            sscanf(line + 3, "%i/%i/%i %i/%i/%i %i/%i/%i",
                &a, &b, &c, &d, &e, &f, &g, &h, &i);
        }

        //if (!init) {
        //    init = true;
        //    bindPoseTris = (Tri*)_aligned_malloc(sizeof(Tri) * numTris, ALIGN);
        //    vertData = (TriVerts*)_aligned_malloc(sizeof(TriVerts) * numTris, ALIGN);
        //}

        bindPoseTris[idx].verts[0] = P[a]; vertData[idx].norm[0] = N[c]; vertData[idx].uv[0] = UV[b];
        bindPoseTris[idx].verts[1] = P[d]; vertData[idx].norm[1] = N[f]; vertData[idx].uv[1] = UV[e];
        bindPoseTris[idx].verts[2] = P[g]; vertData[idx].norm[2] = N[i]; vertData[idx].uv[2] = UV[h];

        idx++;
    }
    fclose(file);

    numTris = idx;

    tris = (Tri*)_aligned_malloc(sizeof(Tri) * numTris, ALIGN); //new Tri[numTris];//
    for (int i = 0; i < numTris; i++) {
        bindPoseTris[i].CachedCalculations();
        tris[i] = bindPoseTris[i];
    }
    id = meshId;
 }

Mesh::~Mesh()
{
    //delete[] bindPoseTris;
    //delete[] vertData;
    //delete[] tris;
    _aligned_free(bindPoseTris);
    _aligned_free(vertData);
    _aligned_free(tris);
    delete texture;
}

void Mesh::Animate(float deltaTime)
{
	if ((rotation += (rotationSpeed * deltaTime)) > 2.0f * PI) rotation -= 2.0f * (float)PI;
	float a = sinf(rotation) * 0.5f;
#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < numTris; i++) {
		for (int j = 0; j < 3; j++) {
			Vec3 original = bindPoseTris[i].verts[j];
			float step = a * (original[1] - 0.2f) * 0.2f;
			float x = original[0] * cosf(step) - original[1] * sinf(step);
			float y = original[0] * sinf(step) + original[1] * cosf(step);
			tris[i].verts[j] = Vec3(x, y, original[2]);
			tris[i].CachedCalculations();
		}
	}
}