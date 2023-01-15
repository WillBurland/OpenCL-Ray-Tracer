#include <chrono>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>

#include "bitmap_io.hpp"
#include "colour.hpp"
#include "globals.hpp"
#include "vec3.hpp"

unsigned char image[IMAGE_WIDTH][IMAGE_HEIGHT][BYTES_PER_PIXEL];

int main()
{
	srand(time(NULL));

	char* imageFileName = (char*)"output.bmp";
	char* kernelFileName = (char*)"gpu_kernel.cl";

	printf("\n === System info ===\n");

	cl::Platform defaultPlatform = cl::Platform::getDefault();
	std::cout << "Using platform: " << defaultPlatform.getInfo<CL_PLATFORM_NAME>() << std::endl;

	cl::Device defaultDevice = cl::Device::getDefault();
	std::cout << "Using device: " << defaultDevice.getInfo<CL_DEVICE_NAME>() << std::endl;
	
	printf("Global work size: %d\n", IMAGE_WIDTH * IMAGE_HEIGHT);
	printf("Max work item dimensions: %d\n", defaultDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>());
	auto sizes = defaultDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
	printf("Max work item sizes: %d, %d, %d\n", sizes[0], sizes[1], sizes[2]);
	printf("Max work group size: %d\n", defaultDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());
	printf("Work groups to use: %d\n", (IMAGE_WIDTH * IMAGE_HEIGHT) / defaultDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());

	printf(" === Done ===\n");

	cl::Context context({ defaultDevice });

	std::chrono::steady_clock::time_point beginKernel = std::chrono::steady_clock::now();
	printf("\n === Compiling kernel ===\n");

	unsigned char *imageDataR = (unsigned char*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT);
	unsigned char *imageDataG = (unsigned char*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT);
	unsigned char *imageDataB = (unsigned char*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT);
	unsigned int *randomSeeds = (unsigned int*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned int));
	int *imageDataWidth = (int*)malloc(sizeof(int));
	int *imageDataHeight = (int*)malloc(sizeof(int));
	int *imageDataSamplesPerPixel = (int*)malloc(sizeof(int));
	int *imageDataMaxDepth = (int*)malloc(sizeof(int));

	imageDataWidth[0] = IMAGE_WIDTH;
	imageDataHeight[0] = IMAGE_HEIGHT;
	imageDataSamplesPerPixel[0] = SAMPLES_PER_PIXEL;
	imageDataMaxDepth[0] = MAX_DEPTH;

	printf("Generating random numbers...\n");
	for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
	{
		randomSeeds[i] = (rand() * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1) >> 16;
	}
	
	printf("Loading kernel .cl file...\n");
	std::ifstream kernelSourceFile(kernelFileName);
	std::string kernelSourceCode(std::istreambuf_iterator<char>(kernelSourceFile), (std::istreambuf_iterator<char>()));

	printf("Creating kernel program...\n");
	cl::Program::Sources sources;
	sources.push_back({kernelSourceCode.c_str(), kernelSourceCode.length()});

	printf("Building kernel...\n");
	cl::Program program(context, sources);
	if (program.build({defaultDevice}, "-cl-std=CL3.0") != CL_SUCCESS)
	{
		std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(defaultDevice) << std::endl;
		return 0;
	}

	printf("Allocating GPU memory...\n");
	cl::Buffer bufferR(context, CL_MEM_WRITE_ONLY, IMAGE_WIDTH * IMAGE_HEIGHT);
	cl::Buffer bufferG(context, CL_MEM_WRITE_ONLY, IMAGE_WIDTH * IMAGE_HEIGHT);
	cl::Buffer bufferB(context, CL_MEM_WRITE_ONLY, IMAGE_WIDTH * IMAGE_HEIGHT);
	cl::Buffer bufferRandomSeeds(context, CL_MEM_READ_ONLY, IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned int));
	cl::Buffer bufferWidth(context, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferHeight(context, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferSamplesPerPixel(context, CL_MEM_READ_ONLY, sizeof(int));
	cl::Buffer bufferMaxDepth(context, CL_MEM_READ_ONLY, sizeof(int));

	cl::CommandQueue queue(context, defaultDevice);

	printf("Writing to GPU memory...\n");
	queue.enqueueWriteBuffer(bufferR, CL_TRUE, 0, IMAGE_WIDTH * IMAGE_HEIGHT, imageDataR);
	queue.enqueueWriteBuffer(bufferG, CL_TRUE, 0, IMAGE_WIDTH * IMAGE_HEIGHT, imageDataG);
	queue.enqueueWriteBuffer(bufferB, CL_TRUE, 0, IMAGE_WIDTH * IMAGE_HEIGHT, imageDataB);
	queue.enqueueWriteBuffer(bufferRandomSeeds, CL_TRUE, 0, IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned int), randomSeeds);
	queue.enqueueWriteBuffer(bufferWidth, CL_TRUE, 0, sizeof(int), imageDataWidth);
	queue.enqueueWriteBuffer(bufferHeight, CL_TRUE, 0, sizeof(int), imageDataHeight);
	queue.enqueueWriteBuffer(bufferSamplesPerPixel, CL_TRUE, 0, sizeof(int), imageDataSamplesPerPixel);
	queue.enqueueWriteBuffer(bufferMaxDepth, CL_TRUE, 0, sizeof(int), imageDataMaxDepth);

	printf("Setting kernel arguments...\n");
	cl::Kernel kernel(program, "pixel_colour");
	cl::NDRange global(IMAGE_WIDTH * IMAGE_HEIGHT); // amount of pixels to process
	cl::NDRange local(defaultDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()); // max threads gpu can use

	kernel.setArg(0, bufferR);
	kernel.setArg(1, bufferG);
	kernel.setArg(2, bufferB);
	kernel.setArg(3, bufferRandomSeeds);
	kernel.setArg(4, bufferWidth);
	kernel.setArg(5, bufferHeight);
	kernel.setArg(6, bufferSamplesPerPixel);
	kernel.setArg(7, bufferMaxDepth);

	std::chrono::steady_clock::time_point endKernel = std::chrono::steady_clock::now();
	printf(" === Done in %f s ===\n\n", (float)std::chrono::duration_cast<std::chrono::microseconds>(endKernel- beginKernel).count() / 1000000);

	printf(" === Rendering ===\nGPU memory usage: %d MB\n", (
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned char) + // imageBufferR
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned char) + // imageBufferG
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned char) + // imageBufferB
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned int) + // randomSeeds
		sizeof(int) + // widthBuffer
		sizeof(int) + // heightBuffer
		sizeof(int) + // samplesPerPixelBuffer
		sizeof(int) +  // maxDepthBuffer
		336 * IMAGE_WIDTH * IMAGE_HEIGHT // internal kernel memory
	) / 1024 / 1024);
	
	printf("Running kernel...\n");
	std::chrono::steady_clock::time_point beginRender = std::chrono::steady_clock::now();
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
	
	queue.enqueueReadBuffer(bufferR, CL_TRUE, 0, IMAGE_WIDTH * IMAGE_HEIGHT, imageDataR);
	printf("Reading GPU memory...\n");
	queue.enqueueReadBuffer(bufferG, CL_TRUE, 0, IMAGE_WIDTH * IMAGE_HEIGHT, imageDataG);
	queue.enqueueReadBuffer(bufferB, CL_TRUE, 0, IMAGE_WIDTH * IMAGE_HEIGHT, imageDataB);

	printf("Cleaning up...\n");
	queue.flush();
	queue.finish();

	printf("Writing to file...\n");
	int rowsDone = 0;
	for (int i = 0; i < IMAGE_HEIGHT; i++)
	{
		for (int j = 0; j < IMAGE_WIDTH; j++)
		{
			Colour pixelColour(
				imageDataR[i * IMAGE_WIDTH + j],
				imageDataG[i * IMAGE_WIDTH + j],
				imageDataB[i * IMAGE_WIDTH + j]);
			WriteColour(i, j, IMAGE_WIDTH, IMAGE_HEIGHT, pixelColour);
		}
		
		rowsDone++;
	}

	generateBitmapImage((unsigned char*)image, IMAGE_HEIGHT, IMAGE_WIDTH, imageFileName);

	std::chrono::steady_clock::time_point endRender = std::chrono::steady_clock::now();
	printf(" === Done in %f s ===\n\n", (float)std::chrono::duration_cast<std::chrono::microseconds>(endRender - beginRender).count() / 1000000);

	return 0;
}