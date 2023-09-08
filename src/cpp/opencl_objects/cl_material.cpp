#include "cl_material.hpp"

CLMaterial CreateMaterial(CLVec3 albedo, cl_float fuzz, cl_float ior, cl_int type)
{
	CLMaterial result;
	result.albedo = albedo;
	result.fuzz = fuzz;
	result.ior = ior;
	result.type = type;
	return result;
}