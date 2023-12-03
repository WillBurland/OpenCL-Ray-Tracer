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
	char* imageFileName = (char*)"output";
	char* imageFileExtension = (char*)".bmp";
	char* kernelFileName = (char*)"gpu_kernel.cl";

	// opencl setup
	cl_int err;
	cl::Device defaultDevice = cl::Device::getDefault();
	cl::Platform defaultPlatform = cl::Platform::getDefault();
	cl::Context context({ defaultDevice });

	if (PrintOpenCLInfo() != 0) return -1;

	std::chrono::steady_clock::time_point beginKernel = std::chrono::steady_clock::now();
	printf("\n === Compiling kernel ===\n");

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

	cl::Kernel kernel(program, "pixel_colour"); // kernel function name

	printf("Calculating block size...\n");
	// get work group size
	size_t workGroupSize;
	err = kernel.getWorkGroupInfo(defaultDevice, CL_KERNEL_WORK_GROUP_SIZE, &workGroupSize);
	if (err != CL_SUCCESS)
	{
		std::cout << "Could not get work group size: " << err << std::endl;
		return -1;
	}

	// get ideal block size, based on target block number
	printf("Blocks to use: %d\n", TARGET_BLOCK_NUM);
	cl_vec3 blockSize = GetIdealBlockSize(IMAGE_WIDTH * IMAGE_HEIGHT, TARGET_BLOCK_NUM);
	size_t newSize = 0;
	for (int i = workGroupSize; i > 0; i--)
	{
		if ((int)(blockSize.x * blockSize.y) % i == 0)
		{
			newSize = i;
			break;
		}
	}
	workGroupSize = newSize;

	cl::NDRange local(workGroupSize); // amount of pixels to process per work group
	cl::NDRange global(blockSize.x * blockSize.y); // amount of pixels in each block

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
	camera->blockSize = blockSize;

	// image block data
	cl_vec3 *imageBlockData = (cl_vec3*)malloc(blockSize.x * blockSize.y * sizeof(cl_vec3));

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

	// allocate memory on GPU
	printf("Allocating GPU memory...\n");
	cl::Buffer bufferImageBlockData    (context, CL_MEM_WRITE_ONLY, sizeof(cl_vec3) * blockSize.x * blockSize.y);
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
	queue.enqueueWriteBuffer(bufferImageBlockData,     CL_TRUE, 0, sizeof(cl_vec3) * blockSize.x * blockSize.y,    imageBlockData);
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

	// set kernel arguments
	printf("Setting kernel arguments...\n");
	kernel.setArg(0,  bufferImageBlockData);
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
	printf("Global work size: %d\n", (int)global[0] * (int)global[1]);
	printf("Work group size: %d\n", workGroupSize);
	printf("Work groups to use: %d\n", (int)global[0] * (int)global[1] / workGroupSize);
	printf(" === Done ===\n\n");
	
	// start kernel timer
	printf("Running kernel...\n");
	std::chrono::steady_clock::time_point beginRender = std::chrono::steady_clock::now();
	
	// calculate block numbers
	int blockNumX = IMAGE_WIDTH / blockSize.x;
	int blockNumY = IMAGE_HEIGHT / blockSize.y;
	printf("Blocks to render: %d (%d x %d)\n", blockNumX * blockNumY, blockNumX, blockNumY);
	printf("Block size: %d x %d\n", (int)blockSize.x, (int)blockSize.y);

	// render blocks
	int blockNum = 0;
	for (int y = blockNumY - 1; y >= 0; y--)
	{
		for (int x = 0; x < blockNumX; x++)
		{
			// update camera (block offset)
			camera->blockOffset = CreateVec3(x, y, 0);
			cl::Buffer bufferCamera (context, CL_MEM_READ_ONLY,  sizeof(cl_camera));
			queue.enqueueWriteBuffer(bufferCamera, CL_TRUE, 0, sizeof(cl_camera), camera);
			kernel.setArg(5,  bufferCamera);

			// run kernel for block
			err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
			if (err != CL_SUCCESS)
			{
				std::cout << "Could not run kernel: " << err << std::endl;
				return -1;
			}
			// read image data back
			queue.enqueueReadBuffer(bufferImageBlockData, CL_TRUE, 0, sizeof(cl_vec3) * blockSize.x * blockSize.y, imageBlockData, nullptr);

			// write image data to file
			for (int i = 0; i < blockSize.y; i++)
			{
				for (int j = 0; j < blockSize.x; j++)
				{
					cl_vec3 pixelColour = (cl_vec3){
						imageBlockData[i * (int)blockSize.x + j].x,
						imageBlockData[i * (int)blockSize.x + j].y,
						imageBlockData[i * (int)blockSize.x + j].z};
					WriteColour(i + (int)blockSize.y * y, j + (int)blockSize.x * x, pixelColour);
				}
			}
			
			printf("Blocks rendered: %d%%\r", (int)((float)blockNum / (float)(blockNumX * blockNumY) * 100.0f));
		}
	}

	// clean up
	printf("\nCleaning up...\n");
	queue.flush();
	queue.finish();

	// write image to .bmp file
	printf("Writing image to file...\n");
	char* fileName = (char*)malloc(256);
	sprintf(fileName, "%s%s", imageFileName, imageFileExtension);
	generateBitmapImage((unsigned char*)image, IMAGE_HEIGHT, IMAGE_WIDTH, fileName);

	std::chrono::steady_clock::time_point endRender = std::chrono::steady_clock::now();
	printf(" === Done in %f s ===\n\n", (float)std::chrono::duration_cast<std::chrono::microseconds>(endRender - beginRender).count() / 1000000);

	return 0;
}