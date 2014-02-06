#ifndef __MESH_H__
#define __MESH_H__

#define MAX_VERTS 4096
#define MAX_TEX_VERTS 8192
#define MAX_FACES 4096

#include <Vec3f.h>

struct Face
{
	int v[3];
	int tv[3];
	Vec3f normal;
	float dist;
};

struct Mesh
{
	Vec3f verts[MAX_VERTS];
	Vec2f texVerts[MAX_VERTS];
	Face faces[MAX_FACES];
	int numVerts,
	    numTexVerts,
	    numFaces;
};
		

#endif
