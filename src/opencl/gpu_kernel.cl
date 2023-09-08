// ===== STRUCT DEFINITIONS =====

typedef struct
{
	float x, y, z;
} Vec3;

typedef struct
{
	Vec3 origin;
	Vec3 direction;
	Vec3 invDirection;
} Ray;

typedef struct
{
	Vec3 albedo;
	float fuzz;
	float ior;
	int type;
} Material;

typedef struct
{
	Vec3 center;
	float radius;
	Material material;
	Vec3 boundingBoxMin;
	Vec3 boundingBoxMax;
} Sphere;

typedef struct
{
	Vec3 p0, p1, p2;
	Material material;
	int boundingBoxId;
} Triangle;

typedef struct
{
	int id;
	Vec3 min;
	Vec3 max;
} BoundingBox;

typedef struct
{
	Vec3 p;
	Vec3 normal;
	float t;
	bool frontFace;
	Material material;
} HitRecord;

typedef struct
{
	Vec3 origin;
	Vec3 horizontal;
	Vec3 vertical;
	Vec3 lowerLeftCorner;
	int width;
	int height;
	int samplesPerPixel;
	int maxDepth;
} Camera;

// ===== FUNCTION DEFINITIONS =====

Vec3 Vec3AddVec3(Vec3 a, Vec3 b);
Vec3 Vec3SubVec3(Vec3 a, Vec3 b);
Vec3 Vec3MulVec3(Vec3 a, Vec3 b);
Vec3 Vec3MulFloat(Vec3 a, float b);
Vec3 Vec3DivFloat(Vec3 a, float b);
float Vec3LengthSquared(Vec3 a);
float Vec3Length(Vec3 a);
float Vec3Dot(Vec3 a, Vec3 b);
Vec3 Vec3Cross(Vec3 a, Vec3 b);
Vec3 Vec3Unit(Vec3 a);
Vec3 Vec3RandInUnitSphere(ulong *seed);
Vec3 Vec3RandUnitVector(ulong *seed);
bool Vec3NearZero(Vec3 a);
Vec3 Vec3Reflect(Vec3 v, Vec3 n);
Vec3 Vec3Refract(Vec3 uv, Vec3 n, float etaiOverEtat);
float Vec3Reflectance(float cosine, float refIdx);
Vec3 Vec3Inv(Vec3 a);
Vec3 RayAt(Ray ray, float t);
Vec3 RayColour(Ray ray, int maxDepth, Sphere *spheres, int sphereCount, Triangle *triangles, int triangleCount, BoundingBox *boundingBoxes, int boundingBoxCount, ulong *seed);
void SetFaceNormal(HitRecord *hitRecord, Ray ray, Vec3 outwardNormal);
bool HitAnything(HitRecord *hitRecord, Ray ray, float tMin, float tMax, Sphere *spheres, int sphereCount, Triangle *triangles, int triangleCount, BoundingBox *boundingBoxes, int boundingBoxCount);
bool LambertianScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed);
bool MetalScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed);
bool DielectricScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed);
bool HitSphere(Sphere s, Ray r, float tMin, float tMax, HitRecord *hit);
bool HitTriangle(Triangle t, Ray r, float tMin, float tMax, HitRecord *hit);
bool HitBoundingBox(Vec3 min, Vec3 max, Ray r);
Ray GetRay(Camera camera, float s, float t);
ulong NextSeed(ulong seed);
float RandFloatFromSeed(ulong *seed);
float DegToRad(float degrees);

// ===== VECTOR FUNCTIONS =====

Vec3 Vec3AddVec3(Vec3 a, Vec3 b)
{
	Vec3 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

Vec3 Vec3SubVec3(Vec3 a, Vec3 b)
{
	Vec3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

Vec3 Vec3MulVec3(Vec3 a, Vec3 b)
{
	Vec3 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}

Vec3 Vec3MulFloat(Vec3 a, float b)
{
	Vec3 result;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
	return result;
}

Vec3 Vec3DivFloat(Vec3 a, float b)
{
	Vec3 result;
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	return result;
}

float Vec3LengthSquared(Vec3 a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

float Vec3Length(Vec3 a)
{
	return sqrt(Vec3LengthSquared(a));
}

float Vec3Dot(Vec3 a, Vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 Vec3Cross(Vec3 a, Vec3 b)
{
	Vec3 result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

Vec3 Vec3Unit(Vec3 a)
{
	return Vec3DivFloat(a, Vec3Length(a));
}

Vec3 Vec3RandInUnitSphere(ulong *seed)
{
	Vec3 p;
	while (true)
	{
		float randX = RandFloatFromSeed(seed) * 2.0f - 1.0f;
		float randY = RandFloatFromSeed(seed) * 2.0f - 1.0f;
		float randZ = RandFloatFromSeed(seed) * 2.0f - 1.0f;

		p = (Vec3){ randX, randY, randZ };
	
		if (Vec3LengthSquared(p) >= 1.0f) continue;
		return p;
	}
}

Vec3 Vec3RandUnitVector(ulong *seed)
{
	return Vec3Unit(Vec3RandInUnitSphere(seed));
}

bool Vec3NearZero(Vec3 a)
{
	const float s = 1e-8;
	return (fabs(a.x) < s) && (fabs(a.y) < s) && (fabs(a.z) < s);
}

Vec3 Vec3Reflect(Vec3 v, Vec3 n)
{
	return Vec3SubVec3(v, Vec3MulFloat(n, 2.0f * Vec3Dot(v, n)));
}

Vec3 Vec3Refract(Vec3 uv, Vec3 n, float etaiOverEtat)
{
	float cosTheta = fmin(Vec3Dot(Vec3MulFloat(uv, -1.0f), n), 1.0f);
	Vec3 rOutPerp = Vec3AddVec3(Vec3MulFloat(uv, etaiOverEtat), Vec3MulFloat(n, cosTheta * etaiOverEtat));
	Vec3 rOutParallel = Vec3MulFloat(n, -sqrt(fabs(1.0f - Vec3LengthSquared(rOutPerp))));
	return Vec3AddVec3(rOutPerp, rOutParallel);
}

float Vec3Reflectance(float cosine, float refIdx)
{
	float r0 = (1 - refIdx) / (1 + refIdx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

Vec3 Vec3Inv(Vec3 a)
{
	return (Vec3){ 1.0f / a.x, 1.0f / a.y, 1.0f / a.z };
}

// ===== RAY FUNCTIONS =====

Vec3 RayAt(Ray ray, float t)
{
	return Vec3AddVec3(ray.origin, Vec3MulFloat(ray.direction, t));
}

Vec3 RayColour(Ray ray, int maxDepth, Sphere *spheres, int sphereCount, Triangle *triangles, int triangleCount, BoundingBox *boundingBoxes, int boundingBoxCount, ulong *seed)
{
	Vec3 unitDirection = Vec3Unit(ray.direction);
	float t = 0.5f * (unitDirection.y + 1.0f);
	
	bool useSun = true;
	Vec3 skyColour = (Vec3){0.0f, 0.0f, 0.0f};
	if (useSun)
		skyColour = Vec3AddVec3(Vec3MulFloat((Vec3){ 1.0f, 1.0f, 1.0f }, 1.0f - t), Vec3MulFloat((Vec3){ 0.5f, 0.7f, 1.0f }, t));

	Vec3 directLightColour = (Vec3){ 1.0f, 1.0f, 1.0f };

	int currentDepth = 0;
	HitRecord hitRecord;

	while (currentDepth < maxDepth)
	{
		if (HitAnything(&hitRecord, ray, 0.001f, INFINITY, spheres, sphereCount, triangles, triangleCount, boundingBoxes, boundingBoxCount))
		{
			Ray scattered;
			Vec3 attenuation;
			switch (hitRecord.material.type)
			{
				case 0:
				{
					if (LambertianScatter(ray, &hitRecord, &attenuation, &scattered, seed))
					{
						ray = scattered;
						skyColour = Vec3MulVec3(skyColour, attenuation);
						directLightColour = Vec3MulVec3(directLightColour, attenuation);
						currentDepth++;
						continue;
					}
					else
					{
						return (Vec3){ 0, 0, 0 };
					}
				} break;
				case 1:
				{
					if (MetalScatter(ray, &hitRecord, &attenuation, &scattered, seed))
					{
						ray = scattered;
						skyColour = Vec3MulVec3(skyColour, attenuation);
						directLightColour = Vec3MulVec3(directLightColour, attenuation);
						currentDepth++;
						continue;
					}
					else
					{
						return (Vec3){ 0, 0, 0 };
					}
				} break;
				case 2:
				{
					if (DielectricScatter(ray, &hitRecord, &attenuation, &scattered, seed))
					{
						ray = scattered;
						skyColour = Vec3MulVec3(skyColour, attenuation);
						directLightColour = Vec3MulVec3(directLightColour, attenuation);
						currentDepth++;
						continue;
					}
					else
					{
						return (Vec3){ 0, 0, 0 };
					}
				} break;
				case 3:
				{
					return directLightColour;
				}
			}
			currentDepth++;
			continue;
		}
		break;
	}

	if (currentDepth == maxDepth)
	{
		return (Vec3){ 0, 0, 0 };
	}

	return skyColour;
}

// ===== HIT RECORD FUNCTIONS =====

void SetFaceNormal(HitRecord *hitRecord, Ray ray, Vec3 outwardNormal)
{
	hitRecord->frontFace = Vec3Dot(ray.direction, outwardNormal) < 0;
	hitRecord->normal = hitRecord->frontFace ? outwardNormal : Vec3MulFloat(outwardNormal, -1);
}

bool HitAnything(HitRecord *hitRecord, Ray ray, float tMin, float tMax, Sphere *spheres, int sphereCount, Triangle *triangles, int triangleCount, BoundingBox *boundingBoxes, int boundingBoxCount)
{
	HitRecord temp;
	bool hitAnything = false;
	float closestSoFar = INFINITY;

	for (int i = 0; i < sphereCount; i++)
	{
		if (HitSphere(spheres[i], ray, tMin, closestSoFar, &temp))
		{
			hitAnything = true;
			closestSoFar = temp.t;
			*hitRecord = temp;
		}
	}

	for (int i = 0; i < boundingBoxCount; i++)
	{
		if (HitBoundingBox(boundingBoxes[i].min, boundingBoxes[i].max, ray))
		{
			for (int j = 0; j < triangleCount; j++)
			{
				if (triangles[j].boundingBoxId == i)
				{
					if (HitTriangle(triangles[j], ray, tMin, closestSoFar, &temp))
					{
						hitAnything = true;
						closestSoFar = temp.t;
						*hitRecord = temp;
					}
				}
			}
		}
	}

	return hitAnything;
}

// ===== SCATTER FUNCTIONS =====
bool LambertianScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed)
{
	Vec3 scatterDir = Vec3AddVec3(hitRecord->normal, Vec3RandUnitVector(seed));

	if (Vec3NearZero(scatterDir))
	{
		scatterDir = hitRecord->normal;
	}

	scattered->origin = hitRecord->p;
	scattered->direction = scatterDir;
	scattered->invDirection = Vec3Inv(scattered->direction);
	*attenuation = hitRecord->material.albedo;

	return true;
}

bool MetalScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed)
{
	Vec3 reflected = Vec3Reflect(Vec3Unit(ray.direction), hitRecord->normal);
	scattered->origin = hitRecord->p;
	scattered->direction = Vec3AddVec3(reflected, Vec3MulFloat(Vec3RandInUnitSphere(seed), hitRecord->material.fuzz));
	scattered->invDirection = Vec3Inv(scattered->direction);
	*attenuation = hitRecord->material.albedo;
	return Vec3Dot(scattered->direction, hitRecord->normal) > 0;
}

bool DielectricScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed)
{
	*attenuation = (Vec3){ 1.0f, 1.0f, 1.0f };
	float refractionRatio = hitRecord->frontFace ? (1.0f / hitRecord->material.ior) : hitRecord->material.ior;

	Vec3 unitDirection = Vec3Unit(ray.direction);
	float cosTheta = fmin(Vec3Dot(Vec3MulFloat(unitDirection, -1), hitRecord->normal), 1.0f);
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	bool cannotRefract = refractionRatio * sinTheta > 1.0f;
	Vec3 direction;

	if (cannotRefract || Vec3Reflectance(cosTheta, refractionRatio) > RandFloatFromSeed(seed))
	{
		direction = Vec3Reflect(unitDirection, hitRecord->normal);
	}
	else
	{
		direction = Vec3Refract(unitDirection, hitRecord->normal, refractionRatio);
	}

	scattered->origin = hitRecord->p;
	scattered->direction = direction;
	scattered->invDirection = Vec3Inv(scattered->direction);

	return true;
}

// ===== COLLISION FUNCTIONS =====

bool HitSphere(Sphere s, Ray r, float tMin, float tMax, HitRecord *hit)
{
	Vec3 oc = Vec3SubVec3(r.origin, s.center);
	float a = Vec3LengthSquared(r.direction);
	float halfB = Vec3Dot(oc, r.direction);
	float c = Vec3LengthSquared(oc) - s.radius * s.radius;
	float discriminant = halfB * halfB - a * c;

	if (discriminant < 0)
	{
		return false;
	}

	float sqrtd = sqrt(discriminant);

	float root = (-halfB - sqrtd) / a;
	if (root < tMin || tMax < root)
	{
		root = (-halfB + sqrtd) / a;
		if (root < tMin || tMax < root)
		{
			return false;
		}
	}

	hit->t = root;
	hit->p = RayAt(r, root);
	Vec3 outwardNormal = Vec3DivFloat(Vec3SubVec3(hit->p, s.center), s.radius);
	SetFaceNormal(hit, r, outwardNormal);
	hit->material = s.material;

	return true;
}

bool HitTriangle(Triangle t, Ray r, float tMin, float tMax, HitRecord *hit)
{
	Vec3 edge0 = Vec3SubVec3(t.p1, t.p0);
	Vec3 edge1 = Vec3SubVec3(t.p2, t.p0);
	Vec3 h = Vec3Cross(r.direction, edge1);
	float a = Vec3Dot(edge0, h);

	if (a > -0.00001f && a < 0.00001f)
	{
		return false;
	}

	float f = 1.0f / a;
	Vec3 s = Vec3SubVec3(r.origin, t.p0);
	float u = f * Vec3Dot(s, h);

	if (u < 0.0f || u > 1.0f)
	{
		return false;
	}

	Vec3 q = Vec3Cross(s, edge0);
	float v = f * Vec3Dot(r.direction, q);

	if (v < 0.0f || u + v > 1.0f)
	{
		return false;
	}

	float t0 = f * Vec3Dot(edge1, q);

	if (t0 > tMin && t0 < tMax)
	{
		hit->t = t0;
		hit->p = RayAt(r, t0);
		Vec3 outwardNormal = Vec3Unit(Vec3Cross(edge0, edge1));
		SetFaceNormal(hit, r, outwardNormal);
		hit->material = t.material;

		return true;
	}
	
	return false;
}

bool HitBoundingBox(Vec3 min, Vec3 max, Ray r)
{
	float tx1 = (min.x - r.origin.x) * r.invDirection.x;
	float tx2 = (max.x - r.origin.x) * r.invDirection.x;

	float tmin = fmin(tx1, tx2);
	float tmax = fmax(tx1, tx2);

	float ty1 = (min.y - r.origin.y) * r.invDirection.y;
	float ty2 = (max.y - r.origin.y) * r.invDirection.y;

	tmin = fmax(tmin, fmin(ty1, ty2));
	tmax = fmin(tmax, fmax(ty1, ty2));

	float tz1 = (min.z - r.origin.z) * r.invDirection.z;
	float tz2 = (max.z - r.origin.z) * r.invDirection.z;

	tmin = fmax(tmin, fmin(tz1, tz2));
	tmax = fmin(tmax, fmax(tz1, tz2));

	return tmax > fmax(tmin, 0.0f);
}

// ===== CAMERA FUNCTIONS =====

Ray GetRay(Camera camera, float s, float t)
{
	Ray result;
	result.origin = camera.origin;
	result.direction = Vec3SubVec3(Vec3AddVec3(Vec3AddVec3(camera.lowerLeftCorner, Vec3MulFloat(camera.horizontal, s)), Vec3MulFloat(camera.vertical, t)), camera.origin);
	result.invDirection = Vec3Inv(result.direction);
	return result;
}

// ===== RANDOM FUNCTIONS =====
ulong NextSeed(ulong seed)
{
	return (seed * 0x5DEECE66D + 0xB) & ((1ULL << 48) - 1);
}

float RandFloatFromSeed(ulong *seed)
{
	*seed = NextSeed(*seed);
	return (float)(*seed >> 16) / 0xFFFFFFFF;
}

// ===== OTHER FUNCTIONS =====
float DegToRad(float degrees)
{
	return degrees * (float)M_PI / 180.0f;
}

// ===== MAIN FUNCTION =====

__kernel void pixel_colour(
	__global Vec3 *RGB,
	__global unsigned int *_randSeeds,
	__constant Camera *_camera,
	__global Sphere      *_spheres,       __constant int *_sphereCount,
	__global Triangle    *_triangles,     __constant int *_triangleCount,
	__global BoundingBox *_boundingBoxes, __constant int *_boundingBoxCount)
{
	int global_id = get_global_id(0);

	ulong seed = _randSeeds[global_id];

	Camera camera = *_camera;
	Sphere *spheres = _spheres;
	int sphereCount = *_sphereCount;
	Triangle *triangles = _triangles;
	int triangleCount = *_triangleCount;
	BoundingBox *boundingBoxes = _boundingBoxes;
	int boundingBoxCount = *_boundingBoxCount;

	Vec3 pixelColour = {0.0, 0.0, 0.0};

	for (int i = 0; i < camera.samplesPerPixel; i++)
	{
		float u = ((float)(global_id % camera.width) + RandFloatFromSeed(&seed)) / camera.width;
		float v = ((float)(global_id / camera.width) + RandFloatFromSeed(&seed)) / camera.height;

		Ray ray = GetRay(camera, u, v);
		pixelColour = Vec3AddVec3(pixelColour, RayColour(ray, camera.maxDepth, spheres, sphereCount, triangles, triangleCount, boundingBoxes, boundingBoxCount, &seed));
	}
	
	RGB[global_id] = Vec3DivFloat(pixelColour, camera.samplesPerPixel / 255.0f);
}