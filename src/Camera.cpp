#include <Camera.h>
#include <GL/gl.h>

Camera::Camera()
{
	mat.MakeIdentity ();
	projMat.MakeIdentity ();
}

void Camera::SetProjection (int w, int h, float fov)
{
    double radfov = fov/180.0 * PI;
    double cotan = 1.0/tan(radfov * 0.5);
    projMat.M[1][0] = projMat.M[2][0] = projMat.M[3][0] = 
	    projMat.M[0][1] = projMat.M[2][1] = projMat.M[3][1] = 
	    projMat.M[0][2] = projMat.M[1][2] =
	    projMat.M[0][3] = projMat.M[1][3] =
	    projMat.M[3][3] = 0;
    projMat.M[0][0] = cotan;
    projMat.M[1][1] = cotan * ((double)w)/h;
    projMat.M[2][2] = -1.0;
    projMat.M[3][2] = -cotan;
    projMat.M[2][3] = -1.0;

    glMatrixMode (GL_PROJECTION);
    glLoadMatrixf (projMat.M[0]);
}
Vec2f Camera::GetDrawScale (float z) const
{
	Vec2f ret;
	Vec4f tmp1(1.0f,1.0f,z - mat.M[3][2],1.0);
	Vec4f tmp2 = projMat*tmp1;
	ret.x = tmp2.V[0];
	ret.y = tmp2.V[1];
	if (tmp2.V[3] != 0.0)
	{
		ret.x /= tmp2.V[3];
		ret.y /= tmp2.V[3];
	}
	return ret;

}

