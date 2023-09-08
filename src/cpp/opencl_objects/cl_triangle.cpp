#include "cl_triangle.hpp"

CLTriangle CreateTriangle(CLVec3 p0, CLVec3 p1, CLVec3 p2, CLMaterial material, cl_int boundingBoxId)
{
	CLTriangle result;
	result.p0 = p0;
	result.p1 = p1;
	result.p2 = p2;
	result.material = material;
	result.boundingBoxId = boundingBoxId;
	return result;
}