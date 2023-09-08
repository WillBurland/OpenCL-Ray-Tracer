#ifndef WB_RT_CL_TRIANGLE_HPP
#define WB_RT_CL_TRIANGLE_HPP

#include "cl_material.hpp"
#include "cl_vec3.hpp"
#include "../globals.hpp"

typedef struct
{
	CLVec3 p0, p1, p2;
	CLMaterial material;
	cl_int boundingBoxId;
} CLTriangle;

CLTriangle CreateTriangle(CLVec3 p0, CLVec3 p1, CLVec3 p2, CLMaterial material, cl_int boundingBoxId);

#endif