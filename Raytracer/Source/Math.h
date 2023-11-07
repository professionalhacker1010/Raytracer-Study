#pragma once
#include <math.h>
#include "Util.h"

struct Mat4;
struct ALIGN(16) Vec3 {
    friend Mat4;
    inline Vec3() { mC = _mm_setzero_ps(); }
	explicit Vec3(float x, float y, float z) { mC = _mm_setr_ps(x, y, z, pad0); }
    inline Vec3(const Vec3& v) { mC = v.mC; }
	explicit Vec3(float val) { mC = _mm_set_ps1(val); }
	explicit Vec3(__m128 v) { mC = v; }

	//operator overloads
    inline const Vec3& operator = (const Vec3& rhs) noexcept {
		if (this == &rhs) return *this;
        mC = rhs.mC;//_mm_setr_ps(rhs[0], rhs[1], rhs[2], 0.0f);
		return *this;
	}

    inline Vec3 operator + (const Vec3& rhs) const { return Vec3(_mm_add_ps(mC, rhs.mC)); }
    inline Vec3 operator += (const Vec3& rhs) {
		mC = _mm_add_ps(mC, rhs.mC);
		return *this;
	}
    inline Vec3 operator - (const Vec3& rhs) const { return Vec3(_mm_sub_ps(mC, rhs.mC)); }
    inline Vec3 operator -= (const Vec3& rhs) {
		mC = _mm_sub_ps(mC, rhs.mC);
		return *this;
	}
    inline  friend Vec3 operator* (float a, const Vec3& b) { return Vec3(_mm_mul_ps(b.mC, _mm_set_ps1(a))); }
    inline Vec3 operator * (const Vec3& rhs) const { return Vec3(_mm_mul_ps(mC, rhs.mC)); }
    inline Vec3 operator * (float rhs) const { return Vec3(_mm_mul_ps(mC, _mm_set_ps1(rhs))); }
    inline Vec3 operator *= (float rhs) {
		mC = _mm_mul_ps(mC, _mm_set_ps1(rhs));
		return *this;
	}
    inline Vec3 operator / (const Vec3& rhs) const { return Vec3(_mm_div_ps(mC, rhs.mC)); }
    inline Vec3 operator / (float rhs) const { return Vec3(_mm_div_ps(mC, _mm_set_ps1(rhs))); }
    inline Vec3 operator /= (float rhs) {
		mC = _mm_div_ps(mC, _mm_set_ps1(rhs));
		return *this;
	}
    inline float operator [] (int i) const { return c[i]; }
    inline operator std::string() const { return " " + std::to_string(c[0]) + " " + std::to_string(c[1]) + " " + std::to_string(c[2]); }

	//static vector math operations
    inline static float Dot(const Vec3& a, const Vec3& b) {
		return _mm_dp_ps(a.mC, b.mC, 0x71).m128_f32[0];
	}

    inline static Vec3 Cross(const Vec3& a, const Vec3& b) {
		__m128 tempA = _mm_shuffle_ps(a.mC, a.mC, _MM_SHUFFLER(1, 2, 0, 3)); //y, z, x
		__m128 tempB = _mm_shuffle_ps(b.mC, b.mC, _MM_SHUFFLER(2, 0, 1, 3)); //z, x, y
		__m128 result = _mm_mul_ps(tempA, tempB);
		tempA = _mm_shuffle_ps(a.mC, a.mC, _MM_SHUFFLER(2, 0, 1, 3));
		tempB = _mm_shuffle_ps(b.mC, b.mC, _MM_SHUFFLER(1, 2, 0, 3));
		tempA = _mm_mul_ps(tempA, tempB);
		result = _mm_sub_ps(result, tempA);
		return Vec3(result);
	}

    inline static Vec3 BaryCoord(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& coord) {
		__m128 result = _mm_mul_ps(v1.mC, _mm_set_ps1(coord[0]));
		result = _mm_add_ps(result, _mm_mul_ps(v2.mC, _mm_set_ps1(coord[1])));
		return Vec3(_mm_add_ps(result, _mm_mul_ps(v3.mC, _mm_set_ps1(coord[2]))));
	}

    inline static Vec3 Min(const Vec3& v1, const Vec3& v2) {
		return Vec3(_mm_min_ps(v1.mC, v2.mC));
	}

    inline static Vec3 Max(const Vec3& v1, const Vec3& v2) {
		return Vec3(_mm_max_ps(v1.mC, v2.mC));
	}

    static bool WithinBounds(const Vec3& pos, const Vec3& max, const Vec3& min) {
        return pos[0] <= max[0] && pos[1] <= max[1] && pos[2] <= max[2]
            && pos[0] >= min[0] && pos[1] >= min[1] && pos[2] >= min[2];
    }

	//nonstatic vector math operations
    inline float Length() const { return sqrtf(_mm_dp_ps(mC, mC, 0x71).m128_f32[0]); }
    inline float LengthSq() const { return _mm_dp_ps(mC, mC, 0x71).m128_f32[0]; }
    inline void Normalize() { mC = _mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0x7F))); }
    inline Vec3 Normalized() const { return Vec3(_mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0x77)))); }
    inline void Set(float x, float y, float z) { mC = _mm_setr_ps(x, y, z, 0.0f); }
    inline void Set(const Vec3& v) { mC = v.mC; }
    inline Vec3 Abs() { return Vec3(_mm_max_ps(_mm_sub_ps(_mm_set_ps1(0.0f), mC), mC)); }

	//constants
    inline static Vec3 One() { return Vec3(1.0f); }
    inline static Vec3 Zero() { return Vec3(0.0f); }
    inline  static Vec3 UnitX() { return Vec3(1.0f, 0.0f, 0.0f); }
    inline static Vec3 UnitY() { return Vec3(0.0f, 1.0f, 0.0f); }
    inline static Vec3 UnitZ() { return Vec3(0.0f, 0.0f, 1.0f); }

	//other
    inline void ParseFromFile(FILE* file) {
        double temp[3] = { 0. };
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



struct ALIGN(8) Vec2 {
    inline  Vec2() = default;
    inline Vec2(const Vec2& v) { c[0] = v[0]; c[1] = v[1]; }
    inline Vec2(float x, float y) { c[0] = x; c[1] = y; }
    inline Vec2(float val) { c[0] = val; c[1] = val; }

    //operator overloads
    inline const Vec2& operator = (const Vec2& rhs) noexcept {
        if (this == &rhs) return *this;
        c[0] = rhs[0]; c[1] = rhs[1];//_mm_setr_ps(rhs[0], rhs[1], rhs[2], 0.0f);
        return *this;
    }

    inline Vec2 operator + (const Vec2& rhs) const { return Vec2(c[0] + rhs[0], c[1] + rhs[1]); }
    inline Vec2 operator += (const Vec2& rhs) {
        c[0] += rhs[0]; c[1] += rhs[1];
        return *this;
    }
    inline Vec2 operator - (const Vec2& rhs) const { return Vec2(c[0] - rhs[0], c[1] - rhs[1]); }
    inline Vec2 operator -= (const Vec2& rhs) {
        c[0] -= rhs[0]; c[1] -= rhs[1];
        return *this;
    }
    inline friend Vec2 operator* (float a, const Vec2& b) { return Vec2(b[0] * a, b[1] * a); }
    inline Vec2 operator * (const Vec2& rhs) const { return Vec2(c[0] * rhs[0], c[1] * rhs[1]); }
    inline Vec2 operator * (float rhs) const { return Vec2(c[0] * rhs, c[1] * rhs); }
    inline Vec2 operator *= (float rhs) {
        c[0] *= rhs; c[1] *= rhs;
        return *this;
    }
    inline Vec2 operator / (const Vec2& rhs) const { return Vec2(c[0] / rhs[0], c[1] / rhs[1]); }
    inline Vec2 operator / (float rhs) const { return Vec2(c[0] / rhs, c[1] / rhs); }
    inline Vec2 operator /= (float rhs) {
        c[0] /= rhs; c[1] /= rhs;
        return *this;
    }
    inline float operator [] (int i) const { return c[i]; }
    inline operator std::string() const { return " " + std::to_string(c[0]) + " " + std::to_string(c[1]); }

    inline static Vec2 BaryCoord(const Vec2& v1, const Vec2& v2, const Vec2& v3, const Vec3& coord) {
        return coord[0] * v1 + coord[1] * v2 + coord[2] * v3;
    }

    inline void Set(float x, float y) { c[0] = x; c[1] = y; }
    inline void Set(const Vec2& v) { c[0] = v[0]; c[1] = v[1]; }

private:
    float c[2] = { 0.0f };
};

struct ALIGN(16) Vec4
{
public:
    
    union
    {
        __m128 mC;
        struct { float c[4]; };
    };

    inline Vec4() { *this = Vec4::Identity(); }

    // This directly sets the Vec4 components --
    // don't use for axis/angle
    explicit Vec4(float x, float y, float z, float w){ mC = _mm_set_ps(x, y, z, w); }
    inline Vec4(const Vec4& v) { mC = v.mC; }
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
    inline const Vec4& operator = (const Vec4& rhs) noexcept {
        if (this == &rhs) return *this;
        mC = _mm_setr_ps(rhs[0], rhs[1], rhs[2], rhs[3]);
        return *this;
    }
    inline Vec4 operator + (const Vec4& rhs) const { return Vec4(_mm_add_ps(mC, rhs.mC)); }
    inline Vec4 operator += (const Vec4& rhs) {
        mC = _mm_add_ps(mC, rhs.mC);
        return *this;
    }
    inline Vec4 operator - (const Vec4& rhs) const { return Vec4(_mm_sub_ps(mC, rhs.mC)); }
    inline Vec4 operator -= (const Vec4& rhs) {
        mC = _mm_sub_ps(mC, rhs.mC);
        return *this;
    }
    inline friend Vec4 operator* (float a, const Vec4& b) { return Vec4(_mm_mul_ps(b.mC, _mm_set_ps1(a))); }
    inline Vec4 operator * (const Vec4& rhs) const { return Vec4(_mm_mul_ps(mC, rhs.mC)); }
    inline Vec4 operator * (float rhs) const { return Vec4(_mm_mul_ps(mC, _mm_set_ps1(rhs))); }
    inline Vec4 operator *= (float rhs) {
        mC = _mm_mul_ps(mC, _mm_set_ps1(rhs));
        return *this;
    }
    inline Vec4 operator / (const Vec4& rhs) const { return Vec4(_mm_div_ps(mC, rhs.mC)); }
    inline Vec4 operator / (float rhs) const { return Vec4(_mm_div_ps(mC, _mm_set_ps1(rhs))); }
    inline Vec4 operator /= (float rhs) {
        mC = _mm_div_ps(mC, _mm_set_ps1(rhs));
        return *this;
    }
    inline float operator [] (int i) const { return c[i]; }
    inline operator std::string() const { return " " + std::to_string(c[0]) + " " + std::to_string(c[1]) + " " + std::to_string(c[2]) + " " + std::to_string(c[3]); }
    

    // member operations
    inline void Set(const Vec4& v) { mC = v.mC; }
    inline void Set(float x, float y, float z, float w){ mC = _mm_set_ps(x, y, z, w); }
    inline void Conjugate(){ mC = _mm_mul_ps(mC, _mm_set_ps(-1.0f, -1.0f, -1.0f, 1.0f)); }
    inline float LengthSq() const { return  _mm_dp_ps(mC, mC, 0xF1).m128_f32[0]; }
    inline float Length() const { return sqrtf(_mm_dp_ps(mC, mC, 0xF1).m128_f32[0]); }
    inline void Normalize(){ mC = _mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0xFF))); }
    inline Vec4 Normalized(){ return Vec4(_mm_mul_ps(mC, _mm_rsqrt_ps(_mm_dp_ps(mC, mC, 0xFF)))); }
    inline Vec4 Abs() { return Vec4(_mm_max_ps(_mm_sub_ps(_mm_set_ps1(0.0f), mC), mC)); }

    //static operations
    inline static Vec4 Min(const Vec4& v1, const Vec4& v2) {
        return Vec4(_mm_min_ps(v1.mC, v2.mC));
    }

    inline static Vec4 Max(const Vec4& v1, const Vec4& v2) {
        return Vec4(_mm_max_ps(v1.mC, v2.mC));
    }
    inline static Vec4 Cross(const Vec4& a, const Vec4& b) {
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
    inline static Vec4 Lerp(const Vec4& a, const Vec4& b, float f)
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
    inline static Vec4 Slerp(const Vec4& a, const Vec4& b, float f);

    // Concatenate
    // Rotate by q FOLLOWED BY p
    inline static Vec4 Concatenate(const Vec4& q, const Vec4& p)
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

    inline static float Dot(const Vec4& a, const Vec4& b){ return _mm_dp_ps(a.mC, b.mC, 0xF1).m128_f32[0]; }

    inline static Vec4 One() { return Vec4(1.0f); }
    inline static Vec4 Zero() { return Vec4(0.0f); }
    inline static Vec4 UnitX() { return Vec4(1.0f, 0.0f, 0.0f, 0.0f); }
    inline static Vec4 UnitY() { return Vec4(0.0f, 1.0f, 0.0f, 0.0f); }
    inline static Vec4 UnitZ() { return Vec4(0.0f, 0.0f, 1.0f, 0.0f); }
    inline static Vec4 UnitW() { return Vec4(0.0f, 0.0f, 0.0f, 1.0f); }
    inline static Vec4 Identity() { return Vec4(0.0f, 0.0f, 0.0f, 1.0f); }
};

typedef Vec4 Quaternion;

struct Mat4 {
public:
    inline Mat4()
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

    inline const float* GetAsFloatPtr() const { return &mat[0][0]; }

    inline friend Mat4 operator*(const Mat4& a, const Mat4& b)
	{
		// transpose b
        __m128 bT[4] = { 0 };
		__m128 tmp0 = _mm_shuffle_ps(b.rows[0], b.rows[1], 0x44);
		__m128 tmp2 = _mm_shuffle_ps(b.rows[0], b.rows[1], 0xee);
		__m128 tmp1 = _mm_shuffle_ps(b.rows[2], b.rows[3], 0x44);
		__m128 tmp3 = _mm_shuffle_ps(b.rows[2], b.rows[3], 0xee);
		bT[0] = _mm_shuffle_ps(tmp0, tmp1, 0x88);
		bT[1] = _mm_shuffle_ps(tmp0, tmp1, 0xdd);
		bT[2] = _mm_shuffle_ps(tmp2, tmp3, 0x88);
		bT[3] = _mm_shuffle_ps(tmp2, tmp3, 0xdd);

        __m128 rows[4] = { 0 };
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

    inline Mat4& operator*=(const Mat4& right)
	{
		*this = *this * right;
		return *this;
	}

	void Invert();

    // Get the translation component of the matrix
    inline Vec3 GetTranslation() const
    {
        return Vec3(rows[3]);
    }

    // Get the X axis of the matrix (forward)
    inline Vec3 GetXAxis() const
    {
        return (Vec3(rows[0])).Normalized();
    }

    // Get the Y axis of the matrix (left)
    inline Vec3 GetYAxis() const
    {
        return (Vec3(rows[1])).Normalized();
    }

    // Get the Z axis of the matrix (up)
    inline Vec3 GetZAxis() const
    {
        return (Vec3(rows[2])).Normalized();
    }

    // Extract the scale component from the matrix
    inline Vec3 GetScale() const
    {
        __m128 x = _mm_dp_ps(rows[0], rows[0], 0x71);
        __m128 y = _mm_dp_ps(rows[1], rows[1], 0x72);
        __m128 z = _mm_dp_ps(rows[2], rows[2], 0x74);
        return Vec3(_mm_sqrt_ps(_mm_add_ps(_mm_add_ps(x, y), z)));
    }

    // Transpose this matrix
    inline void Transpose()
    {
        _MM_TRANSPOSE4_PS(rows[0], rows[1], rows[2], rows[3]);
    }

    // Transpose the provided matrix
    inline friend Mat4 Transpose(const Mat4& inMat)
    {
        Mat4 retVal = inMat;
        retVal.Transpose();
        return retVal;
    }

    // Create a scale matrix with x, y, and z scales
    inline static Mat4 CreateScale(float xScale, float yScale, float zScale)
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

    inline static Mat4 CreateScale(const Vec3& scaleVector)
    {
        return CreateScale(scaleVector[0], scaleVector[1], scaleVector[2]);
    }

    // Create a scale matrix with a uniform factor
    inline static Mat4 CreateScale(float scale)
    {
        return CreateScale(scale, scale, scale);
    }

    // Rotation about x-axis
    inline static Mat4 CreateRotationX(float theta)
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
    inline static Mat4 CreateRotationY(float theta)
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
    inline static Mat4 CreateRotationZ(float theta)
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

    inline static Mat4 CreateYawPitchRoll(float yaw, float pitch, float roll)
    {
        return Mat4::CreateRotationZ(roll)
            * Mat4::CreateRotationX(pitch)
            * Mat4::CreateRotationY(yaw);
    }

    // Create a rotation matrix from a Vec4
    static Mat4 CreateFromRotation(const Vec4& q);

    inline static Mat4 CreateTranslation(const Vec3& trans)
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

    inline static Mat4 CreateLookAt(const Vec3& eye, const Vec3& at, const Vec3& up)
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

    inline static Mat4 CreateOrtho(float width, float height, float nearClip, float farClip)
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

    inline static Mat4 CreatePerspectiveFOV(float fovY, float width, float height, float nearClip, float farClip)
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

    inline static Vec3 Transform(const Vec3& vec, const Mat4& mat, float w = 1.0f)
    {
        __m128 v = _mm_set_ps(w, vec[2], vec[1], vec[0]);//_mm_set_ps(vec[0], vec[1], vec[2], w);
        return Vec3(
            _mm_dp_ps(v, _mm_set_ps(mat.mat[3][0], mat.mat[2][0], mat.mat[1][0], mat.mat[0][0]), 0xF1).m128_f32[0],
            _mm_dp_ps(v, _mm_set_ps(mat.mat[3][1], mat.mat[2][1], mat.mat[1][1], mat.mat[0][1]), 0xF1).m128_f32[0],
            _mm_dp_ps(v, _mm_set_ps(mat.mat[3][2], mat.mat[2][2], mat.mat[1][2], mat.mat[0][2]), 0xF1).m128_f32[0]
        );
    }

    inline static Mat4 Identity() {
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

//class quat // based on https://github.com/adafruit
//{
//public:
//    quat() = default;
//    quat(float _w, float _x, float _y, float _z) : w(_w), x(_x), y(_y), z(_z) {}
//    quat(float _w, float3 v) : w(_w), x(v.x), y(v.y), z(v.z) {}
//    float magnitude() const { return sqrtf(w * w + x * x + y * y + z * z); }
//    void normalize() { float m = magnitude(); *this = this->scale(1 / m); }
//    quat conjugate() const { return quat(w, -x, -y, -z); }
//    void fromAxisAngle(const float3& axis, float theta)
//    {
//        w = cosf(theta / 2);
//        const float s = sinf(theta / 2);
//        x = axis.x * s, y = axis.y * s, z = axis.z * s;
//    }
//    void fromMatrix(const mat4& m)
//    {
//        float tr = m.Trace3(), S;
//        if (tr > 0)
//        {
//            S = sqrtf(tr + 1.0f) * 2, w = 0.25f * S;
//            x = (m(2, 1) - m(1, 2)) / S, y = (m(0, 2) - m(2, 0)) / S;
//            z = (m(1, 0) - m(0, 1)) / S;
//        }
//        else if (m(0, 0) > m(1, 1) && m(0, 0) > m(2, 2))
//        {
//            S = sqrt(1.0f + m(0, 0) - m(1, 1) - m(2, 2)) * 2;
//            w = (m(2, 1) - m(1, 2)) / S, x = 0.25f * S;
//            y = (m(0, 1) + m(1, 0)) / S, z = (m(0, 2) + m(2, 0)) / S;
//        }
//        else if (m(1, 1) > m(2, 2))
//        {
//            S = sqrt(1.0f + m(1, 1) - m(0, 0) - m(2, 2)) * 2;
//            w = (m(0, 2) - m(2, 0)) / S;
//            x = (m(0, 1) + m(1, 0)) / S, y = 0.25f * S;
//            z = (m(1, 2) + m(2, 1)) / S;
//        }
//        else
//        {
//            S = sqrt(1.0f + m(2, 2) - m(0, 0) - m(1, 1)) * 2;
//            w = (m(1, 0) - m(0, 1)) / S, x = (m(0, 2) + m(2, 0)) / S;
//            y = (m(1, 2) + m(2, 1)) / S, z = 0.25f * S;
//        }
//    }
//    void toAxisAngle(float3& axis, float& angle) const
//    {
//        float s = sqrtf(1 - w * w);
//        if (s == 0) return;
//        angle = 2 * acosf(w);
//        axis.x = x / s, axis.y = y / s, axis.z = z / s;
//    }
//    mat4 toMatrix() const
//    {
//        mat4 ret;
//        ret.cell[0] = 1 - 2 * y * y - 2 * z * z;
//        ret.cell[1] = 2 * x * y - 2 * w * z, ret.cell[2] = 2 * x * z + 2 * w * y, ret.cell[4] = 2 * x * y + 2 * w * z;
//        ret.cell[5] = 1 - 2 * x * x - 2 * z * z;
//        ret.cell[6] = 2 * y * z - 2 * w * x, ret.cell[8] = 2 * x * z - 2 * w * y, ret.cell[9] = 2 * y * z + 2 * w * x;
//        ret.cell[10] = 1 - 2 * x * x - 2 * y * y;
//        return ret;
//    }
//    float3 toEuler() const
//    {
//        float3 ret;
//        float sqw = w * w, sqx = x * x, sqy = y * y, sqz = z * z;
//        ret.x = atan2f(2.0f * (x * y + z * w), (sqx - sqy - sqz + sqw));
//        ret.y = asinf(-2.0f * (x * z - y * w) / (sqx + sqy + sqz + sqw));
//        ret.z = atan2f(2.0f * (y * z + x * w), (-sqx - sqy + sqz + sqw));
//        return ret;
//    }
//    float3 toAngularVelocity(float dt) const
//    {
//        float3 ret;
//        quat one(1, 0, 0, 0), delta = one - *this, r = (delta / dt);
//        r = r * 2, r = r * one, ret.x = r.x, ret.y = r.y, ret.z = r.z;
//        return ret;
//    }
//    float3 rotateVector(const float3& v) const
//    {
//        float3 qv = make_float3(x, y, z), t = cross(qv, v) * 2.0f;
//        return v + t * w + cross(qv, t);
//    }
//    quat operator * (const quat& q) const
//    {
//        return quat(
//            w * q.w - x * q.x - y * q.y - z * q.z, w * q.x + x * q.w + y * q.z - z * q.y,
//            w * q.y - x * q.z + y * q.w + z * q.x, w * q.z + x * q.y - y * q.x + z * q.w
//        );
//    }
//    static quat slerp(const quat& a, const quat& b, const float t)
//    {
//        // from https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
//        quat qm;
//        float cosHalfTheta = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
//        if (abs(cosHalfTheta) >= 1.0)
//        {
//            qm.w = a.w, qm.x = a.x, qm.y = a.y, qm.z = a.z;
//            return qm;
//        }
//        float halfTheta = acosf(cosHalfTheta);
//        float sinHalfTheta = sqrtf(1.0f - cosHalfTheta * cosHalfTheta);
//        if (fabs(sinHalfTheta) < 0.001f)
//        {
//            qm.w = a.w * 0.5f + b.w * 0.5f, qm.x = a.x * 0.5f + b.x * 0.5f;
//            qm.y = a.y * 0.5f + b.y * 0.5f, qm.z = a.z * 0.5f + b.z * 0.5f;
//            return qm;
//        }
//        float ratioA = sinf((1 - t) * halfTheta) / sinHalfTheta;
//        float ratioB = sinf(t * halfTheta) / sinHalfTheta;
//        qm.w = (a.w * ratioA + b.w * ratioB), qm.x = (a.x * ratioA + b.x * ratioB);
//        qm.y = (a.y * ratioA + b.y * ratioB), qm.z = (a.z * ratioA + b.z * ratioB);
//        return qm;
//    }
//    quat operator + (const quat& q) const { return quat(w + q.w, x + q.x, y + q.y, z + q.z); }
//    quat operator - (const quat& q) const { return quat(w - q.w, x - q.x, y - q.y, z - q.z); }
//    quat operator / (float s) const { return quat(w / s, x / s, y / s, z / s); }
//    quat operator * (float s) const { return scale(s); }
//    quat scale(float s) const { return quat(w * s, x * s, y * s, z * s); }
//    float w = 1, x = 0, y = 0, z = 0;
//};