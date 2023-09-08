#ifndef WB_RT_CL_SPHERE_HPP
#define WB_RT_CL_SPHERE_HPP

#include "cl_material.hpp"
#include "cl_vec3.hpp"
#include "../globals.hpp"

typedef struct
{
	CLVec3 center;
	cl_float radius;
	CLMaterial material;
	CLVec3 boundingBoxMin;
	CLVec3 boundingBoxMax;
} CLSphere;

CLSphere CreateSphere(CLVec3 center, cl_float radius, CLMaterial material);

#endif