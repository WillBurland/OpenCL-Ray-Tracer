#include "cl_vec3.hpp"

CLVec3 Vec3SubVec3(CLVec3 a, CLVec3 b)
{
	CLVec3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

CLVec3 Vec3MulFloat(CLVec3 a, float b)
{
	CLVec3 result;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}

CLVec3 Vec3DivFloat(CLVec3 a, float b)
{
	CLVec3 result;
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	return result;
}

CLVec3 Vec3Cross(CLVec3 a, CLVec3 b)
{
	CLVec3 result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

float Vec3LengthSquared(CLVec3 a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

float Vec3Length(CLVec3 a)
{
	return sqrt(Vec3LengthSquared(a));
}

CLVec3 Vec3Unit(CLVec3 a)
{
	return Vec3DivFloat(a, Vec3Length(a));
}

CLVec3 CreateVec3(float x, float y, float z)
{
	CLVec3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

void CalculateCamera(CLCamera *camera, CLVec3 lookFrom, CLVec3 lookAt, CLVec3 vUp, float vfov, float aspectRatio)
{
	float theta = vfov * (float)3.141592654 / 180.0f;
	float h = tan(theta / 2);
	float viewportHeight = 2.0f * h;
	float viewportWidth = aspectRatio * viewportHeight;

	CLVec3 w = Vec3Unit(Vec3SubVec3(lookFrom, lookAt));
	CLVec3 u = Vec3Unit(Vec3Cross(vUp, w));
	CLVec3 v = Vec3Cross(w, u);

	camera->origin = lookFrom;
	camera->horizontal = Vec3MulFloat(u, viewportWidth);
	camera->vertical = Vec3MulFloat(v, viewportHeight);
	camera->lowerLeftCorner = Vec3SubVec3(Vec3SubVec3(Vec3SubVec3(camera->origin, Vec3DivFloat(camera->horizontal, 2)), Vec3DivFloat(camera->vertical, 2)), w);
}