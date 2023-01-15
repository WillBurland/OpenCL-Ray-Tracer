#ifndef WB_RT_BMP_IO_HPP
#define WB_RT_BMP_IO_HPP

#include <stdio.h>

#include "globals.hpp"

const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

#endif