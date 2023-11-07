#pragma once
#include <string>
#include "Constants.h"
#include <smmintrin.h>

struct Util {
	static double CalcTriArea2D(const float p1[2], const float p2[2], const float p3[2]) {
		__m128 A = _mm_set_ps1(p1[0]);
		__m128 B = _mm_set_ps(p1[1], -p3[1], p2[1], -p1[1]);
		__m128 C = _mm_mul_ps(A, B);
		A = _mm_set_ps(-p2[0], -p2[0], -p3[0], -p3[0]);
		B = _mm_mul_ps(A, B);
		C = _mm_add_ps(B, C);
		float dot = _mm_dp_ps(_mm_set_ps1(0.5f), C, 0xF1).m128_f32[0];
		return (abs(dot));
		//return abs(0.5 * ((p2[0] - p1[0]) * (p3[1] - p1[1]) - (p3[0] - p1[0]) * (p2[1] - p1[1])));
	}

	static bool FloatCompare(const float in, const float expected, float epsilon = 0.00001f) {
		return abs(expected - in) < epsilon;
	}

	static void Print(std::string s = "") {
		std::string p = s + "\n";
		printf(p.c_str());
	}
};

// aligned memory allocations
#ifdef _MSC_VER
#define ALIGN( x ) __declspec( align( x ) )
#define MALLOC64( x ) ( ( x ) == 0 ? 0 : _aligned_malloc( ( x ), 64 ) )
#define FREE64( x ) _aligned_free( x )
#else
#define ALIGN( x ) __attribute__( ( aligned( x ) ) )
#define MALLOC64( x ) ( ( x ) == 0 ? 0 : aligned_alloc( 64, ( x ) ) )
#define FREE64( x ) free( x )
#endif
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define CHECK_RESULT __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define CHECK_RESULT _Check_return_
#else
#define CHECK_RESULT
#endif

//GLuint CreateVBO(const GLfloat* data, const uint size);
//void BindVBO(const uint idx, const uint N, const GLuint id);

// pixel operations
inline uint ScaleColor(const uint c, const uint scale)
{
	const uint rb = (((c & 0xff00ff) * scale) >> 8) & 0x00ff00ff;
	const uint ag = (((c & 0xff00ff00) >> 8) * scale) & 0xff00ff00;
	return rb + ag;
}
inline uint AddBlend(const uint c1, const uint c2)
{
	const uint r1 = (c1 >> 16) & 255, r2 = (c2 >> 16) & 255;
	const uint g1 = (c1 >> 8) & 255, g2 = (c2 >> 8) & 255;
	const uint b1 = c1 & 255, b2 = c2 & 255;
	//const uint r = fminf(255u, r1 + r2);
	//const uint g = fminf(255u, g1 + g2);
	//const uint b = fminf(255u, b1 + b2);
	//return (r << 16) + (g << 8) + b;
	return 0;
}

// Perlin noise
float Noise2D(const float x, const float y);

std::string TextFileRead(const char* _File);

