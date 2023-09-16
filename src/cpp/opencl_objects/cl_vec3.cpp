#include "cl_vec3.hpp"

cl_vec3 CreateVec3(float x, float y, float z)
{
	cl_vec3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

cl_vec3 Vec3Cross(cl_vec3 a, cl_vec3 b)
{
	cl_vec3 result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

float Vec3LengthSquared(cl_vec3 a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

float Vec3Length(cl_vec3 a)
{
	return sqrt(Vec3LengthSquared(a));
}


cl_vec3 Vec3Unit(cl_vec3 a)
{
	return a / Vec3Length(a);
}

cl_vec3 operator+(cl_vec3 a, cl_vec3 b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

cl_vec3 operator-(cl_vec3 a, cl_vec3 b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

cl_vec3 operator*(cl_vec3 a, float b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
	return a;
}

cl_vec3 operator/(cl_vec3 a, float b)
{
	a.x /= b;
	a.y /= b;
	a.z /= b;
	return a;
}