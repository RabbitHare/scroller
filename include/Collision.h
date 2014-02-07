#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <Vec3f.h>

#define HALF_AABB(t) (static_cast<HalfAabb*>(t))

enum BoundsType
{
	BT_INVALID,
	BT_AABB,
	BT_SPHERE
};
struct Bounds
{
	BoundsType type;
	Vec2f half; //for aabb
	float radius;
};


struct BasicTriangle
{
	Vec3f p[3];
	Vec3f normal;
};
struct CTri
{
	Vec2f p[3];
	Vec2f norms[3]; // edge normals
};

struct Entity;

struct CollisionResult
{
	float fraction;
	float inside;
	float epsilon;
	Vec3f normal;
	bool hasCollided;
	Entity *ent;
	inline CollisionResult ()
	{
		fraction = 1.0;
		normal = Vec3f(0,0,0);
		hasCollided = false;
		ent = NULL;
	}
};

void HalfAabbCollisionTriangle (const Bounds &aabb, const Vec3f &startPos, const Vec3f &endPos, const CTri &tri, CollisionResult &result);
void HalfAabbCollisionHalfAabb (const Bounds &aabb1, const Vec3f &startPos, const Vec3f &endPos,
				const Bounds &aabb2, const Vec3f &pos2, CollisionResult &result);
void EntityCollisionCheckWorld (Entity *ent, CollisionResult &result);
void EntityCollisionCheckMap (Entity *ent, CollisionResult &result);
void CollisionCheckWorld (); //check everything with everything


#endif

