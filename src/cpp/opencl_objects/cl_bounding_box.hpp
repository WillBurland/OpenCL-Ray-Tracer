#ifndef WB_RT_CL_BOUNDING_BOX_HPP
#define WB_RT_CL_BOUNDING_BOX_HPP

#include "cl_vec3.hpp"

struct cl_bounding_box
{
	cl_int  id;
	cl_vec3 min, max;
};

cl_bounding_box CreateBoundingBox(cl_int id, cl_vec3 min, cl_vec3 max);

#endif