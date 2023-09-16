#ifndef WB_RT_CL_SPHERE_HPP
#define WB_RT_CL_SPHERE_HPP

#include "cl_material.hpp"
#include "cl_vec3.hpp"

struct cl_sphere
{
	cl_vec3     center;
	cl_float    radius;
	cl_material material;
};

cl_sphere CreateSphere(cl_vec3 center, cl_float radius, cl_material material);

#endif