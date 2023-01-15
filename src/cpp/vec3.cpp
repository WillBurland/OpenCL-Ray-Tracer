#include <cmath>

#include "vec3.hpp"

Vec3::Vec3()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

Vec3::Vec3(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}