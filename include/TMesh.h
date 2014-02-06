#ifndef __TMESH_H__
#define __TMESH_H__

#include <List.h>
#include <PoolAllocator.h>
#include <Vec3f.h>

struct TVert;
struct TFace;
struct TFaceVert;

union TPtr
{
	void *ptr;
	TVert *v;
	TFace *f;
	TFaceVert *fv;
	int num;
};
struct TFaceVert
{
	TFaceVert *next; //loop around the the vert

	TFace *f;
	TVert *v;
	Vec2f tco; // texture coord
};
struct TVert
{
	TVert *next,*prev;

	TFaceVert *fverts;
	Vec3f co,
	      no;
	Vec2f tco;
	TPtr tmp;
	inline TVert ()
	{
		tmp.ptr = NULL;
		fverts = NULL;
	}
	inline void* operator new (size_t) { return mAlloc.Alloc(); }
	inline void  operator delete (void *p) { mAlloc.Free ((TVert*)p); }
private:
	static PoolAllocator<TVert,100> mAlloc;
};

enum FaceFlags
{
	F_ALPHA = 1
};
struct TFace
{
	TFace *next,*prev;

	TFaceVert p[3];
	TPtr tmp;
	int flags;
	inline TFace ()
	{
		tmp.ptr = NULL;
		flags = 0;
	}
	inline void* operator new (size_t) { return mAlloc.Alloc(); }
	inline void  operator delete (void *p) { mAlloc.Free ((TFace*)p); }
private:
	static PoolAllocator<TFace,100> mAlloc;
};

struct TMesh
{

	List<TFace> faces;
	List<TVert> verts;
	~TMesh ()
	{
		Clear ();
	}
	void Clear ()
	{
		faces.Purge();
		verts.Purge();
	}
};
#endif

