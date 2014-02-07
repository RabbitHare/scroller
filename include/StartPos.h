#ifndef __START_POS_H__
#define __START_POS_H__

#include <Entity.h>

struct StartPos
{
	Entity ent;
	unsigned int animTimer;
};

#define START_POS(a) ((StartPos*)(a))

StartPos* NewStartPos ();

#endif
