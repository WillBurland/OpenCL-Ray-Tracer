#include "cl_camera.hpp"

void CalculateCamera(cl_camera *camera, cl_vec3 lookFrom, cl_vec3 lookAt, cl_vec3 vUp, float vFov, float aspectRatio, float focusDistance, float aperture)
{
	// convert vFov to radians
	aperture *= (float)3.141592654 / 180.0f;

	// calculate viewport height and width
	float theta = vFov * (float)3.141592654 / 180.0f;
	float h = tan(theta / 2);
	float viewportHeight = 2.0f * h * focusDistance;
	float viewportWidth  = aspectRatio * viewportHeight;

	// calculate camera basis vectors
	cl_vec3 w = Vec3Unit(lookFrom - lookAt);
	cl_vec3 u = Vec3Unit(Vec3Cross(vUp, w));
	cl_vec3 v = Vec3Cross(w, u);

	// calculate viewport corners
	cl_vec3 viewportU   = u * viewportWidth;
	cl_vec3 viewportV   = v * viewportHeight;
	cl_vec3 pixelDeltaU = viewportU / (float)IMAGE_WIDTH;
	cl_vec3 pixelDeltaV = viewportV / (float)IMAGE_HEIGHT;
	cl_vec3 viewportLowerLeftCorner = ((lookFrom - (viewportU / 2.0f)) - (viewportV / 2.0f)) - (w * focusDistance);
	
	// calculate defocus disc
	float defocusRadius = focusDistance * tan(aperture / 2.0f);
	cl_vec3 defocusDiscU = u * defocusRadius;
	cl_vec3 defocusDiscV = v * defocusRadius;

	// set camera values
	camera->origin          = lookFrom;
	camera->horizontal      = viewportU;
	camera->vertical        = viewportV;
	camera->lowerLeftCorner = viewportLowerLeftCorner;
	camera->defocusDiscU    = defocusDiscU;
	camera->defocusDiscV    = defocusDiscV;
	camera->focusDistance   = focusDistance;
	camera->aperture        = aperture;
}