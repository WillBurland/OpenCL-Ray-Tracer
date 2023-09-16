#ifndef WB_RT_COLOUR_HPP
#define WB_RT_COLOUR_HPP

#include <cmath>

#include "opencl_objects/cl_vec3.hpp"

void WriteColour(int x, int y, int width, int height, cl_vec3 pixelColour);

#endif