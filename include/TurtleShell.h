#ifndef __TURTLE_SHELL_H__
#define __TURTLE_SHELL_H__

#include <Entity.h>

struct TurtleShell
{
	Entity ent;
	unsigned int animTimer;
};

#define TURTLE_SHELL(a) ((TurtleShell*)(a))

TurtleShell* NewTurtleShell ();

#endif

