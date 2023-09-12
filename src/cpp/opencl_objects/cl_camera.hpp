#ifndef WB_RT_CL_CAMERA_HPP
#define WB_RT_CL_CAMERA_HPP

#include "cl_vec3.hpp"
#include "../globals.hpp"

typedef struct
{
	CLVec3 origin;
	CLVec3 horizontal;
	CLVec3 vertical;
	CLVec3 lowerLeftCorner;
	CLVec3 defocusDiscU;
	CLVec3 defocusDiscV;
	cl_int width;
	cl_int height;
	cl_int samplesPerPixel;
	cl_int maxDepth;
	cl_float focusDistance;
	cl_float aperture;
} CLCamera;

void CalculateCamera(CLCamera *camera, CLVec3 lookFrom, CLVec3 lookAt, CLVec3 vUp, float vFov, float aspectRatio, float focusDistance, float aperture);

#endif