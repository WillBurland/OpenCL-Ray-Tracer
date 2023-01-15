#ifndef WB_RT_VEC3_HPP
#define WB_RT_VEC3_HPP

#include "globals.hpp"

class Vec3
{
	public:
		double x, y, z;

		Vec3();
		Vec3(double x, double y, double z);
		~Vec3(){};

		Vec3& operator+=(const Vec3 &v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		Vec3& operator*=(const double t)
		{
			x *= t;
			y *= t;
			z *= t;
			return *this;
		}

		Vec3& operator/=(const double t)
		{
			return *this *= 1 / t;
		}
		
		double Length() const;
		double LengthSquared() const;
		Vec3 Random();
		Vec3 Random(double min, double max);
		bool NearZero() const;
};

Vec3 operator+(const Vec3 &u, const Vec3 &v);
Vec3 operator-(const Vec3 &u, const Vec3 &v);
Vec3 operator*(const Vec3 &u, const Vec3 &v);
Vec3 operator*(double t, const Vec3 &v);
Vec3 operator*(const Vec3 &v, double t);
Vec3 operator/(Vec3 v, double t);
double Dot(const Vec3 &u, const Vec3 &v);
Vec3 Cross(const Vec3 &u, const Vec3 &v);
Vec3 UnitVector(Vec3 v);
Vec3 RandomInUnitSphere();
Vec3 RandomInHemisphere(const Vec3 &normal);
Vec3 RandomUnitVector();
Vec3 Reflect(const Vec3 &v, const Vec3 &n);

using Point3D = Vec3;
using Colour = Vec3;

#endif