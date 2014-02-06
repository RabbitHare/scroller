#ifndef __BULLET1_H__
#define __BULLET1_H__

#include <Entity2.h>

struct Bullet1
{
	Entity2 ent;
	unsigned int animTimer;
};

#define BULLET1(a) ((Bullet1*)(a))

Bullet1* NewBullet1 ();

#endif

