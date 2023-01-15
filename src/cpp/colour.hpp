#ifndef WB_RT_COLOUR_HPP
#define WB_RT_COLOUR_HPP

#include "vec3.hpp"

extern unsigned char image[IMAGE_WIDTH][IMAGE_HEIGHT][BYTES_PER_PIXEL];

void WriteColour(int x, int y, int width, int height, Colour pixelColour);

#endif