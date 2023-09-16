#include "cl_triangle.hpp"

cl_triangle CreateTriangle(cl_vec3 p0, cl_vec3 p1, cl_vec3 p2, cl_material material, cl_int boundingBoxId)
{
	cl_triangle result;
	result.p0              = p0;
	result.p1              = p1;
	result.p2              = p2;
	result.material        = material;
	result.boundingBoxId   = boundingBoxId;
	return result;
}