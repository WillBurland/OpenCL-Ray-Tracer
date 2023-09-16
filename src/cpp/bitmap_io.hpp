#ifndef WB_RT_BMP_IO_HPP
#define WB_RT_BMP_IO_HPP

#include "globals.hpp"

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride, int FILE_HEADER_SIZE, int INFO_HEADER_SIZE);
unsigned char* createBitmapInfoHeader(int height, int width, int INFO_HEADER_SIZE);

#endif