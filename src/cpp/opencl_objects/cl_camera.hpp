#ifndef WB_RT_CL_CAMERA_HPP
#define WB_RT_CL_CAMERA_HPP

#include "cl_vec3.hpp"

struct cl_camera
{
	cl_vec3  origin;
	cl_vec3  horizontal;
	cl_vec3  vertical;
	cl_vec3  lowerLeftCorner;
	cl_vec3  defocusDiscU;
	cl_vec3  defocusDiscV;
	cl_int   width;
	cl_int   height;
	cl_int   samplesPerPixel;
	cl_int   maxDepth;
	cl_float focusDistance;
	cl_float aperture;
};

void CalculateCamera(cl_camera *camera, cl_vec3 lookFrom, cl_vec3 lookAt, cl_vec3 vUp, float vFov, float aspectRatio, float focusDistance, float aperture);

#endif