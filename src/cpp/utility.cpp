#include "utility.hpp"

int PrintOpenCLInfo()
{
	printf("\n === Device information ===\n");
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
	cl::Device defaultDevice = cl::Device::getDefault();

	printf("\nUsing platform: %s\n",          defaultPlatform.getInfo<CL_PLATFORM_NAME>().c_str());
	printf("Using device: %s\n",              defaultDevice.getInfo<CL_DEVICE_NAME>().c_str());
	printf("OpenCL support: %s\n",            defaultDevice.getInfo<CL_DEVICE_OPENCL_C_VERSION>().c_str());
	printf("Max compute units: %u\n",         defaultDevice.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>());
	printf("Max work group size: %d\n",       defaultDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());
	printf("Max work item dimensions: %u\n",  defaultDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>());
	printf("Max work item sizes: %d %d %d\n", defaultDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[0], defaultDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[1], defaultDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>()[2]);
	printf("Max clock frequency: %u\n",       defaultDevice.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>());
	printf("Max mem alloc size: %lu\n",       defaultDevice.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>());
	printf("Global mem size: %lu\n",          defaultDevice.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>());
	printf("Local mem size: %lu\n",           defaultDevice.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>());
	printf("Max constant buffer size: %lu\n", defaultDevice.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>());
	printf("Max constant args: %u\n",         defaultDevice.getInfo<CL_DEVICE_MAX_CONSTANT_ARGS>());
	printf("Max parameter size: %u\n",        defaultDevice.getInfo<CL_DEVICE_MAX_PARAMETER_SIZE>());
	printf("Max samplers: %u\n",              defaultDevice.getInfo<CL_DEVICE_MAX_SAMPLERS>());
	printf("Max read image args: %u\n",       defaultDevice.getInfo<CL_DEVICE_MAX_READ_IMAGE_ARGS>());
	printf("Max write image args: %u\n",      defaultDevice.getInfo<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>());
	printf("Max image2d size: %d %d\n",       defaultDevice.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>(), defaultDevice.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>());
	printf("Max image3d size: %d %d %d\n",    defaultDevice.getInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>(), defaultDevice.getInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>(), defaultDevice.getInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>());

	printf(" === Done ===\n");

	return 0;
}

int GetNumOfVertices(std::ifstream &meshFile)
{
	// count rows starting with "v" to get number of vertices
	std::string line;
	int numVertices = 0;
	while (std::getline(meshFile, line))
	{
		std::istringstream iss(line);
		std::string type;
		iss >> type;
		if (type == "v")
		{
			numVertices++;
		}
	}
	return numVertices;
}

int GetNumOfFaces(std::ifstream &meshFile)
{
	// count rows starting with "f" to get number of faces (triangles)
	std::string line;
	int numFaces = 0;
	while (std::getline(meshFile, line))
	{
		std::istringstream iss(line);
		std::string type;
		iss >> type;
		if (type == "f")
		{
			numFaces++;
		}
	}
	return numFaces;
}

void ReadMeshData(std::ifstream &meshFile, cl_vec3 *vertices, cl_triangle *triangles, cl_vec3 &min, cl_vec3 &max, cl_vec3 meshFileScale, cl_vec3 meshFileTranslate)
{
	// read vertices and faces from mesh file, scale and translate vertices
	int currentVertex = 0;
	int currentFace = 0;
	std::string line;
	while (std::getline(meshFile, line))
	{
		std::istringstream iss(line);
		std::string type;
		iss >> type;

		if (type == "v")
		{
			float x, y, z;
			iss >> x >> y >> z;
			vertices[currentVertex] = CreateVec3(
				x * meshFileScale.x + meshFileTranslate.x,
				y * meshFileScale.y + meshFileTranslate.y,
				z * meshFileScale.z + meshFileTranslate.z
			);

			if (vertices[currentVertex].x < min.x) min.x = vertices[currentVertex].x;
			if (vertices[currentVertex].y < min.y) min.y = vertices[currentVertex].y;
			if (vertices[currentVertex].z < min.z) min.z = vertices[currentVertex].z;
			
			if (vertices[currentVertex].x > max.x) max.x = vertices[currentVertex].x;
			if (vertices[currentVertex].y > max.y)max.y = vertices[currentVertex].y;
			if (vertices[currentVertex].z > max.z) max.z = vertices[currentVertex].z;

			currentVertex++;
		}
		else if (type == "f")
		{
			int a, b, c;
			std::string faceLine = line.c_str();

			faceLine.erase(0, 2);
			std::replace(faceLine.begin(), faceLine.end(), ' ', '/');

			std::vector<std::string> tokens;
			std::string token;
			std::istringstream tokenStream(faceLine);
			while (std::getline(tokenStream, token, '/'))
			{
				tokens.push_back(token);
			}

			int start = 1;
			int change = 4;
			if (tokens.size() == 9)
			{
				start = 0;
				change = 3;
			}

			a = std::stoi(tokens[start + change * 0]);
			b = std::stoi(tokens[start + change * 1]);
			c = std::stoi(tokens[start + change * 2]);

			triangles[currentFace] = CreateTriangle(
				vertices[a - 1],
				vertices[b - 1],
				vertices[c - 1],
				CreateMaterial(CreateVec3(0.8f, 0.6f, 0.2f), 0.5f, 0.0f, 1),
				0
			);

			currentFace++;
		}
	}
}

void ReadHdrImageData(cl_vec3 *hdrImageData, HDRLoaderResult &result)
{
	for (int i = 0; i < result.width * result.height; i++)
	{
		float r = result.cols[i * 3 + 0];
		float g = result.cols[i * 3 + 1];
		float b = result.cols[i * 3 + 2];

		// arbitrary gamma correction, kinda just made this up, looks good enough
		// maps [0, inf] to [0, 1]
		int hdrExposure = 2;
		hdrExposure = hdrExposure * 2 + 1;
		r = 1 + (1 / (pow(-1 - r, hdrExposure)));
		g = 1 + (1 / (pow(-1 - g, hdrExposure)));
		b = 1 + (1 / (pow(-1 - b, hdrExposure)));

		hdrImageData[i] = CreateVec3(r, g, b);
	}
}

std::vector<int> GetFactors(int n)
{
	std::vector<int> factors;
	for (int i = 1; i <= n; ++i)
		if (n % i == 0)
			factors.push_back(i);

	return factors;
}

std::tuple<int, int, int> FactorCombination(int num1, int num2, int maxProduct)
{
	std::vector<int> factorsNum1 = GetFactors(num1);
	std::vector<int> factorsNum2 = GetFactors(num2);

	std::vector<std::tuple<int, int, int>> combinations;

	for (int factor1 : factorsNum1)
	{
		for (int factor2 : factorsNum2)
		{
			int product = factor1 * factor2;
			if (product <= maxProduct)
				combinations.push_back(std::make_tuple(factor1, factor2, product));
		}
	}

	// Sort combinations by product and similarity of factors
	std::sort(combinations.begin(), combinations.end(), [](const auto& a, const auto& b)
	{
		return std::make_tuple(std::get<2>(a), -std::abs(std::get<0>(a) - std::get<1>(a))) >
			   std::make_tuple(std::get<2>(b), -std::abs(std::get<0>(b) - std::get<1>(b)));
	});

	return (combinations.empty() ? std::make_tuple(0, 0, 0) : combinations.front());
}

cl_vec3 GetIdealBlockSize(int globalSize, int localSize)
{
	int maxProduct = (int)ceil((float)(globalSize) / (float)localSize);

	std::tuple<int, int, int> combination = FactorCombination(IMAGE_WIDTH, IMAGE_HEIGHT, maxProduct);
	
	return CreateVec3(std::get<0>(combination), std::get<1>(combination), 0);
}