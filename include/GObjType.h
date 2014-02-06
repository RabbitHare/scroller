#ifndef __GOBJ_TYPE_H__
#define __GOBJ_TYPE_H__

struct GObjType
{
	char name[32]; // what's printed on screen
	char desc[64]; // what's used in the menu
	char cat[32]; // category: enemy,modifier, etc.
};

void InitGObjType (GObjType *type);

void LoadGObjTypes ();

#endif

