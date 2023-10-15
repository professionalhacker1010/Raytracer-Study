#pragma once
#include <math.h>
#include <string>
#include <smmintrin.h>
#include "Constants.h"

struct Vec3 {
	explicit Vec3() { mC = _mm_setzero_ps(); }
	explicit Vec3(float x, float y, float z) { mC = _mm_setr_ps(x, y, z, 0.0f); }
	Vec3(const Vec3& v) { mC = v.mC; }
	explicit Vec3(float val) { mC = _mm_set_ps1(val); }
	explicit Vec3(__m128 v) { mC = v; }

	//operator overloads
	const Vec3& operator = (const Vec3& rhs) noexcept { 
		if (this == &rhs) return *this;
		mC = _mm_setr_ps(rhs[0], rhs[1], rhs[2], 0.0f);
		return *this;
	}

	Vec3 operator + (const Vec3& rhs) const { return Vec3(_mm_add_ps(mC, rhs.mC)); }
	Vec3 operator - (const Vec3& rhs) const {
		//__m128 temp = _mm_sub_ps(mC, rhs.mC); 
		return Vec3(c[0] - rhs[0], c[1] - rhs[1], c[2] - rhs[2]);
	}
	Vec3 operator * (const Vec3& rhs) const { return Vec3(_mm_mul_ps(mC, rhs.mC));; }
	Vec3 operator * (float rhs) const { return Vec3(_mm_mul_ps(mC, _mm_set_ps1(rhs))); }
	Vec3 operator / (const Vec3& rhs) const { return Vec3(_mm_div_ps(mC, rhs.mC)); }
	Vec3 operator / (float rhs) const { return Vec3(_mm_div_ps(mC, _mm_set_ps1(rhs))); }
	float operator [] (int i) const { return c[i]; }
	operator std::string() const { return " " + std::to_string(c[0]) + " " + std::to_string(c[1]) + " " + std::to_string(c[2]); }

	//static vector math operations
	static float Dot(const Vec3& a, const Vec3& b) {
		return _mm_dp_ps(a.mC, b.mC, 0x71).m128_f32[0];
	}

	static Vec3 Cross(const Vec3& a, const Vec3& b) {
		__m128 tempA = _mm_shuffle_ps(a.mC, a.mC, _MM_SHUFFLER(1, 2, 0, 3)); //y, z, x
		__m128 tempB = _mm_shuffle_ps(b.mC, b.mC, _MM_SHUFFLER(2, 0, 1, 3)); //z, x, y
		__m128 result = _mm_mul_ps(tempA, tempB);
		tempA = _mm_shuffle_ps(a.mC, a.mC, _MM_SHUFFLER(2, 0, 1, 3));
		tempB = _mm_shuffle_ps(b.mC, b.mC, _MM_SHUFFLER(1, 2, 0, 3));
		tempA = _mm_mul_ps(tempA, tempB);
		result = _mm_sub_ps(result, tempA);
		return Vec3(result);
	}

	static Vec3 BaryCoord(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& coord) {
		__m128 result = _mm_mul_ps(v1.mC, _mm_set_ps1(coord[0]));
		result = _mm_add_ps(result, _mm_mul_ps(v2.mC, _mm_set_ps1(coord[1])));
		return Vec3(_mm_add_ps(result, _mm_mul_ps(v3.mC, _mm_set_ps1(coord[2]))));
	}

	static Vec3 Min(const Vec3& v1, const Vec3& v2) {
		//returns min per component
		return Vec3(_mm_min_ps(v1.mC, v2.mC));
	}

	static Vec3 Max(const Vec3& v1, const Vec3& v2) {
		//returns min per component
		return Vec3(_mm_max_ps(v1.mC, v2.mC));
	}

	static bool WithinBounds(const Vec3& pos, const Vec3& min, const Vec3& max) {
		//__m128 min = _mm_min_ps(pos.mC, max.mC);
		//__m128 max = _mm_max_ps(pos.mC, min.mC);
		__m128 notWithinMax = _mm_cmpgt_ps(pos.mC, max.mC);
		__m128 notWithinMin = _mm_cmplt_ps(pos.mC, min.mC);
		__m128 falseComp = _mm_set_ps1(1.0f);
		return _mm_dp_ps(notWithinMax, falseComp, 0x71).m128_f32[0] == 0x0 && _mm_dp_ps(notWithinMin, falseComp, 0x71).m128_f32[0] == 0x0;
	}

	//nonstatic vector math operations
	float Magnitude() const { return _mm_dp_ps(mC, mC, 0x71).m128_f32[0]; }
	float MagnitudeSquared() const { return _mm_dp_ps(mC, mC, 0x71).m128_f32[0]; }
	void Normalize() { mC = _mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0x7F))); }
	Vec3 Normalized() const { return Vec3(_mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0x77)))); }
	void Set(float x, float y, float z) { mC = _mm_setr_ps(x, y, z, 0.0f); }
	void Set(const Vec3& v) { mC = v.mC; }

	//constants
	static Vec3 One() { return Vec3(1.); }
	static Vec3 Zero() { return Vec3(0.); }

	//other
	void ParseFromFile(FILE* file) {
		double temp[3];
		fscanf(file, "%lf %lf %lf", &temp[0], &temp[1], &temp[2]);
		Set((float)temp[0], (float)temp[1], (float)temp[2]);
	}

private:
	union
	{
		__m128 mC;
		struct { float c[3]; float pad0; };
	};
};