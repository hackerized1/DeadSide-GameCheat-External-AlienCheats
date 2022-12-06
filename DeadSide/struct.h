#include <d3d9.h>
#include <math.h>
#define M_PI 3.14159265358979323846264338327950288419716939937510
#pragma once

#define UCONST_Pi 3.1415926535
#define RadianToURotation 180.0f / UCONST_Pi
template<class T>
class TArray {

public:
	TArray() {
		Data = NULL;
		Count = 0;
		Max = 0;
	};

	T operator[](uint64_t i) const {
		return driver.read<uintptr_t>(((uintptr_t)Data) + i * sizeof(T));
	};

	T* Data;
	unsigned int Count;
	unsigned int Max;
};
#define powFFFFFFFFFFFFFFFFFFFFFF(n) (n)*(n)
class Vector3
{
public:

	inline static float sqrtf(float number)
	{
		long i;
		float x2, y;
		const float threehalfs = 1.5F;

		x2 = number * 0.5F;
		y = number;
		i = *(long*)&y;
		i = 0x5f3759df - (i >> 1);
		y = *(float*)&i;
		y = y * (threehalfs - (x2 * y * y));
		y = y * (threehalfs - (x2 * y * y));

		return 1 / y;
	}

	float distance(Vector3 vec)
	{
		return sqrt(
			powFFFFFFFFFFFFFFFFFFFFFF(vec.x - x, 2) +
			powFFFFFFFFFFFFFFFFFFFFFF(vec.y - y, 2)
		);
	}

	Vector3()
	{
		x = y = z = 0.f;
	}

	Vector3(float fx, float fy, float fz)
	{
		x = fx;
		y = fy;
		z = fz;
	}

	inline float operator[](int i) const {
		return ((float*)this)[i];
	}

	inline float Length() const
	{
		return sqrt((x * x) + (y * y) + (z * z));
	}

	float x, y, z;

	Vector3 operator+(const Vector3& input) const
	{
		return Vector3{ x + input.x, y + input.y, z + input.z };
	}

	Vector3 operator-(const Vector3& input) const
	{
		return Vector3{ x - input.x, y - input.y, z - input.z };
	}

	Vector3 operator/(float input) const
	{
		return Vector3{ x / input, y / input, z / input };
	}

	Vector3 operator*(float input) const
	{
		return Vector3{ x * input, y * input, z * input };
	}

	Vector3& operator+=(const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}

	Vector3& operator-=(const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}

	Vector3& operator/=(float input)
	{
		x /= input;
		y /= input;
		z /= input;
		return *this;
	}

	Vector3& operator*=(float input)
	{
		x *= input;
		y *= input;
		z *= input;
		return *this;
	}

	Vector3 midPoint(Vector3 v2)
	{
		return Vector3((x + v2.x) / 2, (y + v2.y) / 2, (z + v2.z) / 2);
	}

	bool operator==(const Vector3& input) const
	{
		return x == input.x && y == input.y && z == input.z;
	}



	float clamp0to1(float value)
	{
		float result;
		if (value < 0)
		{
			result = 0;
		}
		else if (value > 1.f)
		{
			result = 1.f;
		}
		else
		{
			result = value;
		}
		return result;
	}

	float Lerp()
	{
		return x + (y - x) * clamp0to1(z);
	}

	float length_sqr() const
	{
		return (x * x) + (y * y) + (z * z);
	}

	float length() const
	{
		return (float)sqrt(length_sqr());
	}

	float length_2d() const
	{
		return (float)sqrt((x * x) + (y * y));
	}

	auto length() -> float { return sqrtf((x * x) + (y * y) + (z * z)); }

	Vector3 normalized()
	{
		float len = length();
		return Vector3(x / len, y / len, z / len);
	}

	Vector3 cross(Vector3 rhs)
	{
		return Vector3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
	}



	float dot(Vector3 input) const
	{
		return (x * input.x) + (y * input.y) + (z * input.z);
	}


	auto empty() -> bool { return x == 0.000000 && y == 0.000000 && z == 0.000000; }

	bool is_valid() const
	{
		return !(x == 0.f && y == 0.f && z == 0.f) || (x == -1.f && y == -1.f && z == -1.f);
	}

	inline float Dot(Vector3 v) {
		return x * v.x + y * v.y + z * v.z;
	}

	float distancee(Vector3 vec)
	{
		return sqrt(
			pow(vec.x - x, 2) +
			pow(vec.y - y, 2)
		);
	}


	


	float distance_to(const Vector3& other) {
		Vector3 delta;
		delta.x = x - other.x;
		delta.y = y - other.y;
		delta.z = z - other.z;

		return delta.length();
	}
};

Vector3 fhgfsdhkfshdghfsd205(Vector3 src, Vector3 dst)
{
	Vector3 angle;
	angle.x = -atan2f(dst.x - src.x, dst.y - src.y) / M_PI * 180.0f + 180.0f;
	angle.y = asinf((dst.z - src.z) / src.distancee(dst)) * 180.0f / M_PI;
	angle.z = 0.0f;

	return angle;
}
Vector3 CaadadalcAngle(Vector3 src, Vector3 dst)
{
	Vector3 angle;
	Vector3 delta = Vector3((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));

	double hyp = sqrtf(delta.x * delta.x + delta.y * delta.y);

	angle.x = atanf(delta.z / hyp) * (180.0f / hyp);
	angle.y = atanf(delta.y / delta.x) * (180.0f / M_PI);
	angle.z = 0;
	if (delta.x >= 0.0) angle.y += 180.0f;

	return angle;
}
float deg_2_rad(float degrees)
{
	float radians;
	radians = degrees * (M_PI / 180);
	return radians;
}
void angle_vectors(const Vector3& angles, Vector3* forward)
{
	float    sp, sy, cp, cy;
	sy = sin(deg_2_rad(angles.y));
	cy = cos(deg_2_rad(angles.y));
	sp = sin(deg_2_rad(angles.x));
	cp = cos(deg_2_rad(angles.x));
	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}
void NormalizeAngles(Vector3& angle)
{
	while (angle.x > 89.0f)
		angle.x -= 180.f;

	while (angle.x < -89.0f)
		angle.x += 180.f;

	while (angle.y > 180.f)
		angle.y -= 360.f;

	while (angle.y < -180.f)
		angle.y += 360.f;
}

class Vector2
{
public:
	float x, y;

	__forceinline Vector2()
	{
		x = y = 0.0f;
	}

	__forceinline Vector2(float X, float Y)
	{
		x = X; y = Y;
	}

	float distance(Vector2 b)
	{
		return sqrt(powFFFFFFFFFFFFFFFFFFFFFF(b.x - x, 2) + powFFFFFFFFFFFFFFFFFFFFFF(b.y - y, 2));
	}


	__forceinline Vector2 operator+(float v) const
	{
		return Vector2(x + v, y + v);
	}

	__forceinline Vector2 operator-(float v) const
	{
		return Vector2(x - v, y - v);
	}

	__forceinline Vector2& operator+=(float v)
	{
		x += v; y += v; return *this;
	}

	__forceinline Vector2& operator*=(float v)
	{
		x *= v; y *= v; return *this;
	}

	__forceinline Vector2& operator/=(float v)
	{
		x /= v; y /= v; return *this;
	}

	__forceinline Vector2 operator-(const Vector2& v) const
	{
		return Vector2(x - v.x, y - v.y);
	}

	__forceinline Vector2 operator+(const Vector2& v) const
	{
		return Vector2(x + v.x, y + v.y);
	}

	__forceinline Vector2& operator+=(const Vector2& v)
	{
		x += v.x; y += v.y; return *this;
	}

	__forceinline Vector2& operator-=(const Vector2& v) {
		x -= v.x; y -= v.y; return *this;
	}

	__forceinline Vector2 operator/(float v) const {
		return Vector2(x / v, y / v);
	}

	__forceinline Vector2 operator*(float v) const {
		return Vector2(x * v, y * v);
	}

	__forceinline Vector2 operator/(const Vector2& v) const {
		return Vector2(x / v.x, y / v.y);
	}

	__forceinline bool Zero() const {
		return (x > -0.1f && x < 0.1f && y > -0.1f && y < 0.1f);
	}



};

struct FQuat
{
	float x;
	float y;
	float z;
	float w;
};
inline D3DXMATRIX Matrix(Vector3 rot, Vector3 origin)
{
	float radPitch = (rot.x * float(M_PI) / 180.f);
	float radYaw = (rot.y * float(M_PI) / 180.f);
	float radRoll = (rot.z * float(M_PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}
struct FMinimalViewInfo
{
	Vector3 Location;
	Vector3 Rotation;
	float FOV;
};


struct FTransform
{
	FQuat rot;
	Vector3 translation;
	char pad[4];
	Vector3 scale;
	char pad1[4];

	D3DMATRIX ToMatrixWithScale()
	{
		D3DMATRIX m;
		m._41 = translation.x;
		m._42 = translation.y;
		m._43 = translation.z;

		float x2 = rot.x + rot.x;
		float y2 = rot.y + rot.y;
		float z2 = rot.z + rot.z;

		float xx2 = rot.x * x2;
		float yy2 = rot.y * y2;
		float zz2 = rot.z * z2;
		m._11 = (1.0f - (yy2 + zz2)) * scale.x;
		m._22 = (1.0f - (xx2 + zz2)) * scale.y;
		m._33 = (1.0f - (xx2 + yy2)) * scale.z;

		float yz2 = rot.y * z2;
		float wx2 = rot.w * x2;
		m._32 = (yz2 - wx2) * scale.z;
		m._23 = (yz2 + wx2) * scale.y;

		float xy2 = rot.x * y2;
		float wz2 = rot.w * z2;
		m._21 = (xy2 - wz2) * scale.y;
		m._12 = (xy2 + wz2) * scale.x;

		float xz2 = rot.x * z2;
		float wy2 = rot.w * y2;
		m._31 = (xz2 + wy2) * scale.z;
		m._13 = (xz2 - wy2) * scale.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}
};

D3DMATRIX MatrixMultiplication(D3DMATRIX pM1, D3DMATRIX pM2)
{
	D3DMATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}
