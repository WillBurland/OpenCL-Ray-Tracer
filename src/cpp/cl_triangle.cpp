#include "cl_triangle.hpp"

CLTriangle CreateTriangle(CLVec3 p0, CLVec3 p1, CLVec3 p2, CLMaterial material)
{
	CLTriangle result;
    result.p0 = p0;
    result.p1 = p1;
    result.p2 = p2;
	result.material = material;
    result.boundingBoxMin = (CLVec3){
        (float)fmin(p0.x, (float)fmin(p1.x, p2.x)),
        (float)fmin(p0.y, (float)fmin(p1.y, p2.y)),
        (float)fmin(p0.z, (float)fmin(p1.z, p2.z))};
    result.boundingBoxMax = (CLVec3){
        (float)fmax(p0.x, (float)fmax(p1.x, p2.x)),
        (float)fmax(p0.y, (float)fmax(p1.y, p2.y)),
        (float)fmax(p0.z, (float)fmax(p1.z, p2.z))};
	return result;
}