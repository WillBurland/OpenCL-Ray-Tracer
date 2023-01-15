#ifndef WB_RT_GLOBALS_HPP
#define WB_RT_GLOBALS_HPP

#include <cstdlib>
#include <limits>

#define ASPECT_RATIO 1.7777777 // 16:9
#define IMAGE_WIDTH 1280 // must be multiple of 128
#define IMAGE_HEIGHT (int)(IMAGE_WIDTH / ASPECT_RATIO)
#define BYTES_PER_PIXEL 3
#define SAMPLES_PER_PIXEL 250
#define MAX_DEPTH 50

#endif