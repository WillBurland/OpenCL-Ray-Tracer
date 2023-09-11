#ifndef WB_RT_COLOUR_HPP
#define WB_RT_COLOUR_HPP

#include <cmath>
#include <stdio.h>

#include "colour.hpp"
#include "globals.hpp"
#include "vec3.hpp"

extern unsigned char image[IMAGE_HEIGHT][IMAGE_WIDTH][BYTES_PER_PIXEL];

void WriteColour(int x, int y, int width, int height, Colour pixelColour)
{
	float r = sqrt(pixelColour.x * 255.0f);
	float g = sqrt(pixelColour.y * 255.0f);
	float b = sqrt(pixelColour.z * 255.0f);

	image[x][y][2] = (unsigned char)r;
	image[x][y][1] = (unsigned char)g;
	image[x][y][0] = (unsigned char)b;
}

#endif