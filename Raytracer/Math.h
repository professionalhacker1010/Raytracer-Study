#pragma once
#include <math.h>
#include <string>
#include <smmintrin.h>
#include "Constants.h"

struct Mat4;
struct Vec3 {
    friend Mat4;
	Vec3() { mC = _mm_setzero_ps(); }
	explicit Vec3(float x, float y, float z) { mC = _mm_setr_ps(x, y, z, pad0); }
	Vec3(const Vec3& v) { mC = v.mC; }
	explicit Vec3(float val) { mC = _mm_set_ps1(val); }
	explicit Vec3(__m128 v) { mC = v; }

	//operator overloads
	const Vec3& operator = (const Vec3& rhs) noexcept { 
		if (this == &rhs) return *this;
        mC = rhs.mC;//_mm_setr_ps(rhs[0], rhs[1], rhs[2], 0.0f);
		return *this;
	}

	Vec3 operator + (const Vec3& rhs) const { return Vec3(_mm_add_ps(mC, rhs.mC)); }
	Vec3 operator += (const Vec3& rhs) {
		mC = _mm_add_ps(mC, rhs.mC);
		return *this;
	}
	Vec3 operator - (const Vec3& rhs) const { return Vec3(_mm_sub_ps(mC, rhs.mC)); }
	Vec3 operator -= (const Vec3& rhs) {
		mC = _mm_sub_ps(mC, rhs.mC);
		return *this;
	}
    friend Vec3 operator* (float a, const Vec3& b) { return Vec3(_mm_mul_ps(b.mC, _mm_set_ps1(a))); }
	Vec3 operator * (const Vec3& rhs) const { return Vec3(_mm_mul_ps(mC, rhs.mC)); }
	Vec3 operator * (float rhs) const { return Vec3(_mm_mul_ps(mC, _mm_set_ps1(rhs))); }
	Vec3 operator *= (float rhs) {
		mC = _mm_mul_ps(mC, _mm_set_ps1(rhs));
		return *this;
	}
	Vec3 operator / (const Vec3& rhs) const { return Vec3(_mm_div_ps(mC, rhs.mC)); }
	Vec3 operator / (float rhs) const { return Vec3(_mm_div_ps(mC, _mm_set_ps1(rhs))); }
	Vec3 operator /= (float rhs) {
		mC = _mm_div_ps(mC, _mm_set_ps1(rhs));
		return *this;
	}
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
		return Vec3(_mm_min_ps(v1.mC, v2.mC));
	}

	static Vec3 Max(const Vec3& v1, const Vec3& v2) {
		return Vec3(_mm_max_ps(v1.mC, v2.mC));
	}

	//static bool WithinBounds(const Vec3& pos, const Vec3& min, const Vec3& max) {
	//	//__m128 min = _mm_min_ps(pos.mC, max.mC);
	//	//__m128 max = _mm_max_ps(pos.mC, min.mC);
	//	__m128 notWithinMax = _mm_cmpgt_ps(pos.mC, max.mC);
	//	__m128 notWithinMin = _mm_cmplt_ps(pos.mC, min.mC);
	//	__m128 falseComp = _mm_set_ps1(1.0f);
	//	return _mm_dp_ps(notWithinMax, falseComp, 0x71).m128_f32[0] == 0x0 && _mm_dp_ps(notWithinMin, falseComp, 0x71).m128_f32[0] == 0x0;
	//}

	//nonstatic vector math operations
	float Length() const { return sqrtf(_mm_dp_ps(mC, mC, 0x71).m128_f32[0]); }
	float LengthSq() const { return _mm_dp_ps(mC, mC, 0x71).m128_f32[0]; }
	void Normalize() { mC = _mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0x7F))); }
	Vec3 Normalized() const { return Vec3(_mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0x77)))); }
	void Set(float x, float y, float z) { mC = _mm_setr_ps(x, y, z, 0.0f); }
	void Set(const Vec3& v) { mC = v.mC; }
	Vec3 Abs() { return Vec3(_mm_max_ps(_mm_sub_ps(_mm_set_ps1(0.0f), mC), mC)); }

	//constants
    static Vec3 One() { return Vec3(1.0f); }
    static Vec3 Zero() { return Vec3(0.0f); }
    static Vec3 UnitX() { return Vec3(1.0f, 0.0f, 0.0f); }
    static Vec3 UnitY() { return Vec3(0.0f, 1.0f, 0.0f); }
    static Vec3 UnitZ() { return Vec3(0.0f, 0.0f, 1.0f); }

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

struct Vec2 {
    Vec2() = default;
    Vec2(const Vec2& v) { c[0] = v[0]; c[1] = v[1]; }
    Vec2(float x, float y) { c[0] = x; c[1] = y; }
    Vec2(float val) { c[0] = val; c[1] = val; }

    //operator overloads
    const Vec2& operator = (const Vec2& rhs) noexcept {
        if (this == &rhs) return *this;
        c[0] = rhs[0]; c[1] = rhs[1];//_mm_setr_ps(rhs[0], rhs[1], rhs[2], 0.0f);
        return *this;
    }

    Vec2 operator + (const Vec2& rhs) const { return Vec2(c[0] + rhs[0], c[1] + rhs[1]); }
    Vec2 operator += (const Vec2& rhs) {
        c[0] += rhs[0]; c[1] += rhs[1];
        return *this;
    }
    Vec2 operator - (const Vec2& rhs) const { return Vec2(c[0] - rhs[0], c[1] - rhs[1]); }
    Vec2 operator -= (const Vec2& rhs) {
        c[0] -= rhs[0]; c[1] -= rhs[1];
        return *this;
    }
    friend Vec2 operator* (float a, const Vec2& b) { return Vec2(b[0] * a, b[1] * a); }
    Vec2 operator * (const Vec2& rhs) const { return Vec2(c[0] * rhs[0], c[1] * rhs[1]); }
    Vec2 operator * (float rhs) const { return Vec2(c[0] * rhs, c[1] * rhs); }
    Vec2 operator *= (float rhs) {
        c[0] *= rhs; c[1] *= rhs;
        return *this;
    }
    Vec2 operator / (const Vec2& rhs) const { return Vec2(c[0] / rhs[0], c[1] / rhs[1]); }
    Vec2 operator / (float rhs) const { return Vec2(c[0] / rhs, c[1] / rhs); }
    Vec2 operator /= (float rhs) {
        c[0] /= rhs; c[1] /= rhs;
        return *this;
    }
    float operator [] (int i) const { return c[i]; }
    operator std::string() const { return " " + std::to_string(c[0]) + " " + std::to_string(c[1]); }

    static Vec2 BaryCoord(const Vec2& v1, const Vec2& v2, const Vec2& v3, const Vec3& coord) {
        return coord[0] * v1 + coord[1] * v2 + coord[3] * v3;
    }

    void Set(float x, float y) { c[0] = x; c[1] = y; }
    void Set(const Vec2& v) { c[0] = v[0]; c[1] = v[1]; }

private:
    float c[2] = { 0.0f };
};

struct Vec4
{
public:
    
    union
    {
        __m128 mC;
        struct { float c[4]; };
    };

    Vec4() { *this = Vec4::Identity(); }

    // This directly sets the Vec4 components --
    // don't use for axis/angle
    explicit Vec4(float x, float y, float z, float w){ mC = _mm_set_ps(x, y, z, w); }
    Vec4(const Vec4& v) { mC = v.mC; }
    explicit Vec4(float val) { mC = _mm_set_ps1(val); }
    explicit Vec4(__m128 v) { mC = v; }

    // Construct the Vec4 from an axis and angle
    // It is assumed that axis is already normalized,
    // and the angle is in radians
    explicit Vec4(const Vec3& axis, float angle)
    {
        float scalar = sinf(angle / 2.0f);
        Vec3 temp = (axis * scalar);
        mC = _mm_set_ps(temp[0], temp[1], temp[2], cosf(angle / 2.0f));
    }

    //operator overloads
    const Vec4& operator = (const Vec4& rhs) noexcept {
        if (this == &rhs) return *this;
        mC = _mm_setr_ps(rhs[0], rhs[1], rhs[2], rhs[3]);
        return *this;
    }
    Vec4 operator + (const Vec4& rhs) const { return Vec4(_mm_add_ps(mC, rhs.mC)); }
    Vec4 operator += (const Vec4& rhs) {
        mC = _mm_add_ps(mC, rhs.mC);
        return *this;
    }
    Vec4 operator - (const Vec4& rhs) const { return Vec4(_mm_sub_ps(mC, rhs.mC)); }
    Vec4 operator -= (const Vec4& rhs) {
        mC = _mm_sub_ps(mC, rhs.mC);
        return *this;
    }
    friend Vec4 operator* (float a, const Vec4& b) { return Vec4(_mm_mul_ps(b.mC, _mm_set_ps1(a))); }
    Vec4 operator * (const Vec4& rhs) const { return Vec4(_mm_mul_ps(mC, rhs.mC)); }
    Vec4 operator * (float rhs) const { return Vec4(_mm_mul_ps(mC, _mm_set_ps1(rhs))); }
    Vec4 operator *= (float rhs) {
        mC = _mm_mul_ps(mC, _mm_set_ps1(rhs));
        return *this;
    }
    Vec4 operator / (const Vec4& rhs) const { return Vec4(_mm_div_ps(mC, rhs.mC)); }
    Vec4 operator / (float rhs) const { return Vec4(_mm_div_ps(mC, _mm_set_ps1(rhs))); }
    Vec4 operator /= (float rhs) {
        mC = _mm_div_ps(mC, _mm_set_ps1(rhs));
        return *this;
    }
    float operator [] (int i) const { return c[i]; }
    operator std::string() const { return " " + std::to_string(c[0]) + " " + std::to_string(c[1]) + " " + std::to_string(c[2]) + " " + std::to_string(c[3]); }
    

    // member operations
    void Set(const Vec4& v) { mC = v.mC; }
    void Set(float x, float y, float z, float w){ mC = _mm_set_ps(x, y, z, w); }
    void Conjugate(){ mC = _mm_mul_ps(mC, _mm_set_ps(-1.0f, -1.0f, -1.0f, 1.0f)); }
    float LengthSq() const { return  _mm_dp_ps(mC, mC, 0xF1).m128_f32[0]; }
    float Length() const { return sqrtf(_mm_dp_ps(mC, mC, 0xF1).m128_f32[0]); }
    void Normalize(){ mC = _mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0xFF))); }
    Vec4 Normalized(){ return Vec4(_mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0xFF)))); }
    Vec4 Abs() { return Vec4(_mm_max_ps(_mm_sub_ps(_mm_set_ps1(0.0f), mC), mC)); }

    //static operations
    static Vec4 Min(const Vec4& v1, const Vec4& v2) {
        return Vec4(_mm_min_ps(v1.mC, v2.mC));
    }

    static Vec4 Max(const Vec4& v1, const Vec4& v2) {
        return Vec4(_mm_max_ps(v1.mC, v2.mC));
    }
    static Vec4 Cross(const Vec4& a, const Vec4& b) {
        __m128 tempA = _mm_shuffle_ps(a.mC, a.mC, _MM_SHUFFLER(1, 2, 0, 3)); //y, z, x
        __m128 tempB = _mm_shuffle_ps(b.mC, b.mC, _MM_SHUFFLER(2, 0, 1, 3)); //z, x, y
        __m128 result = _mm_mul_ps(tempA, tempB);
        tempA = _mm_shuffle_ps(a.mC, a.mC, _MM_SHUFFLER(2, 0, 1, 3));
        tempB = _mm_shuffle_ps(b.mC, b.mC, _MM_SHUFFLER(1, 2, 0, 3));
        tempA = _mm_mul_ps(tempA, tempB);
        result = _mm_sub_ps(result, tempA);
        return Vec4(result);
    }
    // Linear interpolation
    static Vec4 Lerp(const Vec4& a, const Vec4& b, float f)
    {
        const float dotResult = _mm_dp_ps(a.mC, b.mC, 0xF1).m128_f32[0];
        float bias = -1.0f;
        if (dotResult >= 0.0f)
        {
            bias = 1.0f;
        }

        Vec4 retVal = b * f + a * bias * (1.0f - f);
        return retVal;
    }

    // Spherical Linear Interpolation
    static Vec4 Slerp(const Vec4& a, const Vec4& b, float f);

    // Concatenate
    // Rotate by q FOLLOWED BY p
    static Vec4 Concatenate(const Vec4& q, const Vec4& p)
    {
        // Vector component is:
        // ps * qv + qs * pv + pv x qv
        Vec3 qv(q[0], q[1], q[2]);
        Vec3 pv(p[0], p[1], p[2]);
        Vec3 newVec = p[3] * qv + q[3] * pv + Vec3::Cross(pv, qv);

        // Scalar component is:
        // ps * qs - pv . qv
        return Vec4(newVec[0], newVec[1], newVec[2], p[3] * q[3] - Vec3::Dot(pv, qv));;
    }

    static float Dot(const Vec4& a, const Vec4& b){ return _mm_dp_ps(a.mC, b.mC, 0xF1).m128_f32[0]; }

    static Vec4 One() { return Vec4(1.0f); }
    static Vec4 Zero() { return Vec4(0.0f); }
    static Vec4 UnitX() { return Vec4(1.0f, 0.0f, 0.0f, 0.0f); }
    static Vec4 UnitY() { return Vec4(0.0f, 1.0f, 0.0f, 0.0f); }
    static Vec4 UnitZ() { return Vec4(0.0f, 0.0f, 1.0f, 0.0f); }
    static Vec4 UnitW() { return Vec4(0.0f, 0.0f, 0.0f, 1.0f); }
    static Vec4 Identity() { return Vec4(0.0f, 0.0f, 0.0f, 1.0f); }
};

typedef Vec4 Quaternion;

struct Mat4 {
public:
	Mat4()
	{
		rows[0] = _mm_setzero_ps();
		rows[1] = _mm_setzero_ps();
		rows[2] = _mm_setzero_ps();
		rows[3] = _mm_setzero_ps();
	}

	explicit Mat4(const float inMat[4][4]){ memcpy(mat, inMat, 16 * sizeof(float)); }

	explicit Mat4(const __m128 inRows[4])
	{
		rows[0] = inRows[0];
		rows[1] = inRows[1];
		rows[2] = inRows[2];
		rows[3] = inRows[3];
	}

	const float* GetAsFloatPtr() const { return &mat[0][0]; }

	friend Mat4 operator*(const Mat4& a, const Mat4& b)
	{
		// transpose b
		__m128 bT[4];
		__m128 tmp0 = _mm_shuffle_ps(b.rows[0], b.rows[1], 0x44);
		__m128 tmp2 = _mm_shuffle_ps(b.rows[0], b.rows[1], 0xee);
		__m128 tmp1 = _mm_shuffle_ps(b.rows[2], b.rows[3], 0x44);
		__m128 tmp3 = _mm_shuffle_ps(b.rows[2], b.rows[3], 0xee);
		bT[0] = _mm_shuffle_ps(tmp0, tmp1, 0x88);
		bT[1] = _mm_shuffle_ps(tmp0, tmp1, 0xdd);
		bT[2] = _mm_shuffle_ps(tmp2, tmp3, 0x88);
		bT[3] = _mm_shuffle_ps(tmp2, tmp3, 0xdd);

		__m128 rows[4];
		for (int i = 0; i < 4; i++)
		{
			rows[i] = _mm_add_ps(
				_mm_add_ps(_mm_dp_ps(a.rows[i], bT[0], 0xF1),
					_mm_dp_ps(a.rows[i], bT[1], 0xF2)
				),
				_mm_add_ps(_mm_dp_ps(a.rows[i], bT[2], 0xF4),
					_mm_dp_ps(a.rows[i], bT[3], 0xF8)
				)
			);
		}

		return Mat4(rows);
	}

	Mat4& operator*=(const Mat4& right)
	{
		*this = *this * right;
		return *this;
	}

	void Invert();

    // Get the translation component of the matrix
    Vec3 GetTranslation() const
    {
        return Vec3(rows[3]);
    }

    // Get the X axis of the matrix (forward)
    Vec3 GetXAxis() const
    {
        return (Vec3(rows[0])).Normalized();
    }

    // Get the Y axis of the matrix (left)
    Vec3 GetYAxis() const
    {
        return (Vec3(rows[1])).Normalized();
    }

    // Get the Z axis of the matrix (up)
    Vec3 GetZAxis() const
    {
        return (Vec3(rows[2])).Normalized();
    }

    // Extract the scale component from the matrix
    Vec3 GetScale() const
    {
        __m128 x = _mm_dp_ps(rows[0], rows[0], 0x71);
        __m128 y = _mm_dp_ps(rows[1], rows[1], 0x72);
        __m128 z = _mm_dp_ps(rows[2], rows[2], 0x74);
        return Vec3(_mm_sqrt_ps(_mm_add_ps(_mm_add_ps(x, y), z)));
    }

    // Transpose this matrix
    void Transpose()
    {
        _MM_TRANSPOSE4_PS(rows[0], rows[1], rows[2], rows[3]);
    }

    // Transpose the provided matrix
    friend Mat4 Transpose(const Mat4& inMat)
    {
        Mat4 retVal = inMat;
        retVal.Transpose();
        return retVal;
    }

    // Create a scale matrix with x, y, and z scales
    static Mat4 CreateScale(float xScale, float yScale, float zScale)
    {
        float temp[4][4] =
        {
            { xScale, 0.0f, 0.0f, 0.0f },
            { 0.0f, yScale, 0.0f, 0.0f },
            { 0.0f, 0.0f, zScale, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
        return Mat4(temp);
    }

    static Mat4 CreateScale(const Vec3& scaleVector)
    {
        return CreateScale(scaleVector[0], scaleVector[1], scaleVector[2]);
    }

    // Create a scale matrix with a uniform factor
    static Mat4 CreateScale(float scale)
    {
        return CreateScale(scale, scale, scale);
    }

    // Rotation about x-axis
    static Mat4 CreateRotationX(float theta)
    {
        float temp[4][4] =
        {
            { 1.0f, 0.0f, 0.0f , 0.0f },
            { 0.0f, cosf(theta), sinf(theta), 0.0f },
            { 0.0f, -sinf(theta), cosf(theta), 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        };
        return Mat4(temp);
    }

    // Rotation about y-axis
    static Mat4 CreateRotationY(float theta)
    {
        float temp[4][4] =
        {
            { cosf(theta), 0.0f, -sinf(theta), 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { sinf(theta), 0.0f, cosf(theta), 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        };
        return Mat4(temp);
    }

    // Rotation about z-axis
    static Mat4 CreateRotationZ(float theta)
    {
        float temp[4][4] =
        {
            { cosf(theta), sinf(theta), 0.0f, 0.0f },
            { -sinf(theta), cosf(theta), 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        };
        return Mat4(temp);
    }

    static Mat4 CreateYawPitchRoll(float yaw, float pitch, float roll)
    {
        return Mat4::CreateRotationZ(roll)
            * Mat4::CreateRotationX(pitch)
            * Mat4::CreateRotationY(yaw);
    }

    // Create a rotation matrix from a Vec4
    static Mat4 CreateFromRotation(const Vec4& q);

    static Mat4 CreateTranslation(const Vec3& trans)
    {
        float temp[4][4] =
        {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { trans[0], trans[1], trans[2], 1.0f}
        };
        return Mat4(temp);
    }

    static Mat4 CreateLookAt(const Vec3& eye, const Vec3& at, const Vec3& up)
    {
        Vec3 forward = (at - eye).Normalized();
        Vec3 left = (Vec3::Cross(up, forward)).Normalized();
        Vec3 newUp = (Vec3::Cross(forward, left)).Normalized();

        float temp[4][4] =
        {
            { left[0], left[1], left[2], 0.0f},
            { newUp[0], newUp[1], newUp[2], 0.0f},
            { forward[0], forward[1], forward[2], 0.0f},
            { eye[0], eye[1], eye[2], 1.0f}
        };
        return Mat4(temp);
    }

    static Mat4 CreateOrtho(float width, float height, float nearClip, float farClip)
    {
        float temp[4][4] =
        {
            { 2.0f / width, 0.0f, 0.0f, 0.0f },
            { 0.0f, 2.0f / height, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f / (farClip - nearClip), 0.0f },
            { 0.0f, 0.0f, nearClip / (nearClip - farClip), 1.0f }
        };
        return Mat4(temp);
    }

    static Mat4 CreatePerspectiveFOV(float fovY, float width, float height, float nearClip, float farClip)
    {
        float yScale = 1.0f / tanf(fovY / 2.0f);
        float xScale = yScale * height / width;
        float temp[4][4] =
        {
            { xScale, 0.0f, 0.0f, 0.0f },
            { 0.0f, yScale, 0.0f, 0.0f },
            { 0.0f, 0.0f, farClip / (farClip - nearClip), 1.0f },
            { 0.0f, 0.0f, -nearClip * farClip / (farClip - nearClip), 0.0f }
        };
        return Mat4(temp);
    }

    static Vec3 Transform(const Vec3& vec, const Mat4& mat, float w = 1.0f)
    {
        __m128 v = _mm_set_ps(vec[0], vec[1], vec[2], w);
        return Vec3(
            _mm_dp_ps(v, _mm_set_ps(mat.mat[0][0], mat.mat[1][0], mat.mat[2][0], mat.mat[3][0]), 0xF1).m128_f32[0],
            _mm_dp_ps(v, _mm_set_ps(mat.mat[0][1], mat.mat[1][1], mat.mat[2][1], mat.mat[3][1]), 0xF1).m128_f32[0],
            _mm_dp_ps(v, _mm_set_ps(mat.mat[0][2], mat.mat[1][2], mat.mat[2][2], mat.mat[3][2]), 0xF1).m128_f32[0]
        );
    }

    static Mat4 Identity() {
        static const float m4Ident[4][4] =
        {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
        return Mat4(m4Ident);
    }
private:
	union {
		__m128 rows[4];
		float mat[4][4];
	};
};