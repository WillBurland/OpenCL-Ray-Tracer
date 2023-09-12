#include "cl_camera.hpp"

void CalculateCamera(CLCamera *camera, CLVec3 lookFrom, CLVec3 lookAt, CLVec3 vUp, float vFov, float aspectRatio, float focusDistance, float aperture)
{
	aperture *= (float)3.141592654 / 180.0f;

	float theta = vFov * (float)3.141592654 / 180.0f;
	float h = tan(theta / 2);
	float viewportHeight = 2.0f * h * focusDistance;
	float viewportWidth = aspectRatio * viewportHeight;
	
	CLVec3 w = Vec3Unit(Vec3SubVec3(lookFrom, lookAt));
	CLVec3 u = Vec3Unit(Vec3Cross(vUp, w));
	CLVec3 v = Vec3Cross(w, u);

	CLVec3 viewportU = Vec3MulFloat(u, viewportWidth);
	CLVec3 viewportV = Vec3MulFloat(v, viewportHeight);

	CLVec3 pixelDeltaU = Vec3DivFloat(viewportU, (float)IMAGE_WIDTH);
	CLVec3 pixelDeltaV = Vec3DivFloat(viewportV, (float)IMAGE_HEIGHT);

	CLVec3 viewportLowerLeftCorner = Vec3SubVec3(Vec3SubVec3(Vec3SubVec3(lookFrom, Vec3DivFloat(viewportU, 2.0f)), Vec3DivFloat(viewportV, 2.0f)), Vec3MulFloat(w, focusDistance));
	
	float defocusRadius = focusDistance * tan(aperture / 2.0f);
	CLVec3 defocusDiscU = Vec3MulFloat(u, defocusRadius);
	CLVec3 defocusDiscV = Vec3MulFloat(v, defocusRadius);
	
	printf("defocusDiscU: %f %f %f\n", defocusDiscU.x, defocusDiscU.y, defocusDiscU.z);
	printf("defocusDiscV: %f %f %f\n", defocusDiscV.x, defocusDiscV.y, defocusDiscV.z);
	printf("lowerLeftCorner %f %f %f\n", viewportLowerLeftCorner.x, viewportLowerLeftCorner.y, viewportLowerLeftCorner.z);

	camera->origin = lookFrom;
	camera->horizontal = viewportU;
	camera->vertical = viewportV;
	camera->lowerLeftCorner = viewportLowerLeftCorner;
	camera->defocusDiscU = defocusDiscU;
	camera->defocusDiscV = defocusDiscV;
	camera->focusDistance = focusDistance;
	camera->aperture = aperture;
}