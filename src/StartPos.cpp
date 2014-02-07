
#include <StartPos.h>
#include <GameGlobals.h>

StartPos* NewStartPos ()
{

	Entity *ent = NewEntity ();
	StartPos *self = START_POS(ent);

	ent->type = ET_START;

	return self;
}

