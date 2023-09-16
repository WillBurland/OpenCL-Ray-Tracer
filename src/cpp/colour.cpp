#include "colour.hpp"

extern unsigned char image[IMAGE_HEIGHT][IMAGE_WIDTH][BYTES_PER_PIXEL];

void WriteColour(int x, int y, int width, int height, cl_vec3 pixelColour)
{
	// gamma correct for gamma=2.0
	float r = sqrt(pixelColour.x) * 255.0f;
	float g = sqrt(pixelColour.y) * 255.0f;
	float b = sqrt(pixelColour.z) * 255.0f;

	image[x][y][2] = (unsigned char)r;
	image[x][y][1] = (unsigned char)g;
	image[x][y][0] = (unsigned char)b;
}