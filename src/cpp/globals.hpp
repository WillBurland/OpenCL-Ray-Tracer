#ifndef WB_RT_GLOBALS_HPP
#define WB_RT_GLOBALS_HPP

#define CL_HPP_TARGET_OPENCL_VERSION 300

#include <CL/opencl.hpp>

#define ASPECT_RATIO      1.7777777 // 16:9
#define IMAGE_WIDTH       1280 // must be divisible by 64
#define IMAGE_HEIGHT      (int)(IMAGE_WIDTH / ASPECT_RATIO)
#define TARGET_BLOCK_NUM  4
#define BYTES_PER_PIXEL   3
#define SAMPLES_PER_PIXEL 5
#define MAX_DEPTH         32

#endif