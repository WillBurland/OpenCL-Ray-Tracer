#ifndef WB_RT_CL_MATERIAL_HPP
#define WB_RT_CL_MATERIAL_HPP

#include "cl_vec3.hpp"
#include "globals.hpp"

typedef struct
{
	CLVec3 albedo;
	cl_float fuzz;
	cl_float ior;
	cl_int type;
} CLMaterial;

CLMaterial CreateMaterial(CLVec3 albedo, cl_float fuzz, cl_float ior, cl_int type);

#endif