#ifndef __NORM_TURT_H__
#define __NORM_TURT_H__

#include <Entity.h>

struct NormTurt
{
	Entity ent;
	unsigned int animTimer;
};

#define NORM_TURT(a) ((NormTurt*)(a))

NormTurt* NewNormTurt ();

#endif

