#include "cl_sphere.hpp"

CLSphere CreateSphere(CLVec3 center, cl_double radius, CLMaterial material)
{
	CLSphere result;
	result.center = center;
	result.radius = radius;
	result.material = material;
	return result;
}