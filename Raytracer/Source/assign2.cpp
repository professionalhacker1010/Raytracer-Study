// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 480 Computer Graphics
	Assignment 2: Simulating a Roller Coaster
	Name: Sarah Yuen
	Email: skyuen@usc.edu
*/

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iostream>
#include <GL/glu.h>
#include <GL/glut.h>
#include "glext.h"
#include <map>
#include <string>

#include "opencv2/core/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "util.h"

int g_iMenuId;

/* camera */
const float fov = 60.0f;
const float initWindowHeight = 480.0f;
const float initWindowWidth = 640.0f;
const float initWindowPos[] = { 100.0f, 100.0f };

/* mouse ctrls */
int g_vMousePos[2] = { 0, 0 };
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

typedef enum { NORMAL, DEBUG } CAMERASTATE;
CAMERASTATE g_CameraState = DEBUG;

/* state of the world */
float g_vLandRotate[3] = { 0.0, 0.0, 0.0 };
float g_vLandTranslate[3] = { 0.0, 0.0, 0.0 };
float g_vLandScale[3] = { 1.0, 1.0, 1.0 };

/* transform offsets */
const float deltaRotate = 10.0f;
const float translateOffset[] = { 0.0f, 0.0f, -20.0f };

/* Object where you can load an image */
cv::Mat3b imageBGR;

/* represents one control point along the spline */
struct point {
	point(double x_, double y_, double z_) {
		x = x_;
		y = y_;
		z = z_;
	}
	double x;
	double y;
	double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
	int numControlPoints;
	struct point *points;
	double*** CRBasisControlMatrices;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

/* textures */
std::map<std::string, GLuint> textures;

/*skybox*/
std::string skyboxFaces[6] = {
	"cityBG",
	"cityBG",
	"cityBG",
	"cityBG",
	"nightSky",
	"bottom"
};
double skyboxVerts[] = {
	//front
	-1.0, -1.0, -.999,
	-1.0, 1.0, -.999,
	1.0, 1.0, -.999,
	1.0, -1.0, -.999,

	//right
	1.0, -1.0, -1.0,
	1.0, 1.0, -1.0,
	1.0, 1.0, 1.0,
	1.0, -1.0, 1.0,

	//left
	-1.0, -1.0, 1.0,
	-1.0, 1.0, 1.0,
	-1.0, 1.0, -1.0,
	-1.0, -1.0, -1.0,

	//back
	1.0, -1.0, 1.0,
	1.0, 1.0, 1.0,
	-1.0, 1.0, 1.0,
	-1.0, -1.0, 1.0,

	//top
	-1.0, .999, -1.0,
	-1.0, .999, 1.0,
	1.0, .999, 1.0,
	1.0, .999, -1.0,

	//bottom
	-1.0, -1.0, 1.0,
	-1.0, -1.0, -1.0,
	1.0, -1.0, -1.0,
	1.0, -1.0, 1.0,
};

/* time */
int elapsedTime = 0;

/*objects*/
struct mesh {
	double** vertices;
	double** normals;
	double** texCoords;
	int* indices;
	int numIndices;
	int numVerts;
};
struct transform {
	double position[3];
	double rotation[3];
	double scale[3];
};
struct object {
	mesh* mesh;
	transform transform;
	float repeatTextureS;
	float repeatTextureT;
	char* texture;
};
double vector0[3] = { 0,0,0 };

/* splines */
double stepU = 0.01;
double splineThickness = 0.01;
double trackWidthHeightRatio = 10.0;
struct mesh* g_SplineMeshes;
struct object* g_SplineObjects;

/* bumpers */
double bumperHeight = 0.03;
double bumperWidth = 0.03;
struct mesh* g_BumperMeshes;
struct object* g_BumperObjects;

/*props*/
int numPropMeshes = 2;
struct mesh* g_PropMeshes;
int numProps = 6;
struct object* g_Props;

/* camera movement */
double camPos[3];
double camPosUpOffset = 0.1;
double camForward[3];
double camUp[3];
double prevBinormal[3];

double posU = 0;
double deltaPosU = 0.02;
int currPoint = 0;
int currSpline = 0;

int debugPoint = -1;

#pragma region Mouse ctrls

void menufunc(int value)
{
	switch (value)
	{
	case 0:
		exit(0);
		break;
	}
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const double width = glutGet(GLUT_WINDOW_WIDTH);
	const double height = glutGet(GLUT_WINDOW_HEIGHT);
	gluPerspective(fov, width / height, 0.1, 1000.0);
}

/* converts mouse drags into information about
rotation/translation/scaling */
void mousedrag(int x, int y)
{
	int vMouseDelta[2] = { x - g_vMousePos[0], y - g_vMousePos[1] };

	switch (g_ControlState)
	{
	case TRANSLATE:
		if (g_iLeftMouseButton)
		{
			g_vLandTranslate[0] += vMouseDelta[0] * 0.01;
			g_vLandTranslate[1] -= vMouseDelta[1] * 0.01;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandTranslate[2] += vMouseDelta[1] * 0.01;
		}
		break;
	case ROTATE:
		if (g_iLeftMouseButton)
		{
			g_vLandRotate[0] += (vMouseDelta[1]);
			g_vLandRotate[1] += (vMouseDelta[0]);
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandRotate[2] += (vMouseDelta[1]);
		}
		break;
	case SCALE:
		if (g_iLeftMouseButton)
		{
			g_vLandScale[0] *= 1.0 + vMouseDelta[0] * 0.01;
			g_vLandScale[1] *= 1.0 - vMouseDelta[1] * 0.01;

			if (g_vLandScale[0] < 0.0f) g_vLandScale[0] = 0.0f;
			if (g_vLandScale[1] < 0.0f) g_vLandScale[1] = 0.0f;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandScale[2] *= 1.0 - vMouseDelta[1] * 0.01;

			if (g_vLandScale[2] < 0.0f) g_vLandScale[2] = 0.0f;
		}
		break;
	}
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		g_iLeftMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_MIDDLE_BUTTON:
		g_iMiddleMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_RIGHT_BUTTON:
		g_iRightMouseButton = (state == GLUT_DOWN);
		break;
	}

	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		g_ControlState = TRANSLATE;
		break;
	case GLUT_ACTIVE_SHIFT:
		g_ControlState = SCALE;
		break;
	default:
		g_ControlState = ROTATE;
		break;
	}

	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}
#pragma endregion

int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf ("can't open file\n");
		exit(1);
	}
  
	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);
	printf("%d\n", g_iNumOfSplines);
	g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf ("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
		g_Splines[j].numControlPoints = iLength;

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%lf %lf %lf", 
			&g_Splines[j].points[i].x, 
			&g_Splines[j].points[i].y, 
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return 0;
}

void initCamVectors(spline s) {
	setVec3(camPos, getSplinePoint(0.0, s.CRBasisControlMatrices[0]));
	double tan0[3];
	double norm0[3] = { 0.0, 1.0, 0.0 };
	setVec3(tan0, getSplineTangent(0.0, s.CRBasisControlMatrices[0]));
	setVec3(camForward, tan0);
	setVec3(camUp, norm0);
	crossVec3(tan0, norm0, prevBinormal);
}

void myInit() {
	/* Catmull-Rom spline data */
	double s = 0.5;
	double CRBasisMatrix[4][4] = {
		{-s, 2 - s, s - 2, s},
		{2 * s, s - 3, 3 - 2 * s, -s},
		{-s, 0.0, s, 0.0},
		{0.0, 1.0, 0.0, 0.0}
	};

	for (int j = 0; j < g_iNumOfSplines; j++) {
		/* premultiply basis - control point matrices */
		g_Splines[j].CRBasisControlMatrices = (double***)malloc((g_Splines[j].numControlPoints - 3) * sizeof(double**));
		for (int k = 0; k < g_Splines[j].numControlPoints - 3; k++) {
			point p1 = g_Splines[j].points[k];
			point p2 = g_Splines[j].points[k + 1];
			point p3 = g_Splines[j].points[k + 2];
			point p4 = g_Splines[j].points[k + 3];

			double controlMatrix[4][3] = {
				{p1.x, p1.y, p1.z},
				{p2.x, p2.y, p2.z},
				{p3.x, p3.y, p3.z},
				{p4.x, p4.y, p4.z}
			};

			g_Splines[j].CRBasisControlMatrices[k] = (double**)malloc(4 * sizeof(double*));
			// for each row in m1
			for (int i = 0; i < 4; i++) {
				g_Splines[j].CRBasisControlMatrices[k][i] = (double*)malloc(3 * sizeof(double));
				// for each col in m2
				for (int l = 0; l < 3; l++) {
					// dot product
					double colVec[4] = { controlMatrix[0][l], controlMatrix[1][l], controlMatrix[2][l], controlMatrix[3][l] };
					g_Splines[j].CRBasisControlMatrices[k][i][l] = dotVec4(CRBasisMatrix[i], colVec);
				}
			}
		}
	}

	double startVec[3] = { -1.0, 0.0, 0.0 };

	/* spline mesh data */
	#pragma region Spline mesh generation
	g_SplineMeshes = (mesh*)malloc(g_iNumOfSplines * sizeof(mesh));
	g_SplineObjects = (object*)malloc(g_iNumOfSplines * sizeof(object));
	g_BumperMeshes = (mesh*)malloc(g_iNumOfSplines * 6 * sizeof(mesh));
	g_BumperObjects = (object*)malloc(g_iNumOfSplines * 6 * sizeof(object));
	for (int j = 0, b = 0; j < g_iNumOfSplines; j++, b+=6) {

		// allocate road mesh memory
		g_SplineMeshes[j].numVerts = (1.0 / stepU) * (g_Splines[j].numControlPoints - 3) * 6 + 6; // verts per cross section
		g_SplineMeshes[j].vertices = (double**)malloc(g_SplineMeshes[j].numVerts * sizeof(double*));
		g_SplineMeshes[j].normals = (double**)malloc(g_SplineMeshes[j].numVerts * sizeof(double*));
		for (int k = 0; k < g_SplineMeshes[j].numVerts; k++) {
			g_SplineMeshes[j].vertices[k] = (double*)malloc(3 * sizeof(double));
			g_SplineMeshes[j].normals[k] = (double*)malloc(3 * sizeof(double));
		}
		g_SplineMeshes[j].numIndices = (g_SplineMeshes[j].numVerts - 6) * 14 / 6; // indices per cross section
		g_SplineMeshes[j].indices = (int*)malloc(g_SplineMeshes[j].numIndices * sizeof(int));
		g_SplineMeshes[j].texCoords = (double**)malloc(g_SplineMeshes[j].numIndices * sizeof(double*));

		// road objects
		g_SplineObjects[j].mesh = &g_SplineMeshes[j];
		g_SplineObjects[j].texture = "roadArrow";
		setVec3(g_SplineObjects[j].transform.position, vector0);
		setVec3(g_SplineObjects[j].transform.rotation, vector0);
		setVec3(g_SplineObjects[j].transform.scale, vector0);
		g_SplineObjects[j].repeatTextureS = 1; g_SplineObjects[j].repeatTextureT = 1;

		//allocate bumper mesh memory and objects
		for (int k = 0; k < 6; k++) {
			g_BumperMeshes[b + k].numVerts = (1.0 / stepU) * (g_Splines[j].numControlPoints - 3) * 2; // verts per cross section
			g_BumperMeshes[b + k].vertices = (double**)malloc(g_BumperMeshes[b + k].numVerts * sizeof(double*));
			g_BumperMeshes[b + k].normals = (double**)malloc(g_BumperMeshes[b + k].numVerts * sizeof(double*));
			for (int c = 0; c < g_BumperMeshes[b + k].numVerts; c++) {
				g_BumperMeshes[b + k].vertices[c] = (double*)malloc(3 * sizeof(double));
				g_BumperMeshes[b + k].normals[c] = (double*)malloc(3 * sizeof(double));
			}
			g_BumperMeshes[b + k].numIndices = -1; //-1 signals to use order of vertices
			g_BumperMeshes[b + k].indices = nullptr;
			g_BumperMeshes[b + k].texCoords = (double**)malloc(g_BumperMeshes[b + k].numVerts * sizeof(double*));
			
			g_BumperObjects[b + k].mesh = &g_BumperMeshes[b + k];
			setVec3(g_BumperObjects[b + k].transform.position, vector0);
			setVec3(g_BumperObjects[b + k].transform.rotation, vector0);
			setVec3(g_BumperObjects[b + k].transform.scale, vector0);
			g_BumperObjects[b + k].repeatTextureS = 1; g_BumperObjects[b + k].repeatTextureT = 1;
		}

		//bumper textures
		g_BumperObjects[b].texture = "bumperArrow";
		g_BumperObjects[b + 1].texture = "bumperTop";
		g_BumperObjects[b + 2].texture = "roadSide";
		g_BumperObjects[b+3].texture = "bumperArrow";
		g_BumperObjects[b + 4].texture = "bumperTop";
		g_BumperObjects[b + 5].texture = "roadSide";

		// init direction vectors
		double* tan0 = getSplineTangent(0.0, g_Splines[j].CRBasisControlMatrices[0]);
		double norm0[3] = { 0.0, 1.0, 0.0 };
		double binorm0[3];
		crossVec3(tan0, norm0, binorm0);
		
		int v = 0, v2 = 0, i = 0;
		double edgeS = 0.1, deltaT = -0.1, t = 0.0;
		// for each point
		for (int k = 0; k < g_Splines[j].numControlPoints - 3; k++) {
			// for each u step
			for (double u = 0; u <= 1.0; u += stepU) {

				if (u > 0.999 && k < g_Splines[j].numControlPoints - 4) continue;

				// calc direction vectors
				double point[3];
				double tan[3];
				double norm[3];
				setVec3(point, getSplinePoint(u, g_Splines[j].CRBasisControlMatrices[k]));
				setVec3(tan, normalizeVec3(getSplineTangent(u, g_Splines[j].CRBasisControlMatrices[k])));
				setVec3(norm, crossVec3(binorm0, tan));
				setVec3(norm, normalizeVec3(norm));
				setVec3(binorm0, crossVec3(tan, norm));
				setVec3(binorm0, normalizeVec3(binorm0));	

				double temp[3];

				#pragma region ROAD MESH
				//bottom right
				setVec3(temp, mulVec3(binorm0, trackWidthHeightRatio));
				setVec3(temp, subVec3(temp, norm));
				setVec3(temp, mulVec3(temp, splineThickness));
				setVec3(temp, addVec3(point, temp));
				setVec3(g_SplineMeshes[j].vertices[v], temp);
				setVec3(g_SplineMeshes[j].normals[v], binorm0);

				//top right
				setVec3(temp, mulVec3(binorm0, trackWidthHeightRatio));
				setVec3(temp, addVec3(temp, norm));
				setVec3(temp, mulVec3(temp, splineThickness));
				setVec3(temp, addVec3(point, temp));
				setVec3(g_SplineMeshes[j].vertices[v+1], temp);
				setVec3(g_SplineMeshes[j].normals[v+1], binorm0);

				//top middle
				setVec3(temp, mulVec3(norm, splineThickness));
				setVec3(temp, addVec3(point, temp));
				setVec3(g_SplineMeshes[j].vertices[v + 2], temp);
				setVec3(g_SplineMeshes[j].normals[v+2], norm);

				//top left
				setVec3(temp, mulVec3(binorm0, trackWidthHeightRatio));
				setVec3(temp, subVec3(norm, temp));
				setVec3(temp, mulVec3(temp, splineThickness));
				setVec3(temp, addVec3(point, temp));
				setVec3(g_SplineMeshes[j].vertices[v+3], temp);
				setVec3(g_SplineMeshes[j].normals[v + 3], mulVec3(binorm0, -1));

				//bottom left
				setVec3(temp, mulVec3(binorm0, trackWidthHeightRatio));
				setVec3(temp, addVec3(temp, norm));
				setVec3(temp, mulVec3(temp, -splineThickness));
				setVec3(temp, addVec3(point, temp));
				setVec3(g_SplineMeshes[j].vertices[v+4], temp);
				setVec3(g_SplineMeshes[j].normals[v + 4], mulVec3(binorm0, -1));

				//bottom middle
				setVec3(temp, mulVec3(norm, -splineThickness));
				setVec3(temp, addVec3(point, temp));
				setVec3(g_SplineMeshes[j].vertices[v + 5], temp);
				setVec3(g_SplineMeshes[j].normals[v + 5], mulVec3(norm, -1));

				if (u < 0.999) {
					//store indices
					for (int c = 0, d = 0; c < 12; c += 2, d++) {
						g_SplineMeshes[j].indices[i + c] = v + d;
						g_SplineMeshes[j].indices[i + c + 1] = v + d + 6;
					}
					g_SplineMeshes[j].indices[i + 12] = v;
					g_SplineMeshes[j].indices[i + 13] = v + 6;

					//store texcoords
					g_SplineMeshes[j].texCoords[i] = new double[2]{ -edgeS,t };
					g_SplineMeshes[j].texCoords[i + 1] = new double[2]{ -edgeS,t + deltaT };
					g_SplineMeshes[j].texCoords[i + 2] = new double[2]{ edgeS,t };
					g_SplineMeshes[j].texCoords[i + 3] = new double[2]{ edgeS,t + deltaT };
					g_SplineMeshes[j].texCoords[i + 4] = new double[2]{ 1.0 ,t };
					g_SplineMeshes[j].texCoords[i + 5] = new double[2]{ 1.0 ,t + deltaT };
					g_SplineMeshes[j].texCoords[i + 6] = new double[2]{ edgeS,t };
					g_SplineMeshes[j].texCoords[i + 7] = new double[2]{ edgeS,t + deltaT };
					g_SplineMeshes[j].texCoords[i + 8] = new double[2]{ -edgeS,t };
					g_SplineMeshes[j].texCoords[i + 9] = new double[2]{ -edgeS,t + deltaT };
					g_SplineMeshes[j].texCoords[i + 10] = new double[2]{ -1.0,t };
					g_SplineMeshes[j].texCoords[i + 11] = new double[2]{ -1.0,t + deltaT };
					g_SplineMeshes[j].texCoords[i + 12] = new double[2]{ -edgeS,t };
					g_SplineMeshes[j].texCoords[i + 13] = new double[2]{ -edgeS,t + deltaT };
				}
				#pragma endregion

				#pragma region BUMPER MESHES
				//left bumper - right face
				setVec3(g_BumperMeshes[b].vertices[v2], g_SplineMeshes[j].vertices[v + 3]);
				setVec3(temp, mulVec3(norm, bumperHeight));
				setVec3(g_BumperMeshes[b].vertices[v2+1], addVec3(g_BumperMeshes[b].vertices[v2], temp));
				setVec3(g_BumperMeshes[b].normals[v2], mulVec3(binorm0, -1));
				setVec3(g_BumperMeshes[b].normals[v2+1], mulVec3(binorm0, -1));
				
				//left bumper - top face
				setVec3(g_BumperMeshes[b+1].vertices[v2], g_BumperMeshes[b].vertices[v2 + 1]);
				setVec3(temp, mulVec3(binorm0, -bumperWidth));
				setVec3(g_BumperMeshes[b+1].vertices[v2 + 1], addVec3(g_BumperMeshes[b + 1].vertices[v2], temp));
				setVec3(g_BumperMeshes[b+1].normals[v2], norm);
				setVec3(g_BumperMeshes[b+1].normals[v2 + 1], norm);

				//left bumper - left face
				setVec3(g_BumperMeshes[b + 2].vertices[v2], g_BumperMeshes[b+1].vertices[v2 + 1]);
				setVec3(g_BumperMeshes[b + 2].vertices[v2 + 1], addVec3(g_SplineMeshes[j].vertices[v + 4], temp));
				setVec3(g_BumperMeshes[b + 2].normals[v2], binorm0);
				setVec3(g_BumperMeshes[b + 2].normals[v2 + 1], binorm0);

				//right bumper - left face
				setVec3(g_BumperMeshes[b+3].vertices[v2], g_SplineMeshes[j].vertices[v + 1]);
				setVec3(temp, mulVec3(norm, bumperHeight));
				setVec3(g_BumperMeshes[b+3].vertices[v2 + 1], addVec3(g_BumperMeshes[b+3].vertices[v2], temp));
				setVec3(g_BumperMeshes[b + 3].normals[v2], binorm0);
				setVec3(g_BumperMeshes[b + 3].normals[v2 + 1], binorm0);

				//right bumper - top face
				setVec3(g_BumperMeshes[b + 4].vertices[v2], g_BumperMeshes[b+3].vertices[v2 + 1]);
				setVec3(temp, mulVec3(binorm0, bumperWidth));
				setVec3(g_BumperMeshes[b + 4].vertices[v2 + 1], addVec3(g_BumperMeshes[b + 4].vertices[v2], temp));
				setVec3(g_BumperMeshes[b + 4].normals[v2], norm);
				setVec3(g_BumperMeshes[b + 4].normals[v2 + 1], norm);

				//right bumper - right face
				setVec3(g_BumperMeshes[b + 5].vertices[v2], g_BumperMeshes[b + 4].vertices[v2 + 1]);
				setVec3(g_BumperMeshes[b + 5].vertices[v2 + 1], addVec3(g_SplineMeshes[j].vertices[v], temp));
				setVec3(g_BumperMeshes[b + 5].normals[v2], mulVec3(binorm0, -1));
				setVec3(g_BumperMeshes[b + 5].normals[v2 + 1], mulVec3(binorm0, -1));

				for (int c = 0; c < 6; c++) {
					g_BumperMeshes[b + c].texCoords[v2] = new double[2]{ -t*3.0, 0.0 };
					g_BumperMeshes[b + c].texCoords[v2+1] = new double[2]{ -t*3.0, 1.0 };
				}
				#pragma endregion

				t += deltaT;
				i += 14;
				v += 6;
				v2 += 2;
			}
		}
	}
#pragma endregion

	/*props data*/
	g_PropMeshes = (mesh*)malloc(sizeof(mesh) * numPropMeshes);
	g_Props = (object*)malloc(sizeof(object) * numProps);

	//box
	g_PropMeshes[0].numVerts = 24;
	g_PropMeshes[0].vertices = (double**)malloc(sizeof(double*) * g_PropMeshes[0].numVerts);
	g_PropMeshes[0].normals = (double**)malloc(sizeof(double*) * g_PropMeshes[0].numVerts);
	g_PropMeshes[0].texCoords = (double**)malloc(sizeof(double*) * g_PropMeshes[0].numVerts);
	g_PropMeshes[0].numIndices = -1;
	g_PropMeshes[0].indices = nullptr;
	for (int i = 0; i < 24; i++) {
		g_PropMeshes[0].vertices[i] = new double[3]{ skyboxVerts[(i * 3)],  skyboxVerts[(i * 3) + 1], skyboxVerts[(i * 3) + 2] };

		if (i < 4) g_PropMeshes[0].normals[i] = new double[3]{ 0,0,-1 };
		else if (i < 8) g_PropMeshes[0].normals[i] = new double[3]{ -1,0,0 };
		else if (i < 12) g_PropMeshes[0].normals[i] = new double[3]{ -1,0,0 };
		else if (i < 16) g_PropMeshes[0].normals[i] = new double[3]{ 0,0,1 };
		else if (i < 20) g_PropMeshes[0].normals[i] = new double[3]{ 0,1,0 };
		else g_PropMeshes[0].normals[i] = new double[3]{ 0,-1,0 };

		if (i % 4 == 0) g_PropMeshes[0].texCoords[i] = new double[2]{ 1,1 };
		else if (i % 4 == 1) g_PropMeshes[0].texCoords[i] = new double[2]{ 1,0 };
		else if (i % 4 == 2) g_PropMeshes[0].texCoords[i] = new double[2]{ 0,0 };
		else if (i % 4 == 3) g_PropMeshes[0].texCoords[i] = new double[2]{ 0,1 };
	}
	
	//plane
	g_PropMeshes[1].numVerts = 4;
	g_PropMeshes[1].vertices = (double**)malloc(sizeof(double*) * g_PropMeshes[1].numVerts);
	g_PropMeshes[1].normals = (double**)malloc(sizeof(double*) * g_PropMeshes[1].numVerts);
	g_PropMeshes[1].texCoords = (double**)malloc(sizeof(double*) * g_PropMeshes[1].numVerts);
	g_PropMeshes[1].numIndices = -1;
	g_PropMeshes[1].indices = nullptr;
	
	g_PropMeshes[1].vertices[1] = new double[3]{ 0, 1, 1 };
	g_PropMeshes[1].vertices[0] = new double[3]{ 0, -1, 1 };
	g_PropMeshes[1].vertices[3] = new double[3]{ 0, -1, -1 };
	g_PropMeshes[1].vertices[2] = new double[3]{ 0, 1, -1 };
	
	for (int i = 0; i < 4; i++) {
		g_PropMeshes[1].normals[i] = new double[3]{-1, 0, 0 };
		g_PropMeshes[1].texCoords[i] = new double[2]{ g_PropMeshes[0].texCoords[i][0], g_PropMeshes[0].texCoords[i][1] };
	}

	//building 1
	g_Props[0].mesh = &g_PropMeshes[0];
	g_Props[0].repeatTextureS = 4;
	g_Props[0].repeatTextureT = 4;
	g_Props[0].texture = "building";
	setVec3(g_Props[0].transform.position, 6.75, -0.5, 2.0);
	setVec3(g_Props[0].transform.scale, 3.0, 12.0, 3.0);
	setVec3(g_Props[0].transform.rotation, vector0);

	//building 2
	g_Props[1].mesh = &g_PropMeshes[0];
	g_Props[1].repeatTextureS = 2;
	g_Props[1].repeatTextureT = 4;
	g_Props[1].texture = "building2";
	setVec3(g_Props[1].transform.position, 3.5, 0.0, -1.0);
	setVec3(g_Props[1].transform.scale, 3.0, 12.0, 3.0);
	setVec3(g_Props[1].transform.rotation, vector0);

	//building 3
	g_Props[2].mesh = &g_PropMeshes[0];
	g_Props[2].repeatTextureS = 2;
	g_Props[2].repeatTextureT = 4;
	g_Props[2].texture = "building3";
	setVec3(g_Props[2].transform.position, 0.0, 0.0, 1.5);
	setVec3(g_Props[2].transform.scale, 3.0, 12.0, 3.0);
	setVec3(g_Props[2].transform.rotation, vector0);
	
	//soap sign
	g_Props[3].mesh = &g_PropMeshes[1];
	g_Props[3].repeatTextureS = 1;
	g_Props[3].repeatTextureT = 1;
	g_Props[3].texture = "soap";
	setVec3(g_Props[3].transform.position, 5, 3, 1.7);
	setVec3(g_Props[3].transform.scale, 4, 4, 4);
	setVec3(g_Props[3].transform.rotation, vector0);

	//danger sign
	g_Props[4].mesh = &g_PropMeshes[1];
	g_Props[4].repeatTextureS = 1;
	g_Props[4].repeatTextureT = 1;
	g_Props[4].texture = "danger";
	setVec3(g_Props[4].transform.position, -6, 0, 11.0);
	setVec3(g_Props[4].transform.scale, 1.0, 1.0, 1.0);
	setVec3(g_Props[4].transform.rotation, 0, 180, -45);

	//road closed sign
	g_Props[5].mesh = &g_PropMeshes[1];
	g_Props[5].repeatTextureS = 1;
	g_Props[5].repeatTextureT = 1;
	g_Props[5].texture = "roadClosed";
	setVec3(g_Props[5].transform.position, 6, 1.0, 3.5);
	setVec3(g_Props[5].transform.scale, 2.0, 2.0, 2.0);
	setVec3(g_Props[5].transform.rotation, vector0);
}

/* draw catmull rom spline */
void drawCRSpline(spline CR, double step = 0.001) {
	// loop through groups of 4 control points
	for (int i = 0; i < CR.numControlPoints - 3; i++) {
		double** basisControlMatrix = CR.CRBasisControlMatrices[i];

		if (i == debugPoint) glColor3f(0.0, 1.0, 0.0);

		// interpolate u between [0, 1]
		for (double u = 0; u <= 1.0; u += step) {
			double* finalPoint = getSplinePoint(u, basisControlMatrix);
			glVertex3d(finalPoint[0], finalPoint[1], finalPoint[2]);
		}

		if (i == debugPoint) glColor3f(1.0, 0.0, 0.0);
	}
}

void drawObject(object o) {
	glTranslatef(o.transform.position[0], o.transform.position[1], o.transform.position[2]);
	glScalef(o.transform.scale[0], o.transform.scale[1], o.transform.scale[2]);
	if (o.mesh->numIndices == -1) {
		for (int i = 0; i < o.mesh->numVerts; i++) {
			glTexCoord2f(o.mesh->texCoords[i][0] * o.repeatTextureS, o.mesh->texCoords[i][1] * o.repeatTextureT);
			glNormal3f(o.mesh->normals[i][0], o.mesh->normals[i][1], o.mesh->normals[i][2]);
			glVertex3d(o.mesh->vertices[i][0], o.mesh->vertices[i][1], o.mesh->vertices[i][2]);
		}
	}
	else {
		for (int i = 0; i < o.mesh->numIndices; i++) {
			int idx = o.mesh->indices[i];
			glTexCoord2f(o.mesh->texCoords[i][0] * o.repeatTextureS, o.mesh->texCoords[i][1] * o.repeatTextureT);
			glNormal3f(o.mesh->normals[idx][0], o.mesh->normals[idx][1], o.mesh->normals[idx][2]);
			glVertex3d(o.mesh->vertices[idx][0], o.mesh->vertices[idx][1], o.mesh->vertices[idx][2]);
		}
	}
}

struct light {
	double ambient[4];
	double diffuse[4];
	double specular[4];
	double position[4];
};
int numLights = 1;
struct light g_lights[8];

void display()
{
	glMatrixMode(GL_MODELVIEW);

	// clear color and depth buffers
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set world transform
	glLoadIdentity();

	glPushMatrix();
	
	if (g_CameraState == CAMERASTATE::DEBUG) {
		glTranslatef(g_vLandTranslate[0] + translateOffset[0], g_vLandTranslate[1] + translateOffset[1], g_vLandTranslate[2] + translateOffset[2]);
		glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
		glRotatef(deltaRotate, g_vLandRotate[0], g_vLandRotate[1], g_vLandRotate[2]);
	}
	else {
		gluLookAt(
			camPos[0], camPos[1], camPos[2], 
			camPos[0]+camForward[0], camPos[1]+camForward[1], camPos[2]+camForward[2], 
			camUp[0], camUp[1], camUp[2]);
	}

	glEnable(GL_LIGHTING);
	
	light testLight;
	GLfloat a[4] = { 2, 1.5, 2, 1.0 };
	GLfloat d[4] = { 1.5, 1.0, 0.0, 1.0 };
	GLfloat s[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat p[4] = { 1.0, 3.0, -1.0, 0.0 };
	glEnable(GL_LIGHT0);
	
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, a);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, d);
	glLightfv(GL_LIGHT0, GL_SPECULAR, s);
	glLightfv(GL_LIGHT0, GL_POSITION, p);

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	

	// turn on texture mapping (this disables standard OpenGL lighting, unless in GL_MODULATE mode)
	glEnable(GL_TEXTURE_2D);
	
	//skybox
	glPushMatrix();
	glScalef(60.0, 60.0, 60.0);
	glDepthMask(GL_FALSE);
	for (int i = 0, f = 0; i < 72; i += 12, f++) {
		int m = 1;
		if (skyboxFaces[f] == "nightSky") m = 8;
		glBindTexture(GL_TEXTURE_2D, textures[skyboxFaces[f]]);
		glBegin(GL_QUADS);
		glTexCoord2f(1.0*m, 1.0*m); glVertex3f(skyboxVerts[i], skyboxVerts[i+1], skyboxVerts[i+2]);
		glTexCoord2f(1.0*m, 0.0); glVertex3f(skyboxVerts[i+3], skyboxVerts[i+4], skyboxVerts[i+5]);
		glTexCoord2f(0.0, 0.0); glVertex3f(skyboxVerts[i+6], skyboxVerts[i+7], skyboxVerts[i+8]);
		glTexCoord2f(0.0, 1.0*m); glVertex3f(skyboxVerts[i+9], skyboxVerts[i+10], skyboxVerts[i+11]);
		glEnd();
	}
	glDepthMask(GL_TRUE);
	glPopMatrix();

	// draw road
	glBindTexture(GL_TEXTURE_2D, textures[g_SplineObjects[0].texture]);
	glBegin(GL_QUAD_STRIP);
	drawObject(g_SplineObjects[0]);
	glEnd();

	// draw bumpers
	for (int i = 0; i < 6; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[g_BumperObjects[i].texture]);
		glBegin(GL_QUAD_STRIP);
		drawObject(g_BumperObjects[i]);
		glEnd();
	}

	// draw props
	for (int i = 0; i < numProps; i++) {
		glPushMatrix();
		glScalef(g_Props[i].transform.scale[0], g_Props[i].transform.scale[1], g_Props[i].transform.scale[2]);
		glTranslatef(g_Props[i].transform.position[0], g_Props[i].transform.position[1], g_Props[i].transform.position[2]);
		glRotatef(g_Props[i].transform.rotation[0], 1., 0, 0);
		glRotatef(g_Props[i].transform.rotation[1], 0, 1., 0);
		glRotatef(g_Props[i].transform.rotation[2], 0, 0, 1.);
		glBindTexture(GL_TEXTURE_2D, textures[g_Props[i].texture]);
		glBegin(GL_QUADS);
		drawObject(g_Props[i]);
		glEnd();
		glPopMatrix();
	}
	
	// turn off texture mapping 
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
	glutSwapBuffers();
}

void doIdle()
{
	/* make the screen update */
	glutPostRedisplay();

	/* 60fps update */
	if (glutGet(GLUT_ELAPSED_TIME) - elapsedTime > 17 && g_CameraState!=CAMERASTATE::DEBUG) {
		if (currPoint == 0) {
			currSpline = (currSpline + 1) % g_iNumOfSplines;
			initCamVectors(g_Splines[currSpline]);
		}

		// move camera along spline
		setVec3(camPos, getSplinePoint(posU, g_Splines[currSpline].CRBasisControlMatrices[currPoint]));
		double addPos[3];
		setVec3(addPos, mulVec3(camUp, camPosUpOffset));
		setVec3(camPos, addVec3(camPos, addPos));

		// rotate camera along tangent
		setVec3(camForward, normalizeVec3(getSplineTangent(posU, g_Splines[currSpline].CRBasisControlMatrices[currPoint])));
		setVec3(camUp, crossVec3(prevBinormal, camForward));
		setVec3(camUp, normalizeVec3(camUp));
		setVec3(prevBinormal, crossVec3(camForward, camUp));
		setVec3(prevBinormal, normalizeVec3(prevBinormal));

		//printf("camPos: %f %f %f\n", camPos[0], camPos[1], camPos[2]);
			
		// update indices
		posU += deltaPosU;
		if (posU > 1.) {
			posU -= 1.;
			currPoint = currPoint + 1 == g_Splines[currSpline].numControlPoints - 3 ? 0 : currPoint + 1;
			printf("\ncurr point: %i\n", currPoint);
		}

		// update time
		elapsedTime = glutGet(GLUT_ELAPSED_TIME);
	}
}

void keyboard(unsigned char key, int x, int y) {
	if (key == '1') {
		if (g_CameraState == CAMERASTATE::DEBUG) {
			g_CameraState = CAMERASTATE::NORMAL;
		}
		else if (g_CameraState == CAMERASTATE::NORMAL) {
			g_CameraState = CAMERASTATE::DEBUG;
			debugPoint = currPoint;
		}
	}
}

/* Write a screenshot to the specified filename */
void saveScreenshot(char *filename)
{
	if (filename == NULL)
		return;

	// Allocate a picture buffer // 
	cv::Mat3b bufferRGB = cv::Mat::zeros(480, 640, CV_8UC3); //rows, cols, 3-channel 8-bit.
	printf("File to save to: %s\n", filename);

	//use fast 4-byte alignment (default anyway) if possible
	glPixelStorei(GL_PACK_ALIGNMENT, (bufferRGB.step & 3) ? 1 : 4);
	//set length of one complete row in destination data (doesn't need to equal img.cols)
	glPixelStorei(GL_PACK_ROW_LENGTH, bufferRGB.step / bufferRGB.elemSize());
	glReadPixels(0, 0, bufferRGB.cols, bufferRGB.rows, GL_RGB, GL_UNSIGNED_BYTE, bufferRGB.data);
	//flip to account for GL 0,0 at lower left
	cv::flip(bufferRGB, bufferRGB, 0);
	//convert RGB to BGR
	cv::Mat3b bufferBGR(bufferRGB.rows, bufferRGB.cols, CV_8UC3);
	cv::Mat3b out[] = { bufferBGR };
	// rgb[0] -> bgr[2], rgba[1] -> bgr[1], rgb[2] -> bgr[0]
	int from_to[] = { 0,2, 1,1, 2,0 };
	mixChannels(&bufferRGB, 1, out, 1, from_to, 3);

	if (cv::imwrite(filename, bufferBGR)) {
		printf("File saved Successfully\n");
	}
	else {
		printf("Error in Saving\n");
	}
}

/* Function to get a pixel value. Use like PIC_PIXEL macro. 
Note: OpenCV images are in channel order BGR. 
This means that:
chan = 0 returns BLUE, 
chan = 1 returns GREEN, 
chan = 2 returns RED. */
unsigned char getPixelValue(cv::Mat3b& image, int x, int y, int chan)
{
	return image.at<cv::Vec3b>(y, x)[chan];
}

void normalizePixelValue(cv::Mat3b& image, int x, int y, int chan) {
	//image.at<cv::Vec3b>(y, x)[chan] = image.at<cv::Vec3b>(y, x)[chan] / 255;
	image.data;
}

/* Function that does nothing but demonstrates looping through image coordinates.*/
void loopImage(cv::Mat3b& image)
{
	for (int r = 0; r < image.rows; r++) { // y-coordinate
		for (int c = 0; c < image.cols; c++) { // x-coordinate
			for (int channel = 0; channel < 3; channel++) {
				// DO SOMETHING... example usage
				 unsigned char blue = getPixelValue(image, c, r, 0);
				 unsigned char green = getPixelValue(image, c, r, 1); 
				 unsigned char red = getPixelValue(image, c, r, 2); 
				 printf("col%i row%i: %i %i %i\n", c, r, red, green, blue);
			}
		}
	}
}

/* Read an image into memory. 
Set argument displayOn to true to make sure images are loaded correctly.
One image loaded, set to false so it doesn't interfere with OpenGL window.*/
int readImage(char *filename, cv::Mat3b& image, bool displayOn)
{
	std::cout << "reading image: " << filename << std::endl;
	image = cv::imread(filename);
	if (!image.data) // Check for invalid input                    
	{
		std::cout << "Could not open or find the image." << std::endl;
		return 1;
	}

	if (displayOn)
	{
		cv::imshow("TestWindow", image);
		cv::waitKey(0); // Press any key to enter. 
	}
	return 0;
}

void initTexture(char* filename, std::string storeName) {
	int width, height, numChannels;
	unsigned char* data = stbi_load(filename, &width, &height, &numChannels, 0);

	GLuint* texture = new GLuint;

	// create placeholder for texture
	glGenTextures(1, &(*texture));
	glBindTexture(GL_TEXTURE_2D, *texture); // make texture

	// specify texture parameters (they affect whatever texture is active)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// use linear filter both for magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);

	textures[storeName] = *texture;
}

void initTextureRGBA(char* filename, std::string storeName) {
	int width, height, numChannels;
	unsigned char* data = stbi_load(filename, &width, &height, &numChannels, 0);

	GLuint* texture = new GLuint;

	// create placeholder for texture
	glGenTextures(1, &(*texture));
	glBindTexture(GL_TEXTURE_2D, *texture); // make texture

	// specify texture parameters (they affect whatever texture is active)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// use linear filter both for magnification and minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);

	textures[storeName] = *texture;
}

void initCubeMap(std::string faces[6], std::string storeName) {
	// create cubemap
	int width, height, nrChannels;
	unsigned char* data;

	GLuint* texture = new GLuint;

	glGenTextures(1, &(*texture));
	glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);
	
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; i++)
	{
		data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
		);
		stbi_image_free(data);
	}
	textures[storeName] = *texture;
}

/* OpenCV help:
Access number of rows of image (height): image.rows; 
Access number of columns of image (width): image.cols;
Pixel 0,0 is the upper left corner. Byte order for 3-channel images is BGR. 
*/

int _tmain(int argc, _TCHAR* argv[])
{
	// I've set the argv[1] to track.txt.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your track file name for the "Command Arguments"
	if (argc<2)
	{  
		printf ("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}
	
	loadSplines(argv[1]);

	glutInit(&argc, (char**)argv);

	/* create window */
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(initWindowWidth, initWindowHeight);
	glutInitWindowPosition(initWindowPos[0], initWindowPos[1]);
	glutCreateWindow(argv[0]);

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);

	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/* adjust screen to rescaled window */
	glutReshapeFunc(reshape);

	/* replace with any animate code */
	glutIdleFunc(doIdle);

	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);
	/* callback for keyboard input */
	glutKeyboardFunc(keyboard);

	/* setup gl view here */
	glShadeModel(GL_SMOOTH);

	// Depth Buffer Setup
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	//textures
	initTexture("bumper.png", "bumper");
	initTexture("bumperArrow.png", "bumperArrow");
	initTexture("bumperTop.png", "bumperTop");
	initTexture("cityBG.png", "cityBG");
	initTextureRGBA("nightSky.png", "nightSky");
	initTexture("roadArrow.png", "roadArrow");
	initTexture("roadBump.png", "roadBump");
	initTexture("roadSide.png", "roadSide");
	initTexture("building.png", "building");
	initTexture("building2.png", "building2");
	initTexture("building3.png", "building3");
	initTexture("soap.png", "soap");
	initTexture("danger.png", "danger");
	initTexture("roadClosed.png", "roadClosed");
	initTexture("roadClosedLight.png", "roadClosedLight");
	initTextureRGBA("bottom.png", "bottom");

	GLfloat globalAmbient[] = { 1.5, 1.5, 1.5, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	myInit();
	glutMainLoop();

	return 0;
}
