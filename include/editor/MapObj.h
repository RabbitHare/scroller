#ifndef __MAP_OBJ_H__
#define __MAP_OBJ_H__

#include <libxml/xmlwriter.h>
#include <Entity2.h>

struct MapObj // used in the editor`
{
	Entity2 ent;
	int type; // index into list of containing unmodifieable data, i.e. name, description etc.
};

#define MAP_OBJ(a) ((MapObj*)(a))


MapObj* NewMapObj (int type);

void WriteMapObj (xmlTextWriterPtr writer, MapObj *obj);


#endif

