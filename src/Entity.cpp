#include <Entity2.h>
#include <GameGlobals.h>
#include <stdio.h>

EntityMem g_entityPool[MAX_ENTITIES];
Entity2 *g_entities=NULL;
int g_numEntities=0;

void InitEntities ()
{
	for (int i=0; i<MAX_ENTITIES; i++)
		InitEntity(ENTITY(g_entityPool+i));
	g_entities = ENTITY(g_entityPool+MAX_ENTITIES-1);
	// DUMMY
	g_entities->used=true; 
	g_entities->dead=false;
	g_entities->next = g_entities->prev = NULL;
}
void InitEntity (Entity2* e)
{
	e->type = ET_INVALID;
	e->collWith = 0;
	e->bounds.type = BT_AABB;
	e->bounds.half = Vec2f (10,10);
	e->owner = NULL;
	e->used = false;
	e->dead = false;
	e->destroy = false;
	e->age = 0;
	e->Delete = NULL;
	e->Update = NULL;
	e->Render = NULL;
	e->HandleCollision = NULL;
}
// Finds unused entity, initalizes it, and adds to global list of entities
Entity2* NewEntity ()
{
	if (g_entities == NULL)
		InitEntities ();

	Entity2 *e = NULL;
	for (int i=0; i < MAX_ENTITIES; i++)
		if (!ENTITY(g_entityPool+i)->used)
		{
			e = ENTITY(g_entityPool+i);
			break;
		}
	if (!e)
	{
		std::cerr << "MAX ENTITIES REACHED\n";
		exit (0);
	}

	InitEntity (e);
	e->used = true;
	e->dead = false;
	// add to global list
	e->next = g_entities;
	e->prev = NULL;
	g_entities->prev = e;
	g_entities = e;
	g_numEntities++;
	return e;
}
void DeleteEntity (Entity2 *e)
{
	if (e->Delete)
		e->Delete (e);
	// remove from list
	if (e == g_entities)
	{
		g_entities = e->next;
		e->prev = NULL;
	}
	else
	{
		if (e->prev)
			e->prev->next = e->next;
		if (e->next)
			e->next->prev = e->prev;
	}
	e->used = false;
	g_numEntities--;
}

	
