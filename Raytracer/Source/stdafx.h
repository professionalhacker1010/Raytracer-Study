// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdlib.h>
#include <atomic>
#include <map>
//#define STB_IMAGE_IMPLEMENTATION
//#define STBI_NO_PSD
//#define STBI_NO_PIC
//#define STBI_NO_PNM

// My header files
#include "Math.h"
#include "Util.h"

// Written not by me
#include "Surface.h"
#include "GLTexture.h"
#include "Shader.h"
#include "CLBuffer.h"

// Template, IGAD version 2
// IGAD/NHTV/UU - Jacco Bikker - 2006-2021

// add your includes to this file instead of to individual .cpp files
// to enjoy the benefits of precompiled headers:
// - fast compilation
// - solve issues with the order of header files once (here)
// do not include headers in header files (ever).

// C++ headers
#include <stdio.h>
#include <string>
#include <ctime>
#include <chrono>
#include <fstream>
#include <vector>
#include <list>
#include <thread>
#include <math.h>
#include <algorithm>
#include <assert.h>
#include <io.h>

#include "External/stb_image.h"

// header for AVX, and every technology before it.
// if your CPU does not support this (unlikely), include the appropriate header instead.
// see: https://stackoverflow.com/a/11228864/2844473
#include <immintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

// clang-format off

// "leak" common namespaces to all compilation units. This is not standard
// C++ practice but a simplification for template projects.
using namespace std;

// windows.h: disable as much as possible to speed up compilation.
#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#endif
#define NOGDICAPMASKS
// #define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOIME
#include "windows.h"

// OpenCL headers
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS // safe; see https://stackoverflow.com/a/28500846
#include "OpenCL/inc/CL/cl.h"
#include "OpenCL/inc/CL/cl_gl.h"

// GLFW
#define GLFW_USE_CHDIR 0
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include "External/glad.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

//#include <gl/glew.h>
//#include <gl/glu.h>
//#include <gl/glut.h>

// zlib
#include "zlib/zlib.h"


//#ifdef _MSC_VER
//typedef unsigned char BYTE;		// for freeimage.h
//typedef unsigned short WORD;	// for freeimage.h
//typedef unsigned long DWORD;	// for freeimage.h
//typedef int BOOL;				// for freeimage.h
//#endif

// vector type placeholders, carefully matching OpenCL's layout and alignment
//struct ALIGN(8) int2
//{
//	int2() = default;
//	int2(const int a, const int b) : x(a), y(b) {}
//	int2(const int a) : x(a), y(a) {}
//	union { struct { int x, y; }; int cell[2]; };
//	int& operator [] (const int n) { return cell[n]; }
//};
//struct ALIGN(8) uint2
//{
//	uint2() = default;
//	uint2(const int a, const int b) : x(a), y(b) {}
//	uint2(const uint a) : x(a), y(a) {}
//	union { struct { uint x, y; }; uint cell[2]; };
//	uint& operator [] (const int n) { return cell[n]; }
//};
//
//struct int3;
//struct ALIGN(16) int4
//{
//	int4() = default;
//	int4(const int a, const int b, const int c, const int d) : x(a), y(b), z(c), w(d) {}
//	int4(const int a) : x(a), y(a), z(a), w(a) {}
//	int4(const int3 & a, const int d);
//	union { struct { int x, y, z, w; }; int cell[4]; };
//	int& operator [] (const int n) { return cell[n]; }
//};
//struct ALIGN(16) int3
//{
//	int3() = default;
//	int3(const int a, const int b, const int c) : x(a), y(b), z(c) {}
//	int3(const int a) : x(a), y(a), z(a) {}
//	int3(const int4 a) : x(a.x), y(a.y), z(a.z) {}
//	union { struct { int x, y, z; int dummy; }; int cell[4]; };
//	int& operator [] (const int n) { return cell[n]; }
//};
//struct uint3;
//struct ALIGN(16) uint4
//{
//	uint4() = default;
//	uint4(const uint a, const uint b, const uint c, const uint d) : x(a), y(b), z(c), w(d) {}
//	uint4(const uint a) : x(a), y(a), z(a), w(a) {}
//	uint4(const uint3 & a, const uint d);
//	union { struct { uint x, y, z, w; }; uint cell[4]; };
//	uint& operator [] (const int n) { return cell[n]; }
//};
//struct ALIGN(16) uint3
//{
//	uint3() = default;
//	uint3(const uint a, const uint b, const uint c) : x(a), y(b), z(c) {}
//	uint3(const uint a) : x(a), y(a), z(a) {}
//	uint3(const uint4 a) : x(a.x), y(a.y), z(a.z) {}
//	union { struct { uint x, y, z; uint dummy; }; uint cell[4]; };
//	uint& operator [] (const int n) { return cell[n]; }
//};
//
//struct ALIGN(4) uchar4
//{
//	uchar4() = default;
//	uchar4(const uchar a, const uchar b, const uchar c, const uchar d) : x(a), y(b), z(c), w(d) {}
//	uchar4(const uchar a) : x(a), y(a), z(a), w(a) {}
//	union { struct { uchar x, y, z, w; }; uchar cell[4]; };
//	uchar& operator [] (const int n) { return cell[n]; }
//};

// timer
//struct Timer
//{
//	Timer() { reset(); }
//	float elapsed() const
//	{
//		chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
//		chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - start);
//		return (float)time_span.count();
//	}
//	void reset() { start = chrono::high_resolution_clock::now(); }
//	chrono::high_resolution_clock::time_point start;
//};

// generic error checking for OpenGL code
#define CheckGL() { _CheckGL( __FILE__, __LINE__ ); }

//inline void _CheckGL(const char* f, int l);

// fatal error reporting (with a pretty window)
#define FATALERROR( fmt, ... ) FatalError( "Error on line %d of %s: " fmt "\n", __LINE__, __FILE__, ##__VA_ARGS__ )
#define FATALERROR_IF( condition, fmt, ... ) do { if ( ( condition ) ) FATALERROR( fmt, ##__VA_ARGS__ ); } while ( 0 )
#define FATALERROR_IN( prefix, errstr, fmt, ... ) FatalError( prefix " returned error '%s' at %s:%d" fmt "\n", errstr, __FILE__, __LINE__, ##__VA_ARGS__ );
#define FATALERROR_IN_CALL( stmt, error_parser, fmt, ... ) do { auto ret = ( stmt ); if ( ret ) FATALERROR_IN( #stmt, error_parser( ret ), fmt, ##__VA_ARGS__ ) } while ( 0 )


//inline void FatalError(const char* fmt, ...);

inline void FatalError(const char* fmt, ...)
{
	char t[16384];
	va_list args;
	va_start(args, fmt);
	vsnprintf(t, sizeof(t), fmt, args);
	va_end(args);
#ifdef _MSC_VER
	MessageBox(NULL, t, "Fatal error", MB_OK);
#else
	fprintf(stderr, t);
#endif
	while (1) exit(0);
}

// OpenGL helper functions
inline void _CheckGL(const char* f, int l)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		const char* errStr = "UNKNOWN ERROR";
		if (error == 0x500) errStr = "INVALID ENUM";
		else if (error == 0x502) errStr = "INVALID OPERATION";
		else if (error == 0x501) errStr = "INVALID VALUE";
		else if (error == 0x506) errStr = "INVALID FRAMEBUFFER OPERATION";
		FatalError("GL error %d: %s at %s:%d\n", error, errStr, f, l);
	}
}


// Nils's jobmanager
class Job
{
public:
	virtual void Main() = 0;
protected:
	friend class JobThread;
	void RunCodeWrapper();
};
class JobThread
{
public:
	void CreateAndStartThread(unsigned int threadId);
	void Go();
	void BackgroundTask();
	HANDLE m_GoSignal, m_ThreadHandle;
	int m_ThreadID;
};
class JobManager	// singleton class!
{
protected:
	JobManager(unsigned int numThreads);
public:
	~JobManager();
	static void CreateJobManager(unsigned int numThreads);
	static JobManager* GetJobManager();
	static void GetProcessorCount(uint& cores, uint& logical);
	void AddJob2(Job* a_Job);
	unsigned int GetNumThreads() { return m_NumThreads; }
	void RunJobs();
	void ThreadDone(unsigned int n);
	int MaxConcurrent() { return m_NumThreads; }
protected:
	friend class JobThread;
	Job* GetNextJob();
	static JobManager* m_JobManager;
	Job* m_JobList[256];
	CRITICAL_SECTION m_CS;
	HANDLE m_ThreadDone[64];
	unsigned int m_NumThreads, m_JobCount;
	JobThread* m_JobThreadList;
};






// global project settigs; shared with OpenCL
#include "Constants.h"

// Add your headers here; they will be able to use all previously defined classes and namespaces.
// In your own .cpp files just add #include "precomp.h".
// #include "my_include.h"



// InstructionSet.cpp
// Compile by using: cl /EHsc /W4 InstructionSet.cpp
// processor: x86, x64
// Uses the __cpuid intrinsic to get information about
// CPU extended instruction set support.

#include <iostream>
#include <bitset>
#include <array>
#include <intrin.h>

// instruction set detection
#ifdef _WIN32
#define cpuid(info, x) __cpuidex(info, x, 0)
#else
#include <cpuid.h>
void cpuid(int info[4], int InfoType) { __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]); }
#endif
class CPUCaps // from https://github.com/Mysticial/FeatureDetector
{
public:
	static inline bool HW_MMX = false;
	static inline bool HW_x64 = false;
	static inline bool HW_ABM = false;
	static inline bool HW_RDRAND = false;
	static inline bool HW_BMI1 = false;
	static inline bool HW_BMI2 = false;
	static inline bool HW_ADX = false;
	static inline bool HW_PREFETCHWT1 = false;
	// SIMD: 128-bit
	static inline bool HW_SSE = false;
	static inline bool HW_SSE2 = false;
	static inline bool HW_SSE3 = false;
	static inline bool HW_SSSE3 = false;
	static inline bool HW_SSE41 = false;
	static inline bool HW_SSE42 = false;
	static inline bool HW_SSE4a = false;
	static inline bool HW_AES = false;
	static inline bool HW_SHA = false;
	// SIMD: 256-bit
	static inline bool HW_AVX = false;
	static inline bool HW_XOP = false;
	static inline bool HW_FMA3 = false;
	static inline bool HW_FMA4 = false;
	static inline bool HW_AVX2 = false;
	// SIMD: 512-bit
	static inline bool HW_AVX512F = false;    //  AVX512 Foundation
	static inline bool HW_AVX512CD = false;   //  AVX512 Conflict Detection
	static inline bool HW_AVX512PF = false;   //  AVX512 Prefetch
	static inline bool HW_AVX512ER = false;   //  AVX512 Exponential + Reciprocal
	static inline bool HW_AVX512VL = false;   //  AVX512 Vector Length Extensions
	static inline bool HW_AVX512BW = false;   //  AVX512 Byte + Word
	static inline bool HW_AVX512DQ = false;   //  AVX512 Doubleword + Quadword
	static inline bool HW_AVX512IFMA = false; //  AVX512 Integer 52-bit Fused Multiply-Add
	static inline bool HW_AVX512VBMI = false; //  AVX512 Vector Byte Manipulation Instructions
	// constructor
	CPUCaps()
	{
		int info[4];
		cpuid(info, 0);
		int nIds = info[0];
		cpuid(info, 0x80000000);
		unsigned nExIds = info[0];
		// detect Features
		if (nIds >= 0x00000001)
		{
			cpuid(info, 0x00000001);
			HW_MMX = (info[3] & ((int)1 << 23)) != 0;
			HW_SSE = (info[3] & ((int)1 << 25)) != 0;
			HW_SSE2 = (info[3] & ((int)1 << 26)) != 0;
			HW_SSE3 = (info[2] & ((int)1 << 0)) != 0;
			HW_SSSE3 = (info[2] & ((int)1 << 9)) != 0;
			HW_SSE41 = (info[2] & ((int)1 << 19)) != 0;
			HW_SSE42 = (info[2] & ((int)1 << 20)) != 0;
			HW_AES = (info[2] & ((int)1 << 25)) != 0;
			HW_AVX = (info[2] & ((int)1 << 28)) != 0;
			HW_FMA3 = (info[2] & ((int)1 << 12)) != 0;
			HW_RDRAND = (info[2] & ((int)1 << 30)) != 0;
		}
		if (nIds >= 0x00000007)
		{
			cpuid(info, 0x00000007);
			HW_AVX2 = (info[1] & ((int)1 << 5)) != 0;
			HW_BMI1 = (info[1] & ((int)1 << 3)) != 0;
			HW_BMI2 = (info[1] & ((int)1 << 8)) != 0;
			HW_ADX = (info[1] & ((int)1 << 19)) != 0;
			HW_SHA = (info[1] & ((int)1 << 29)) != 0;
			HW_PREFETCHWT1 = (info[2] & ((int)1 << 0)) != 0;
			HW_AVX512F = (info[1] & ((int)1 << 16)) != 0;
			HW_AVX512CD = (info[1] & ((int)1 << 28)) != 0;
			HW_AVX512PF = (info[1] & ((int)1 << 26)) != 0;
			HW_AVX512ER = (info[1] & ((int)1 << 27)) != 0;
			HW_AVX512VL = (info[1] & ((int)1 << 31)) != 0;
			HW_AVX512BW = (info[1] & ((int)1 << 30)) != 0;
			HW_AVX512DQ = (info[1] & ((int)1 << 17)) != 0;
			HW_AVX512IFMA = (info[1] & ((int)1 << 21)) != 0;
			HW_AVX512VBMI = (info[2] & ((int)1 << 1)) != 0;
		}
		if (nExIds >= 0x80000001)
		{
			cpuid(info, 0x80000001);
			HW_x64 = (info[3] & ((int)1 << 29)) != 0;
			HW_ABM = (info[2] & ((int)1 << 5)) != 0;
			HW_SSE4a = (info[2] & ((int)1 << 6)) != 0;
			HW_FMA4 = (info[2] & ((int)1 << 16)) != 0;
			HW_XOP = (info[2] & ((int)1 << 11)) != 0;
		}
	}
};

// application base class
//class TheApp
//{
//public:
//	virtual void Init() = 0;
//	virtual void Tick(float deltaTime) = 0;
//	virtual void Shutdown() = 0;
//	virtual void MouseUp(int button) = 0;
//	virtual void MouseDown(int button) = 0;
//	virtual void MouseMove(int x, int y) = 0;
//	virtual void MouseWheel(float y) = 0;
//	virtual void KeyUp(int key) = 0;
//	virtual void KeyDown(int key) = 0;
//	Surface* screen = 0;
//};

// EOF