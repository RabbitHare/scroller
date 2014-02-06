#ifndef __ENTITY_2_H__
#define __ENTITY_2_H__
#include <Vec3f.h>
#include <Collision.h>
#include <Camera.h>


struct Entity2
{
	void (*Update) (Entity2 *ent);
	void (*Render) (Entity2 *ent, Camera &camera);
	void (*HandleCollision) (Entity2 *ent1, Entity2 *ent2, Vec3f norm);
	void (*Delete) (Entity2 *ent); //destructor, called by DeleteEntity

	Entity2 *next,*prev;
	Entity2 *owner; // for bullets; who fired them
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
	Entity2 *cent,
		*entrc;
	bool cdone;
	bool ccheck;
	float ctime;
	bool hasMoved;
	int ccycle;
};
struct EntityMem // used for allicating array
{
	Entity2 ent;	
	char extData[128];
};
#define ENTITY(a) ((Entity2*)(a))

void InitEntity (Entity2* e);
void InitEntities ();
Entity2* NewEntity ();
void DeleteEntity (Entity2 *e);


#endif
