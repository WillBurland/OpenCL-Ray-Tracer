// ===== STRUCT DEFINITIONS =====

typedef struct
{
	float x, y, z;
} Vec3;

typedef struct
{
	Vec3 origin;
	Vec3 direction;
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
	double radius;
	Material material;
} Sphere;

typedef struct
{
	Vec3 p;
	Vec3 normal;
	double t;
	bool frontFace;
	Material material;
} HitRecord;

typedef struct
{
	Vec3 origin;
	Vec3 horizontal;
	Vec3 vertical;
	Vec3 lowerLeftCorner;
} Camera;

// ===== FUNCTION DEFINITIONS =====

Vec3 Vec3AddVec3(Vec3 a, Vec3 b);
Vec3 Vec3SubVec3(Vec3 a, Vec3 b);
Vec3 Vec3MulVec3(Vec3 a, Vec3 b);
Vec3 Vec3MulFloat(Vec3 a, float b);
Vec3 FloatMulVec3(float a, Vec3 b);
Vec3 Vec3DivFloat(Vec3 a, float b);
float Vec3LengthSquared(Vec3 a);
float Vec3Length(Vec3 a);
float Vec3Dot(Vec3 a, Vec3 b);
Vec3 Vec3Cross(Vec3 a, Vec3 b);
Vec3 Vec3Unit(Vec3 a);
Vec3 Vec3RandInUnitSphere(ulong *seed);
Vec3 RayColour(Ray ray, int maxDepth, Sphere *spheres, int sphereCount, ulong *seed);
void SetFaceNormal(HitRecord *hitRecord, Ray ray, Vec3 outwardNormal);
bool HitAnything(HitRecord *hitRecord, Ray ray, float tMin, float tMax, Sphere *spheres, int sphereCount);
bool HitSphere(Sphere s, Ray r, double tMin, double tMax, HitRecord *hit);
Ray GetRay(Camera camera, float u, float v);
ulong NextSeed(ulong seed);
float RandFloatFromSeed(ulong *seed);
bool LambertianScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed);
bool MetalScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed);
bool DielectricScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed);

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

Vec3 FloatMulVec3(float a, Vec3 b)
{
	Vec3 result;
	result.x = a * b.x;
	result.y = a * b.y;
	result.z = a * b.z;
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

double Vec3Reflectance(double cosine, double refIdx)
{
	double r0 = (1 - refIdx) / (1 + refIdx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

// ===== RAY FUNCTIONS =====

Vec3 RayAt(Ray ray, float t)
{
	return Vec3AddVec3(ray.origin, FloatMulVec3(t, ray.direction));
}


Vec3 RayColour(Ray ray, int maxDepth, Sphere *spheres, int sphereCount, ulong *seed)
{
	Vec3 unitDirection = Vec3Unit(ray.direction);
	float t = 0.5f * (unitDirection.y + 1.0f);
	Vec3 skyColour = Vec3AddVec3(FloatMulVec3(1.0f - t, (Vec3){ 1.0f, 1.0f, 1.0f }), FloatMulVec3(t, (Vec3){ 0.5f, 0.7f, 1.0f }));
	Vec3 finalColour = skyColour;

	int currentDepth = 0;
	HitRecord hitRecord;

	while (currentDepth < maxDepth)
	{
		if (HitAnything(&hitRecord, ray, 0.001f, INFINITY, spheres, sphereCount))
		{
			// Vec3 target = Vec3AddVec3(Vec3AddVec3(hitRecord.p, hitRecord.normal), Vec3RandUnitVector(seed));
			// ray = (Ray){ hitRecord.p, Vec3SubVec3(target, hitRecord.p) };

			Ray scattered;
			Vec3 attenuation;
			switch (hitRecord.material.type)
			{
				case 0:
				{
					if (LambertianScatter(ray, &hitRecord, &attenuation, &scattered, seed))
					{
						ray = scattered;
						finalColour = Vec3MulVec3(finalColour, attenuation);
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
						finalColour = Vec3MulVec3(finalColour, attenuation);
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
						finalColour = Vec3MulVec3(finalColour, attenuation);
						currentDepth++;
						continue;
					}
					else
					{
						return (Vec3){ 0, 0, 0 };
					}
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
	
	if (currentDepth == 0)
	{
		return skyColour;
	}

	return finalColour;
}

// ===== HIT RECORD FUNCTIONS =====

void SetFaceNormal(HitRecord *hitRecord, Ray ray, Vec3 outwardNormal)
{
	hitRecord->frontFace = Vec3Dot(ray.direction, outwardNormal) < 0;
	hitRecord->normal = hitRecord->frontFace ? outwardNormal : Vec3MulFloat(outwardNormal, -1);
}

bool HitAnything(HitRecord *hitRecord, Ray ray, float tMin, float tMax, Sphere *spheres, int sphereCount)
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
	*attenuation = hitRecord->material.albedo;

	return true;
}

bool MetalScatter(Ray ray, HitRecord *hitRecord, Vec3 *attenuation, Ray *scattered, ulong *seed)
{
	Vec3 reflected = Vec3Reflect(Vec3Unit(ray.direction), hitRecord->normal);
	scattered->origin = hitRecord->p;
	scattered->direction = Vec3AddVec3(reflected, Vec3MulFloat(Vec3RandInUnitSphere(seed), hitRecord->material.fuzz));
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

	return true;
}

// ===== SPHERE FUNCTIONS =====

bool HitSphere(Sphere s, Ray r, double tMin, double tMax, HitRecord *hit)
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

// ===== CAMERA FUNCTIONS =====

Ray GetRay(Camera camera, float u, float v)
{
	Ray result;
	result.origin = camera.origin;
	result.direction = Vec3SubVec3(Vec3AddVec3(Vec3AddVec3(camera.lowerLeftCorner, FloatMulVec3(u, camera.horizontal)), FloatMulVec3(v, camera.vertical)), camera.origin);
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

__kernel void pixel_colour(__global unsigned char *R, __global unsigned char *G, __global unsigned char *B, __global unsigned int *_randSeeds, __global int *_width, __global int *_height, __global int *_samplesPerPixel, __global int *_maxDepth)
{
	int global_id = get_global_id(0);
	float tempR = 0;
	float tempG = 0;
	float tempB = 0;

	int width = *_width;
	int height = *_height;
	int samplesPerPixel = *_samplesPerPixel;
	int maxDepth = *_maxDepth;

	ulong seed = _randSeeds[global_id];

	for (int i = 0; i < 128; i++)
	{
		seed = NextSeed(seed);
	}

	Vec3 lookFrom = { -2.0f, 1.5f, 3.0f };
	Vec3 lookAt = { 0.0f, 0.0f, -1.0f };
	Vec3 vUp = { 0.0f, 1.0f, 0.0f };
	float vfov = 20.0f;
	float aspectRatio = (float)width / (float)height;

	float theta = DegToRad(vfov);
	float h = tan(theta / 2);
	float viewportHeight = 2.0f * h;
	float viewportWidth = aspectRatio * viewportHeight;

	Vec3 w = Vec3Unit(Vec3SubVec3(lookFrom, lookAt));
	Vec3 u = Vec3Unit(Vec3Cross(vUp, w));
	Vec3 v = Vec3Cross(w, u);
	
	Vec3 origin = lookFrom;
	Vec3 horizontal = FloatMulVec3(viewportWidth, u);
	Vec3 vertical = FloatMulVec3(viewportHeight, v);
	Vec3 lowerLeftCorner = Vec3SubVec3(Vec3SubVec3(Vec3SubVec3(origin, FloatMulVec3(0.5f, horizontal)), FloatMulVec3(0.5f, vertical)), w);

	Camera camera = { origin, horizontal, vertical, lowerLeftCorner };

	Sphere spheres[7];
	spheres[0] = (Sphere){(Vec3){ 0.0, -100.5, -1.0}, 100.0, (Material){(Vec3){0.0, 0.8, 0.7}, 0.0, 0.0, 0}};
	spheres[1] = (Sphere){(Vec3){ 0.0,    0.5, -1.0},   0.5, (Material){(Vec3){0.7, 0.3, 0.9}, 0.0, 0.0, 0}};
	spheres[2] = (Sphere){(Vec3){-0.9,    0.0, -1.0},   0.5, (Material){(Vec3){0.8, 0.5, 0.5}, 0.1, 0.0, 1}};
	spheres[3] = (Sphere){(Vec3){ 0.9,    0.0, -1.0},   0.5, (Material){(Vec3){0.8, 0.6, 0.2}, 0.5, 0.0, 1}};
	spheres[4] = (Sphere){(Vec3){ 0.0,   -0.3, -1.0},   0.2, (Material){(Vec3){0.8, 0.8, 0.8}, 0.0, 0.0, 1}};
	spheres[5] = (Sphere){(Vec3){ 0.2,   -0.4, -0.8},   0.1, (Material){(Vec3){0.8, 0.8, 0.8}, 0.0, 1.5, 2}};
	spheres[6] = (Sphere){(Vec3){-0.2,   -0.4, -0.8},   0.1, (Material){(Vec3){0.8, 0.8, 0.8}, 0.0, 1.5, 2}};

	Vec3 pixelColour = {0.0, 0.0, 0.0};

	for (int i = 0; i < samplesPerPixel; i++)
	{
		float u = ((float)(global_id % width) + RandFloatFromSeed(&seed)) / width;
		float v = ((float)(global_id / width) + RandFloatFromSeed(&seed)) / height;

		Ray ray = GetRay(camera, u, v);
		pixelColour = Vec3AddVec3(pixelColour, RayColour(ray, maxDepth, spheres, sizeof(spheres) / sizeof(Sphere), &seed));
	}
	
	R[global_id] = pixelColour.x / samplesPerPixel * 255;
	G[global_id] = pixelColour.y / samplesPerPixel * 255;
	B[global_id] = pixelColour.z / samplesPerPixel * 255;
}