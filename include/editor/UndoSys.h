#ifndef __UNDO_SYS_H__
#define __UNDO_SYS_H__

#include <Entity2.h>

struct Layer;
struct MapObj;

namespace UndoSys
{

class Action
{
public:
	virtual void Undo ()=0;
	virtual void Redo ()=0;
	Action *next,*prev;
};
class TileAction: public Action
{
public:
	TileAction (Layer *l)
	{
		layer = l;
		tiles = numTiles = 0;
	}
	void Do ();
	virtual void Undo ();
	virtual void Redo ();
	void AddTile (int t);
	int tiles;
	int numTiles;
	Layer *layer; // the layer these tiles belong to
};

class AddMapObjAction: public Action
{
public:
	AddMapObjAction (MapObj *o)
	{
		obj = o;
	}
	~AddMapObjAction ()
	{
		DeleteEntity (ENTITY (obj));
	}
	virtual void Undo ();
	virtual void Redo ();

	MapObj *obj;
};

class RemoveMapObjAction: public Action
{
public:
	RemoveMapObjAction (MapObj *o)
	{
		obj = o;
	}
	~RemoveMapObjAction ()
	{
		DeleteEntity (ENTITY (obj));
	}
	virtual void Undo ();
	virtual void Redo ();

	MapObj *obj;
};
	

void Undo ();
void Redo ();
void AddUndo (Action *act);
Action* GetCurrentUndo ();
void NoUnsavedActions ();
bool HasUnsavedActions ();

};

#endif

