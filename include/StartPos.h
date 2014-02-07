#ifndef __START_POS_H__
#define __START_POS_H__

#include <Entity2.h>

struct StartPos
{
	Entity2 ent;
	unsigned int animTimer;
};

#define START_POS(a) ((StartPos*)(a))

StartPos* NewStartPos ();

#endif
