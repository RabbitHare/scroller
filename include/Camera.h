#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <Mat4f.h>

class Camera
{
public:
	Mat4f	mat;
	Mat4f	projMat;
	Camera ();
	void SetProjection (int w, int h, float fov);
	Vec2f GetDrawScale (float z) const;
};
#endif
