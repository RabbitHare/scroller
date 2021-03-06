#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <EditTypes.h>
#include <gdk/gdkgl.h>
struct Layer;

struct Global
{
	Layer *selLayer;
	TileSelection selTiles;
	GdkGLContext *glContext; // for creating widgets with shared lists
	Vec3f bgColor;
	Global ()
	{
		selLayer = NULL;
		glContext = NULL;
		bgColor = Vec3f(0.3,0.3,1.0);
	}
};
extern Global G; // declared in Editor.h

#endif

