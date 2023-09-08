#include "cl_sphere.hpp"

CLSphere CreateSphere(CLVec3 center, cl_float radius, CLMaterial material)
{
	CLSphere result;
	result.center = center;
	result.radius = radius;
	result.material = material;
	result.boundingBoxMin = (CLVec3){center.x - radius, center.y - radius, center.z - radius};
	result.boundingBoxMax = (CLVec3){center.x + radius, center.y + radius, center.z + radius};
	return result;
}