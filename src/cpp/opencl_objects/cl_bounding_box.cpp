#include "cl_bounding_box.hpp"

cl_bounding_box CreateBoundingBox(cl_int id, cl_vec3 min, cl_vec3 max)
{
	cl_bounding_box result;
	result.id  = id;
	result.min = min;
	result.max = max;
	return result;
}