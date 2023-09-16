#ifndef WB_RT_CL_VEC3_HPP
#define WB_RT_CL_VEC3_HPP

#include <cmath>

#include "../globals.hpp"

struct cl_vec3
{
	cl_float x, y, z;
};

cl_vec3 CreateVec3(float x, float y, float z);
cl_vec3 Vec3Cross(cl_vec3 a, cl_vec3 b);
float Vec3LengthSquared(cl_vec3 a);
float Vec3Length(cl_vec3 a);
cl_vec3 Vec3Unit(cl_vec3 a);
cl_vec3 operator+(cl_vec3 a, cl_vec3 b);
cl_vec3 operator-(cl_vec3 a, cl_vec3 b);
cl_vec3 operator*(cl_vec3 a, float b);
cl_vec3 operator/(cl_vec3 a, float b);

#endif