#include <UndoSys.h>
#include <List.h>
#include <Map.h>
#include <MapObj.h>
#include <Ui.h>

#include <vector>

static List<UndoSys::Action> actions;
static UndoSys::Action *currUndo = actions.First();

struct StackTile
{
	int pos;
	int value;
};
static std::vector<StackTile> tileStack;
static int tileStackPos=0;

static int unsavedActions=0;

//-------------------
// TileAction
// ------------------
void UndoSys::TileAction::Do ()
{
	for (int i=0; i < numTiles; i++)
	{
		int pos = tileStack[tiles+i].pos;
		int tmp =  layer->tiles[pos];
		layer->tiles[pos] = tileStack[tiles+i].value;
		// for the redo
		tileStack[tiles+i].value = tmp;
	}
}
void UndoSys::TileAction::Undo ()
{
	Do ();
	tileStackPos -= numTiles;
}
void UndoSys::TileAction::Redo ()
{
	Do ();
	tileStackPos += numTiles;
}
void UndoSys::TileAction::AddTile (int tile)
{
	tiles = tileStackPos-numTiles;
	// see if the position already exists
	for (int i = 0; i < numTiles; i++)
		if (tileStack[tiles+i].pos == tile)
			return;

	if (tileStackPos >= (int)tileStack.size())
	{
		tileStack.resize (tileStackPos+1);
	}
	tileStack[tileStackPos].pos = tile;
	tileStack[tileStackPos].value = layer->tiles[tile];
	numTiles++;
	tileStackPos++;
}
/*==================
 *
 *  UNDO/REDO Add MapObj
 *
 *  ================
 */

void UndoSys::AddMapObjAction::Undo ()
{
	obj->ent.dead = true;
}

void UndoSys::AddMapObjAction::Redo ()
{
	obj->ent.dead = false;
}
void UndoSys::RemoveMapObjAction::Undo ()
{
	obj->ent.dead = false;
}

void UndoSys::RemoveMapObjAction::Redo ()
{
	obj->ent.dead = true;
}
//---------------------
// UndoSys functions
// --------------------

void UndoSys::Undo ()
{
	if (currUndo == actions.End())
		return;
	currUndo->Undo ();
	currUndo = currUndo->prev;

	int prev = unsavedActions;
	unsavedActions--;
	if (!unsavedActions || !prev)
		Ui::UpdateWindowTitle ();
}
void UndoSys::Redo ()
{
	Action *act = currUndo->next;
	if (act == actions.End())
		return;
	act->Redo ();
	currUndo = currUndo->next;

	int tmp = unsavedActions;
	unsavedActions++;
	if (!unsavedActions || !tmp)
		Ui::UpdateWindowTitle ();
}

void UndoSys::AddUndo (Action *act)
{
	// delete any redos
	Action *a = currUndo->next;
	while (a != actions.End())
	{
		Action *n = actions.Remove (a);
		delete a;
		a = n;
	}
	
	actions.Append (act);
	currUndo = act;
	int prev = unsavedActions;
	if (unsavedActions < 0) unsavedActions = 0;
	unsavedActions++;
	
	if (!prev)
		Ui::UpdateWindowTitle ();
}

UndoSys::Action* UndoSys::GetCurrentUndo ()
{
	return currUndo;
}

/*
=============
 NoUnsavedActions

 used to clear number of actions since saved
=============
*/
void
UndoSys::NoUnsavedActions ()
{
	unsavedActions = 0;
}
bool UndoSys::HasUnsavedActions ()
{
	return (unsavedActions != 0);
}



