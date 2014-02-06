#include <MapObj.h>
#include <GameGlobals.h>
#include <GL/glut.h>
#include <Map.h>
#include <Global.h>
#include <iostream>
#include <cfloat>

#define MOBJ_FONT_W 8.
#define MOBJ_FONT_H 10.


static void DrawName (Vec3f pos, const char *text)
{
	Layer *lay = G.selLayer;
	float tw,th;
	tw = lay->tileW;
	th = lay->tileH;
	
	int x_end = tw/MOBJ_FONT_W;

	int len = strlen (text);

	int y_end = len/x_end;
	if (len%x_end) y_end++;

	if (x_end > len) x_end = len;
	
	char c = text[0];
	int i=0;
	pos.y = pos.y + (MOBJ_FONT_H*y_end)*0.5 - MOBJ_FONT_H;
	Vec3f p = pos;

	for (int y=0; y<y_end; y++)
	{
		if (x_end > (len-i)) x_end = len-i;
		p.x = pos.x - (MOBJ_FONT_W*(x_end))*0.5;
		for (int x=0; x<x_end; x++)
		{
			if (c == '\0')
				break;
			glRasterPos3fv (p.V);	
			glutBitmapCharacter (GLUT_BITMAP_TIMES_ROMAN_10, (int)c);
			i++;
			c = text[i];
			p.x += MOBJ_FONT_W;
		}
		p.y -= MOBJ_FONT_H;
	}
}

static void
RenderCb (Entity2 *ent, Camera &cam)
{
	if (ent->dead) return;

	float x1,x2,y1,y2,z,
	      tw,th;

	Layer *lay = G.selLayer;
	tw = lay->tileW;
	th = lay->tileH;

	x1 = ent->pos.x - tw/2;
	x2 = ent->pos.x + tw/2;
	y1 = ent->pos.y + th/2;
	y2 = ent->pos.y - th/2;
	z = ent->pos.z;

	Vec3f c = G.bgColor;
	c *= 0.5;

	glDisable (GL_TEXTURE_2D);
	glColor3f (c.x, c.y, c.z);


	glBegin (GL_QUADS);
	glVertex3f (x1,y1, z);
	glVertex3f (x1,y2, z);
	glVertex3f (x2,y2, z);
	glVertex3f (x2,y1, z);
	glEnd ();

	// DRAW NAME
	glColor3f (1,1,1);
	glRasterPos3f (ent->pos.x, ent->pos.y, ent->pos.z);
	char *name = g_objTypes[MAP_OBJ (ent)->type].name;
	DrawName (ent->pos, name);

	glEnable (GL_TEXTURE_2D);

}
MapObj* NewMapObj (int type)
{
	MapObj *obj;
	obj = MAP_OBJ(NewEntity ());
	obj->type = type;
	obj->ent.dead = false;
	obj->ent.Render = RenderCb;
	return obj;
}

void WriteMapObj (xmlTextWriterPtr writer, MapObj *obj)
{
	using namespace std;
	stringstream ss;

	xmlTextWriterStartElement (writer, BAD_CAST "object");
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "name", "%s", g_objTypes[obj->type].name);

	ss.seekp(0);

	//position
	ss << "( ";
	for (int v=0; v<3; v++)
	{
		ss << obj->ent.pos.V[v];
		ss << ' ';
	}
	ss << ")\n";

	ss << '\0';
	xmlTextWriterWriteFormatElement (writer, BAD_CAST "position", "%s", ss.str().c_str());

	xmlTextWriterEndElement (writer); // object
}
