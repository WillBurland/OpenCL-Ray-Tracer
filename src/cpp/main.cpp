#include <chrono>
#include <iostream>
#include <fstream>

#include "bitmap_io.hpp"
#include "cl_sphere.hpp"
#include "cl_vec3.hpp"
#include "colour.hpp"
#include "globals.hpp"
#include "vec3.hpp"

unsigned char image[IMAGE_WIDTH][IMAGE_HEIGHT][BYTES_PER_PIXEL];

int main()
{
	srand(time(NULL));

	char* imageFileName = (char*)"output.bmp";
	char* kernelFileName = (char*)"gpu_kernel.cl";

	cl_int err;

	printf("\n === Device list ===\n");

	std::vector<cl::Device> devices;
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices_available;
    int n = 0;
    cl::Platform::get(&platforms);
    for (int i = 0; i < (int)platforms.size(); i++)
	{
        devices_available.clear();
        platforms[i].getDevices(CL_DEVICE_TYPE_ALL, &devices_available);
        if(devices_available.size() == 0)
			continue;
        for (int j = 0; j < (int)devices_available.size(); j++)
		{
            n++;
            devices.push_back(devices_available[j]);
        }
    }

    if (platforms.size() ==  0|| devices.size() == 0)
	{
        std::cout << "Error: There are no OpenCL devices available" << std::endl;
        return -1;
    }

    for (int i = 0; i < n; i++)
	{
		printf("ID: %d, Device: %s\n", i, devices[i].getInfo<CL_DEVICE_NAME>().c_str());
	}

	cl::Platform defaultPlatform = cl::Platform::getDefault();
	printf("\nUsing platform: %s\n", defaultPlatform.getInfo<CL_PLATFORM_NAME>().c_str());

	cl::Device defaultDevice = cl::Device::getDefault();
	printf("Using device: %s\n", defaultDevice.getInfo<CL_DEVICE_NAME>().c_str());

	printf("OpenCL support: %s\n", defaultDevice.getInfo<CL_DEVICE_OPENCL_C_VERSION>().c_str());

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

	CLCamera *camera = (CLCamera*)malloc(sizeof(CLCamera));
	CalculateCamera(
		camera,
		CLVec3{ -2.5f, 1.5f,  3.5f },
		CLVec3{  0.2f, 0.0f, -1.5f },
		CLVec3{  0.0f, 1.0f,  0.0f },
		20.0f,
		ASPECT_RATIO
	);

	cl_int numSpheres = 7;
	CLSphere *spheres = (CLSphere*)malloc(numSpheres * sizeof(CLSphere));
	spheres[0] = CreateSphere(CreateVec3( 0.0f, -100.5f, -1.0f), 100.0f, CreateMaterial(CreateVec3(0.0f, 0.8f, 0.7f), 0.0f, 0.0f, 0));
	spheres[1] = CreateSphere(CreateVec3( 0.0f,    0.5f, -1.0f),   0.5f, CreateMaterial(CreateVec3(0.7f, 0.3f, 0.9f), 0.0f, 0.0f, 0));
	spheres[2] = CreateSphere(CreateVec3(-0.9f,    0.0f, -1.0f),   0.5f, CreateMaterial(CreateVec3(0.8f, 0.5f, 0.5f), 0.1f, 0.0f, 1));
	spheres[3] = CreateSphere(CreateVec3( 0.9f,    0.0f, -1.0f),   0.5f, CreateMaterial(CreateVec3(0.8f, 0.6f, 0.2f), 0.5f, 0.0f, 1));
	spheres[4] = CreateSphere(CreateVec3( 0.0f,   -0.3f, -1.0f),   0.2f, CreateMaterial(CreateVec3(0.8f, 0.8f, 0.8f), 0.0f, 0.0f, 1));
	spheres[5] = CreateSphere(CreateVec3( 0.2f,   -0.4f, -0.8f),   0.1f, CreateMaterial(CreateVec3(0.8f, 0.8f, 0.8f), 0.0f, 1.5f, 2));
	spheres[6] = CreateSphere(CreateVec3(-0.2f,   -0.4f, -0.8f),   0.1f, CreateMaterial(CreateVec3(0.8f, 0.8f, 0.8f), 0.0f, 1.5f, 2));

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
	if (program.build({defaultDevice}) != CL_SUCCESS)
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
	cl::Buffer bufferCamera(context, CL_MEM_READ_ONLY, sizeof(CLCamera));
	cl::Buffer bufferSpheres(context, CL_MEM_READ_ONLY, numSpheres * sizeof(CLSphere));
	cl::Buffer bufferNumSpheres(context, CL_MEM_READ_ONLY, sizeof(int));

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
	queue.enqueueWriteBuffer(bufferCamera, CL_TRUE, 0, sizeof(CLCamera), camera);
	queue.enqueueWriteBuffer(bufferSpheres, CL_TRUE, 0, numSpheres * sizeof(CLSphere), spheres);
	queue.enqueueWriteBuffer(bufferNumSpheres, CL_TRUE, 0, sizeof(int), &numSpheres);

	printf("Setting kernel arguments...\n");
	cl::Kernel kernel(program, "pixel_colour");
	cl::NDRange global(IMAGE_WIDTH * IMAGE_HEIGHT); // amount of pixels to process

	size_t workGroupSize;
	err = kernel.getWorkGroupInfo(defaultDevice, CL_KERNEL_WORK_GROUP_SIZE, &workGroupSize);
	if (err != CL_SUCCESS)
	{
		std::cout << "Could not get work group size: " << err << std::endl;
		return -1;
	}

	cl::NDRange local(workGroupSize); // amount of pixels to process per work group

	kernel.setArg(0, bufferR);
	kernel.setArg(1, bufferG);
	kernel.setArg(2, bufferB);
	kernel.setArg(3, bufferRandomSeeds);
	kernel.setArg(4, bufferWidth);
	kernel.setArg(5, bufferHeight);
	kernel.setArg(6, bufferSamplesPerPixel);
	kernel.setArg(7, bufferMaxDepth);
	kernel.setArg(8, bufferCamera);
	kernel.setArg(9, bufferSpheres);
	kernel.setArg(10, bufferNumSpheres);

	std::chrono::steady_clock::time_point endKernel = std::chrono::steady_clock::now();
	printf(" === Done in %f s ===\n", (float)std::chrono::duration_cast<std::chrono::microseconds>(endKernel- beginKernel).count() / 1000000);

	printf("\n === Job info ===\n");
	printf("Global work size: %d\n", IMAGE_WIDTH * IMAGE_HEIGHT);
	printf("Work group size: %d\n", workGroupSize);
	printf("Work groups to use: %d\n", (IMAGE_WIDTH * IMAGE_HEIGHT) / workGroupSize);
	printf(" === Done ===\n\n");

	// output kernel memory usage
	cl_ulong kernelMemoryUsage = 0;
	err = kernel.getWorkGroupInfo(defaultDevice, CL_KERNEL_PRIVATE_MEM_SIZE, &kernelMemoryUsage);
	if (err != CL_SUCCESS)
	{
		std::cout << "Could not get kernel memory usage: " << err << std::endl;
		return -1;
	}

	printf(" === Rendering ===\nGPU memory usage: %d KB\n", (
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned char) + // imageBufferR
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned char) + // imageBufferG
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned char) + // imageBufferB
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(unsigned int) + // randomSeeds
		sizeof(int) + // widthBuffer
		sizeof(int) + // heightBuffer
		sizeof(int) + // samplesPerPixelBuffer
		sizeof(int) + // maxDepthBuffer
		sizeof(CLCamera) + // cameraBuffer
		numSpheres * sizeof(CLSphere) + // spheresBuffer
		sizeof(int) + // numSpheresBuffer
		kernelMemoryUsage * (IMAGE_WIDTH * IMAGE_HEIGHT) / workGroupSize // kernel memory usage
	) / 1024);
	
	printf("Running kernel...\n");
	std::chrono::steady_clock::time_point beginRender = std::chrono::steady_clock::now();
	err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
	if (err != CL_SUCCESS)
	{
		std::cout << "Could not run kernel: " << err << std::endl;
		return -1;
	}
	
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