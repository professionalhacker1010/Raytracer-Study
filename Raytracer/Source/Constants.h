#pragma once
#define PI 3.14159265358979323
#define INV2PI 0.15915494309
#define INVPI 0.31830988618
#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10
#define MAX_BVH_STACK 50

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0

#define _MM_SHUFFLER( xi, yi, zi, wi ) _MM_SHUFFLE( wi, zi, yi, xi )
#define ALIGNSIZE 64


// basic types
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;