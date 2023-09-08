#ifndef WB_RT_CL_BOUNDING_BOX_HPP
#define WB_RT_CL_BOUNDING_BOX_HPP

#include "cl_vec3.hpp"
#include "../globals.hpp"

typedef struct
{
	cl_int id;
	CLVec3 min;
	CLVec3 max;
} CLBoundingBox;

CLBoundingBox CreateBoundingBox(cl_int id, CLVec3 min, CLVec3 max);

#endif