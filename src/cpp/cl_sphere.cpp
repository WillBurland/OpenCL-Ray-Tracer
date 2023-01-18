#include "cl_sphere.hpp"

CLSphere CreateSphere(CLVec3 center, cl_double radius, CLMaterial material)
{
	CLSphere result;
	result.center = center;
	result.radius = radius;
	result.material = material;
	return result;
}

CLMaterial CreateMaterial(CLVec3 albedo, cl_float fuzz, cl_float ior, cl_int type)
{
	CLMaterial result;
	result.albedo = albedo;
	result.fuzz = fuzz;
	result.ior = ior;
	result.type = type;
	return result;
}