#include "cl_bounding_box.hpp"

CLBoundingBox CreateBoundingBox(cl_int id, CLVec3 min, CLVec3 max)
{
	CLBoundingBox result;
	result.id = id;
	result.min = min;
	result.max = max;
	return result;
}