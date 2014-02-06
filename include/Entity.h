#ifndef __ENTITY_H__
#define __ENTITY_H__
#include <Vec3f.h>
#include <Collision.h>
#include <Camera.h>

struct Layer;

class Entity
{
public:
	Entity *next,*prev; // for layer entity list

	virtual Vec3f GetPosition ()=0;
	virtual void SetPosition (const Vec3f &pos)=0;
	virtual Vec3f GetNextPosition ()=0;

	Entity ()
	{
		mLayer = NULL;
	}
	virtual ~Entity(){}
	virtual void Update ()=0;
	virtual void Render (Camera &camera) { }
	virtual void ComputeNextPosition (float time) { }
	virtual void MoveToNextPosition (CollisionResult &result) { }
	// the layer in which this entity resides
	Layer* GetLayer ()
	{ return mLayer; }
	void SetLayer (Layer *l)
	{ mLayer = l; }
private:
	Layer *mLayer;
};
#endif

