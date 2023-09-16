#include "bitmap_io.hpp"
#include "colour.hpp"
#include "utility.hpp"
#include "opencl_objects/cl_bounding_box.hpp"
#include "opencl_objects/cl_camera.hpp"
#include "opencl_objects/cl_sphere.hpp"

unsigned char image[IMAGE_WIDTH][IMAGE_HEIGHT][BYTES_PER_PIXEL];

int main()
{	
	// seed random number generator with current time
	srand(time(NULL));

	// file name constants
	char* imageFileName = (char*)"output.bmp";
	char* kernelFileName = (char*)"gpu_kernel.cl";

	// opencl setup
	cl_int err;
	cl::Device defaultDevice = cl::Device::getDefault();
	cl::Platform defaultPlatform = cl::Platform::getDefault();
	cl::Context context({ defaultDevice });

	if (PrintOpenCLInfo() != 0) return -1;

	std::chrono::steady_clock::time_point beginKernel = std::chrono::steady_clock::now();
	printf("\n === Compiling kernel ===\n");

	// image data
	cl_vec3 *imageData = (cl_vec3*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(cl_vec3));

	// camera data
	cl_camera *camera = (cl_camera*)malloc(sizeof(cl_camera));
	CalculateCamera(
		camera,                        // camera
		cl_vec3{ -1.3f, 0.2f,  0.5f }, // position
		cl_vec3{  0.2f, 0.0f, -1.5f }, // look at
		cl_vec3{  0.0f, 1.0f,  0.0f }, // up (used for rotation)
		60.0f,                         // field of view
		ASPECT_RATIO,                  // aspect ratio
		2.0f,                          // focus distance
		1.2f                           // aperture
	);
	camera->width           = IMAGE_WIDTH;
	camera->height          = IMAGE_HEIGHT;
	camera->samplesPerPixel = SAMPLES_PER_PIXEL;
	camera->maxDepth        = MAX_DEPTH;

	// sphere data
	cl_int numSpheres = 8;
	cl_sphere *spheres = (cl_sphere*)malloc(numSpheres * sizeof(cl_sphere));
	spheres[0] = CreateSphere(CreateVec3( 0.0f, -100.5f, -1.0f), 100.0f, CreateMaterial(CreateVec3(0.3f, 0.5f, 0.4f), 0.0f, 0.0f, 0)); // ground
	spheres[1] = CreateSphere(CreateVec3( 1.6f,    0.0f, -1.3f),   0.5f, CreateMaterial(CreateVec3(0.7f, 0.3f, 0.9f), 0.0f, 0.0f, 0)); // purple diffuse
	spheres[2] = CreateSphere(CreateVec3(-0.5f,    0.0f, -2.0f),   0.5f, CreateMaterial(CreateVec3(0.8f, 0.5f, 0.5f), 0.2f, 0.0f, 1)); // fuzzy pink mirror
	spheres[3] = CreateSphere(CreateVec3( 0.6f,    0.1f, -1.9f),   0.6f, CreateMaterial(CreateVec3(0.8f, 0.8f, 0.8f), 0.0f, 0.0f, 1)); // perfect mirror
	spheres[4] = CreateSphere(CreateVec3( 0.2f,  -0.35f, -0.4f),  0.15f, CreateMaterial(CreateVec3(0.8f, 0.8f, 0.8f), 0.0f, 1.5f, 2)); // glass
	spheres[5] = CreateSphere(CreateVec3(-0.4f,   -0.4f, -0.6f),   0.1f, CreateMaterial(CreateVec3(1.0f, 1.0f, 1.0f), 0.0f, 0.0f, 3)); // light
	spheres[6] = CreateSphere(CreateVec3(-0.2f,   -0.1f,  0.6f),   0.4f, CreateMaterial(CreateVec3(0.5f, 0.5f, 0.8f), 0.0f, 0.0f, 1)); // foreground blue mirror
	spheres[7] = CreateSphere(CreateVec3(-1.5f,   -0.1f, -5.0f),   0.5f, CreateMaterial(CreateVec3(0.5f, 0.8f, 0.5f), 0.0f, 0.0f, 1)); // background green mirror

	printf("Generating triangles from mesh...\n");

	// mesh bounding box data
	cl_int numBoundingBoxes = 1;
	cl_bounding_box *boundingBoxes = (cl_bounding_box*)malloc(numBoundingBoxes * sizeof(cl_bounding_box));
	cl_vec3 min = CreateVec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	cl_vec3 max = CreateVec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

	// mesh data
	std::ifstream meshFile("f1_car.obj");
	cl_vec3 meshFileTranslate = CreateVec3(0.0f,  -0.5f,  -1.0f);
	cl_vec3 meshFileScale     = CreateVec3(0.15f,  0.15f,  0.15f);
	cl_int numVertices        = GetNumOfVertices(meshFile); meshFile.clear(); meshFile.seekg(0, std::ios::beg);
	cl_int numTriangles       = GetNumOfFaces(meshFile);    meshFile.clear(); meshFile.seekg(0, std::ios::beg);
	cl_vec3 *vertices         = (cl_vec3*)    malloc(numVertices  * sizeof(cl_vec3));
	cl_triangle *triangles    = (cl_triangle*)malloc(numTriangles * sizeof(cl_triangle));

	// read mesh data into vertices and triangles, and calculate bounding box
	ReadMeshData(meshFile, vertices, triangles, min, max, meshFileScale, meshFileTranslate);
	meshFile.close();
	boundingBoxes[0] = (cl_bounding_box){0, min, max};

	// hdr image data
	HDRLoaderResult result;
	if (HDRLoader::load("skybox.hdr", result))
	{
		printf("HDR image loaded, size: %d x %d\n", result.width, result.height);
	}
	else
	{
		printf("Error loading HDR image\n");
		return -1;
	}
	cl_int hdrImageDataWidth  = result.width;
	cl_int hdrImageDataHeight = result.height;
	cl_vec3 *hdrImageData = (cl_vec3*)malloc(result.width * result.height * sizeof(cl_vec3));
	ReadHdrImageData(hdrImageData, result);

	printf("Generating random numbers...\n");
	cl_uint *randomSeeds = (cl_uint*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(cl_uint));
	for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
	{
		randomSeeds[i] = (rand() * 0x5DEECE66DLL + 0xBLL) & ((1LL << 48) - 1) >> 16;
	}
	
	/*
		==========================================================
		Mostly OpenCL API calls to set and run kernel from here on
		==========================================================
	*/

	// kernel setup
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

	// allocate memory on GPU
	printf("Allocating GPU memory...\n");
	cl::Buffer bufferImageData         (context, CL_MEM_WRITE_ONLY, sizeof(cl_vec3) * IMAGE_WIDTH * IMAGE_HEIGHT);
	cl::Buffer bufferHDRImageData      (context, CL_MEM_READ_ONLY,  sizeof(cl_vec3) * result.width * result.height);
	cl::Buffer bufferHDRImageDataWidth (context, CL_MEM_READ_ONLY,  sizeof(cl_int));
	cl::Buffer bufferHDRImageDataHeight(context, CL_MEM_READ_ONLY,  sizeof(cl_int));
	cl::Buffer bufferRandomSeeds       (context, CL_MEM_READ_ONLY,  sizeof(cl_uint) * IMAGE_WIDTH * IMAGE_HEIGHT);
	cl::Buffer bufferCamera            (context, CL_MEM_READ_ONLY,  sizeof(cl_camera));
	cl::Buffer bufferSpheres           (context, CL_MEM_READ_ONLY,  sizeof(cl_sphere) * numSpheres);
	cl::Buffer bufferNumSpheres        (context, CL_MEM_READ_ONLY,  sizeof(cl_int));
	cl::Buffer bufferTriangles         (context, CL_MEM_READ_ONLY,  sizeof(cl_triangle) * numTriangles);
	cl::Buffer bufferNumTriangles      (context, CL_MEM_READ_ONLY,  sizeof(cl_int));
	cl::Buffer bufferBoundingBoxes     (context, CL_MEM_READ_ONLY,  sizeof(cl_bounding_box) * numBoundingBoxes);
	cl::Buffer bufferNumBoundingBoxes  (context, CL_MEM_READ_ONLY,  sizeof(cl_int));

	cl::CommandQueue queue(context, defaultDevice);

	// copy data to GPU
	printf("Writing to GPU memory...\n");
	queue.enqueueWriteBuffer(bufferImageData,          CL_TRUE, 0, sizeof(cl_vec3) * IMAGE_WIDTH * IMAGE_HEIGHT,   imageData);
	queue.enqueueWriteBuffer(bufferHDRImageData,       CL_TRUE, 0, sizeof(cl_vec3) * result.width * result.height, hdrImageData);
	queue.enqueueWriteBuffer(bufferHDRImageDataWidth,  CL_TRUE, 0, sizeof(cl_int),                                 &hdrImageDataWidth);
	queue.enqueueWriteBuffer(bufferHDRImageDataHeight, CL_TRUE, 0, sizeof(cl_int),                                 &hdrImageDataHeight);
	queue.enqueueWriteBuffer(bufferRandomSeeds,        CL_TRUE, 0, sizeof(cl_uint) * IMAGE_WIDTH * IMAGE_HEIGHT,   randomSeeds);
	queue.enqueueWriteBuffer(bufferCamera,             CL_TRUE, 0, sizeof(cl_camera),                              camera);
	queue.enqueueWriteBuffer(bufferSpheres,            CL_TRUE, 0, sizeof(cl_sphere) * numSpheres,                 spheres);
	queue.enqueueWriteBuffer(bufferNumSpheres,         CL_TRUE, 0, sizeof(cl_int),                                 &numSpheres);
	queue.enqueueWriteBuffer(bufferTriangles,          CL_TRUE, 0, sizeof(cl_triangle) * numTriangles,             triangles);
	queue.enqueueWriteBuffer(bufferNumTriangles,       CL_TRUE, 0, sizeof(cl_int),                                 &numTriangles);
	queue.enqueueWriteBuffer(bufferBoundingBoxes,      CL_TRUE, 0, sizeof(cl_bounding_box) * numBoundingBoxes,     boundingBoxes);
	queue.enqueueWriteBuffer(bufferNumBoundingBoxes,   CL_TRUE, 0, sizeof(cl_int),                                 &numBoundingBoxes);

	printf("Setting kernel arguments...\n");
	cl::Kernel kernel(program, "pixel_colour");     // kernel function name
	cl::NDRange global(IMAGE_WIDTH * IMAGE_HEIGHT); // amount of pixels to process

	size_t workGroupSize;
	err = kernel.getWorkGroupInfo(defaultDevice, CL_KERNEL_WORK_GROUP_SIZE, &workGroupSize);
	if (err != CL_SUCCESS)
	{
		std::cout << "Could not get work group size: " << err << std::endl;
		return -1;
	}

	cl::NDRange local(workGroupSize); // amount of pixels to process per work group

	// set kernel arguments
	kernel.setArg(0,  bufferImageData);
	kernel.setArg(1,  bufferHDRImageData);
	kernel.setArg(2,  bufferHDRImageDataWidth);
	kernel.setArg(3,  bufferHDRImageDataHeight);
	kernel.setArg(4,  bufferRandomSeeds);
	kernel.setArg(5,  bufferCamera);
	kernel.setArg(6,  bufferSpheres);
	kernel.setArg(7,  bufferNumSpheres);
	kernel.setArg(8,  bufferTriangles);
	kernel.setArg(9,  bufferNumTriangles);
	kernel.setArg(10, bufferBoundingBoxes);
	kernel.setArg(11, bufferNumBoundingBoxes);

	std::chrono::steady_clock::time_point endKernel = std::chrono::steady_clock::now();
	printf(" === Done in %f s ===\n", (float)std::chrono::duration_cast<std::chrono::microseconds>(endKernel- beginKernel).count() / 1000000);

	// output job info
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
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(cl_vec3)   + // imageDataBuffer
		result.width * result.height * sizeof(cl_vec3) + // hdrImageDataBuffer
		sizeof(int)                                    + // imageDataWidthBuffer
		sizeof(int)                                    + // imageDataHeightBuffer
		IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(cl_uint)   + // randomSeeds
		sizeof(cl_camera)                              + // cameraBuffer
		numSpheres * sizeof(cl_sphere)                 + // spheresBuffer
		sizeof(int)                                    + // numSpheresBuffer
		numTriangles * sizeof(cl_triangle)             + // trianglesBuffer
		sizeof(int)                                    + // numTrianglesBuffer
		numBoundingBoxes * sizeof(cl_bounding_box)     + // boundingBoxesBuffer
		sizeof(int)                                    + // numBoundingBoxesBuffer
		kernelMemoryUsage * (IMAGE_WIDTH * IMAGE_HEIGHT) / workGroupSize // kernel memory usage
	) / 1024);
	
	// run kernel
	printf("Running kernel...\n");
	std::chrono::steady_clock::time_point beginRender = std::chrono::steady_clock::now();
	err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
	if (err != CL_SUCCESS)
	{
		std::cout << "Could not run kernel: " << err << std::endl;
		return -1;
	}
	
	// read data from GPU
	queue.enqueueReadBuffer(bufferImageData, CL_TRUE, 0, IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(cl_vec3), imageData);
	printf("Reading GPU memory...\n");

	// clean up
	printf("Cleaning up...\n");
	queue.flush();
	queue.finish();

	// write image data to file
	printf("Writing to file...\n");
	for (int i = 0; i < IMAGE_HEIGHT; i++)
	{
		for (int j = 0; j < IMAGE_WIDTH; j++)
		{
			cl_vec3 pixelColour = (cl_vec3){
				imageData[i * IMAGE_WIDTH + j].x,
				imageData[i * IMAGE_WIDTH + j].y,
				imageData[i * IMAGE_WIDTH + j].z};
			WriteColour(i, j, IMAGE_WIDTH, IMAGE_HEIGHT, pixelColour);
		}
	}

	// write image to .bmp file
	generateBitmapImage((unsigned char*)image, IMAGE_HEIGHT, IMAGE_WIDTH, imageFileName);

	std::chrono::steady_clock::time_point endRender = std::chrono::steady_clock::now();
	printf(" === Done in %f s ===\n\n", (float)std::chrono::duration_cast<std::chrono::microseconds>(endRender - beginRender).count() / 1000000);

	return 0;
}