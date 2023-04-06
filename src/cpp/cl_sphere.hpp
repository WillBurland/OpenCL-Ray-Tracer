#ifndef WB_RT_CL_SPHERE_HPP
#define WB_RT_CL_SPHERE_HPP

#include "cl_material.hpp"
#include "cl_vec3.hpp"
#include "globals.hpp"

typedef struct
{
	CLVec3 center;
	cl_double radius;
	CLMaterial material;
} CLSphere;

CLSphere CreateSphere(CLVec3 center, cl_double radius, CLMaterial material);

#endif