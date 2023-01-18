#ifndef WB_RT_CL_VEC3_HPP
#define WB_RT_CL_VEC3_HPP

#include "globals.hpp"

#include <CL/opencl.hpp>

#include <cmath>

typedef struct
{
	cl_float x, y, z;
} CLVec3;

typedef struct
{
	CLVec3 origin;
	CLVec3 horizontal;
	CLVec3 vertical;
	CLVec3 lowerLeftCorner;
} CLCamera;

CLVec3 Vec3SubVec3(CLVec3 a, CLVec3 b);
CLVec3 Vec3MulFloat(CLVec3 a, float b);
CLVec3 Vec3DivFloat(CLVec3 a, float b);
CLVec3 Vec3Cross(CLVec3 a, CLVec3 b);
float Vec3LengthSquared(CLVec3 a);
float Vec3Length(CLVec3 a);
CLVec3 Vec3Unit(CLVec3 a);
CLVec3 CreateVec3(float x, float y, float z);
void CalculateCamera(CLCamera *camera, CLVec3 lookFrom, CLVec3 lookAt, CLVec3 vUp, float vfov, float aspectRatio);

#endif