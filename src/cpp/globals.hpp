#ifndef WB_RT_GLOBALS_HPP
#define WB_RT_GLOBALS_HPP

#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <cstdlib>
#include <limits>

#define ASPECT_RATIO 1.7777777 // 16:9
#define IMAGE_WIDTH 1600 // must be divisible by 64
#define IMAGE_HEIGHT (int)(IMAGE_WIDTH / ASPECT_RATIO)
#define BYTES_PER_PIXEL 3
#define SAMPLES_PER_PIXEL 64
#define MAX_DEPTH 16

#endif