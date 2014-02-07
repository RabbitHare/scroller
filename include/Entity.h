#ifndef __ENTITY_2_H__
#define __ENTITY_2_H__
#include <Vec3f.h>
#include <Collision.h>
#include <Camera.h>


struct Entity
{
	void (*Update) (Entity *ent);
	void (*Render) (Entity *ent, Camera &camera);
	void (*HandleCollision) (Entity *ent1, Entity *ent2, Vec3f norm);
	void (*Delete) (Entity *ent); //destructor, called by DeleteEntity

	Entity *next,*prev;
	Entity *owner; // for bullets; who fired them
	int type;
	bool used;
	float age; // in miliseconds
	bool dead;
	bool destroy; // when true, entity will be deleted
	Vec3f vel;
	Vec3f pos;
	Vec3f nextPos;
	// for collision checking
	Bounds bounds;
	int collWith; // types of objects it collides with
	CollisionResult cres;
	Entity *cent,
		*entrc;
	bool cdone;
	bool ccheck;
	float ctime;
	bool hasMoved;
	int ccycle;
};
struct EntityMem // used for allicating array
{
	Entity ent;	
	char extData[128];
};
#define ENTITY(a) ((Entity*)(a))

void InitEntity (Entity* e);
void InitEntities ();
Entity* NewEntity ();
void DeleteEntity (Entity *e);


#endif
