#ifndef __BULLET1_H__
#define __BULLET1_H__

#include <Entity.h>

struct Bullet1
{
	Entity ent;
	unsigned int animTimer;
};

#define BULLET1(a) ((Bullet1*)(a))

Bullet1* NewBullet1 ();

#endif

