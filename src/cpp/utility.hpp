#ifndef WB_RT_UTILITY_HPP
#define WB_RT_UTILITY_HPP

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>

#include "lib/hdrloader.hpp"
#include "opencl_objects/cl_triangle.hpp"

int PrintOpenCLInfo();
int GetNumOfVertices(std::ifstream &infile);
int GetNumOfFaces(std::ifstream &infile);
void ReadMeshData(std::ifstream &meshFile, cl_vec3 *vertices, cl_triangle *triangles, cl_vec3 &min, cl_vec3 &max, cl_vec3 meshFileScale, cl_vec3 meshFileTranslate, cl_material material);
void ReadHdrImageData(cl_vec3 *hdrImageData, HDRLoaderResult &result);
cl_vec3 GetIdealBlockSize(int globalSize, int localSize);

#endif