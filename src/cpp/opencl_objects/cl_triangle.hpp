#ifndef WB_RT_CL_TRIANGLE_HPP
#define WB_RT_CL_TRIANGLE_HPP

#include "cl_material.hpp"
#include "cl_vec3.hpp"

struct cl_triangle
{
	cl_vec3     p0, p1, p2;
	cl_material material;
	cl_int      boundingBoxId;
};

cl_triangle CreateTriangle(cl_vec3 p0, cl_vec3 p1, cl_vec3 p2, cl_material material, cl_int boundingBoxId);

#endif