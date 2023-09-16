#ifndef WB_RT_CL_MATERIAL_HPP
#define WB_RT_CL_MATERIAL_HPP

#include "cl_vec3.hpp"

struct cl_material
{
	cl_vec3  albedo;
	cl_float fuzz;
	cl_float ior;
	cl_int   type;
};

cl_material CreateMaterial(cl_vec3 albedo, cl_float fuzz, cl_float ior, cl_int type);

#endif