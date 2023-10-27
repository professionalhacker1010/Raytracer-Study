#pragma once

#include "Math.h"
#include <string>
#include <smmintrin.h>
#include "Constants.h"

static class Util {

public:

	static double calcTriArea2D(const float p1[2], const float p2[2], const float p3[2]) {
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

	static bool withinBounds(const Vec3& pos, const Vec3& max, const Vec3& min) {
		return pos[0] <= max[0] && pos[1] <= max[1] && pos[2] <= max[2]
			&& pos[0] >= min[0] && pos[1] >= min[1] && pos[2] >= min[2];
	}

	static bool doubleCompare(const double in, const double expected, double epsilon = 0.00001) {
		return abs(expected - in) < epsilon;
	}

	static bool FloatCompare(const float in, const float expected, float epsilon = 0.00001f) {
		return abs(expected - in) < epsilon;
	}

	static void Print(std::string s = "") {
		std::string p = s + "\n";
		printf(p.c_str());
	}

	//static void parse_check(char* expected, char* found)
	//{
	//	if (stricmp(expected, found))
	//	{
	//		printf("Expected '%s ' found '%s '\n", expected, found);
	//		printf("Parse error, abnormal abortion\n");
	//		exit(0);
	//	}
	//}

	//static void parse_doubles(FILE* file, char* check, Vec3& p)
	//{
	//	char str[100];
	//	fscanf(file, "%s", str);
	//	parse_check(check, str);
	//	p.ParseFromFile(file);
	//	//printf("%s %lf %lf %lf\n", check, p[0], p[1], p[2]);
	//}

	//static void parse_doubles(FILE* file, char* check, double p[3])
	//{
	//	char str[100];
	//	fscanf(file, "%s", str);
	//	parse_check(check, str);
	//	fscanf(file, "%lf %lf %lf", &p[0], &p[1], &p[2]);
	//	//printf("%s %lf %lf %lf\n", check, p[0], p[1], p[2]);
	//}

	//static void parse_rad(FILE* file, double* r)
	//{
	//	char str[100];
	//	fscanf(file, "%s", str);
	//	parse_check("rad:", str);
	//	fscanf(file, "%lf", r);
	//	//printf("rad: %f\n", *r);
	//}

	//static void parse_shi(FILE* file, double* shi)
	//{
	//	char s[100];
	//	fscanf(file, "%s", s);
	//	parse_check("shi:", s);
	//	fscanf(file, "%lf", shi);
	//	//printf("shi: %f\n", *shi);
	//}

//#pragma region plot pixels
//	static void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b);
//	static void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
//#pragma endregion
};

