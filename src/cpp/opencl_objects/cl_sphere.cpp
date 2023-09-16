#include "cl_sphere.hpp"

cl_sphere CreateSphere(cl_vec3 center, cl_float radius, cl_material material)
{
	cl_sphere result;
	result.center   = center;
	result.radius   = radius;
	result.material = material;
	return result;
}