#include "cl_material.hpp"

cl_material CreateMaterial(cl_vec3 albedo, cl_float fuzz, cl_float ior, cl_int type)
{
	cl_material result;
	result.albedo = albedo;
	result.fuzz   = fuzz;
	result.ior    = ior;
	result.type   = type;
	return result;
}