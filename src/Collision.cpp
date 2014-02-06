#include <Collision.h>
#include <Entity2.h>
#include <Map.h>
#include <GameGlobals.h>

static const Vec3f boxNorms[] = {Vec3f(1.0,0.0,0.0),
			    Vec3f(-1.0,0.0,0.0),
			    Vec3f(0.0,1.0,0.0),
			    Vec3f(0.0,-1.0,0.0),
			    Vec3f(0.0,0.0,1.0),
			    Vec3f(0.0,0.0,-1.0)};
static const int NUM_BOX_NORMS = 6;
static Vec2f triBoxNorms[] = {
				Vec2f(0.,0.), // for tri
				Vec2f(0.,0.), // for tri
				Vec2f(0.,0.), // for tri
				Vec2f(1.,0.), // read only
				Vec2f(0.,1.)}; // read only

void HalfAabbCollisionTriangle (const Bounds &aabb, const Vec3f &startPos, const Vec3f &endPos,
				const CTri &tri, CollisionResult &result)
{
	float epsilon = 0.0;
	float invalid = 1000.0;
	float first=-invalid,last=invalid,
	      enter,exit,
		min,max,
		start,end;
	float radias;
	float invdist;
	result.hasCollided = false;
	result.inside = false;
	result.fraction = 1.0;
	result.normal = Vec3f(0,0,0);


	triBoxNorms[0] = tri.norms[0];
	triBoxNorms[1] = tri.norms[1];
	triBoxNorms[2] = tri.norms[2];
	bool hasCollided = false;
	Vec2f norm,collNorm;
	Vec2f startV(startPos.x,startPos.y),
	      endV(endPos.x,endPos.y);
	// box planes
	for (int i = 0; i < 5; i++)
	{
		enter = -invalid;
		exit = invalid;
		norm = triBoxNorms[i];
		min = norm.Dot (tri.p[0]);
		max = min;
		for (int i = 1; i < 3; i++)
		{
			float t  = norm.Dot (tri.p[i]);
			if (t > max)
				max = t;
			else if (t < min)
				min = t;
		}
		radias = Vec2f::Abs(norm).Dot(aabb.half);
		min -= radias;
		max += radias;

		start = norm.Dot (startV);
		end = norm.Dot (endV);
		invdist = end - start;
		invdist = fabs (invdist);
		if (invdist == 0.0)
		{
			invdist = 1.0;
		}
		else invdist = 1.0/invdist;
		
		float ep = fabs(epsilon * invdist);
		float Ep = 0.0;
		if (start < min + Ep)
		{
			if (end < min + Ep)
				return;
			enter = (min - start) * invdist;
			exit = (max - start) * invdist;

			if (enter < 0.) enter = -enter;
			enter -= ep;
		}
		else if (start > max - Ep)
		{
			if (end > max - Ep)
				return;
			enter = (start - max) * invdist;
			exit = (min - start) * invdist;

			if (enter < 0.) enter = -enter;
			enter -= ep;
		}
		else if (start < max && end > max)
				exit = (max - start) * invdist;
		else if (start > min && end < min)
				exit = (min - start) * invdist;

		else
			continue;
		
		if (exit < 0.) exit = -exit;
		exit -= ep;
		if (enter > first)
		{
			first = enter;
			collNorm = norm;
			hasCollided = true;
		}
		if (exit < last)
			last = exit;

	}

	if (last < first)
		return;

	if (hasCollided)
	{
		result.fraction = first;
		result.normal = collNorm;
	}
	else return;
	// flip normal
	if (result.normal.Dot(startPos - endPos) < 0)
		result.normal = -result.normal;
	result.hasCollided = true;
	return;
}

void HalfAabbCollisionHalfAabb (const Bounds &aabb1, const Vec3f &startPos, const Vec3f &endPos,
				const Bounds &aabb2, const Vec3f &pos2, CollisionResult &result)
{
	float epsilon = 0.0;
	float invalid = 1000.0;
	float first=-invalid,last=invalid,
	      enter,exit, pos, min, max,
		start,end;
	float radias;
	float invdist;
	result.hasCollided = false;
	result.inside = false;
	result.fraction = 1.0;
	result.normal = Vec3f(0,0,0);


	bool hasCollided = false;
	Vec2f norm,collNorm;
	Vec2f startV(startPos.x,startPos.y),
	      endV(endPos.x,endPos.y);
	bool inside=true;
	// box planes
	for (int i = 3; i < 5; i++)
	{
		enter = -invalid;
		exit = invalid;
		norm = triBoxNorms[i];
		radias = Vec2f::Abs(norm).Dot(aabb1.half) + Vec2f::Abs(norm).Dot(aabb2.half);
		pos = norm.Dot(pos2);
		min = pos - radias;
		max = pos + radias;

		start = norm.Dot (startV);
		end = norm.Dot (endV);
		invdist = end - start;
		if (fabs(invdist) == 0.0)
		{
			invdist = 1.0;
		}
		else invdist = 1.0/invdist;
		
		float ep = fabs(epsilon * invdist);
		float Ep = 0.0;
		if (start < min + Ep)
		{
			if (end < min + Ep)
				return;
			enter = (min - start) * invdist;
			exit = (max - start) * invdist;

			if (enter < 0.) enter = -enter;
			enter -= ep;
			inside = false;
		}
		else if (start > max - Ep)
		{
			if (end > max - Ep)
				return;
			enter = (start - max) * invdist;
			exit = (min - start) * invdist;

			if (enter < 0.) enter = -enter;
			enter -= ep;

			inside = false;
		}
		else if (start < max && end > max)
				exit = (max - start) * invdist;
		else if (start > min && end < min)
				exit = (min - start) * invdist;

		else
			continue;
		

		if (exit < 0.) exit = -exit;
		exit -= ep;
		if (enter > first)
		{
			first = enter;
			collNorm = norm;
			hasCollided = true;
		}
		if (exit < last)
			last = exit;

	}
	if (last < first)
		return;

	if (hasCollided)
	{
		result.fraction = first;
		result.normal = collNorm;
	}
	else return;
	// flip normal
	if (result.normal.Dot(startPos - endPos) < 0)
		result.normal = -result.normal;
	result.hasCollided = true;
	return;
}

void
EntityCollisionCheckWorld (Entity2 *ent1, CollisionResult &result)
{
	Map &map = *GetMap();
	CollisionResult res;
	Entity2 *e = g_entities;
	while (e)
	{
		
		if (e->dead)
			e->cdone = true;
		else if (e->bounds.type != BT_AABB)
			e->cdone = true;
		else e->cdone = false;

		e->cres.fraction = 1000.;
		e->cres.hasCollided = false;
		e->cres.ent = NULL;
		e->cres.ent = NULL;
		e->entrc = NULL;
		e->ccheck=false;
		e->ccycle=0;

		e = e->next;
	}
	
	result.hasCollided = false;
	result.fraction = 1000.;
	Entity2 *ent2 = g_entities;
	while (ent2)
	{
		if (ent2 == ent1 || ent2->cdone)//  || ent2->ccycle > cycle)
		{
			ent2 = ent2->next;
			continue;
		}
		Vec3f end;

		end = ent1->nextPos;

		HalfAabbCollisionHalfAabb (ent1->bounds, ent1->pos, end, ent2->bounds, ent2->pos, res);
		if (res.hasCollided && res.fraction < result.fraction)
		{
			result = res;
			result.ent = ent2;

		}
		ent2 = ent2->next;
	}
	// TEST MAP
	map.CollisionCheck (*ent1, res);
	if (res.hasCollided && res.fraction < result.fraction)
	{
		result = res;
		res.ent = NULL;
	}

	if (!result.hasCollided) result.fraction = 1.;


}

void EntityCollisionCheckMap (Entity2 *ent, CollisionResult &result)
{
	GetMap()->CollisionCheck (*ent, result);
}

void CollisionCheckWorld ()
{
	Map &map = *GetMap();
	Entity2 *ent = g_entities;
	int i;
	while (ent)
	{

		if (ent->dead)
			ent->cdone = true;
		else if (ent->bounds.type != BT_AABB)
			ent->cdone = true;
		else ent->cdone = false;

		ent->cres.fraction = 1000.;
		ent->cres.hasCollided = false;
		ent->cres.ent = NULL;
		ent->entrc = NULL;
		ent->ccheck=false;
		ent->ccycle=0;
		ent->ctime=1;

		ent = ent->next;
	}
	Entity2 *ent1,*ent2,
		*cent1,*cent2;
	ent1 = g_entities;
	CollisionResult res,result;
	result.fraction = 1000.;
	cent1 = cent2 = NULL;
	i=4;
	int cycle = 0;
	while (1)
	{
		if (!ent1)
		{
			i--;
			if (!i) break;

			cycle++;
			ent1 = g_entities;
		}

		if (ent1->cdone || ent1->ctime < 0.1)
		{
			ent1 = ent1->next;
			continue;
		}
		ent1->ccycle++;
		ent2 = g_entities;
		result.fraction = 1000.;
		result.hasCollided = false;
		result.ent = NULL;
		// TEST OTHER ENTITIES
		while (ent2)
		{
			if (ent2 == ent1 || ent2->owner == ent1  || ent1->owner == ent2 || ent2->cdone)//  || ent2->ccycle > cycle)
			{
				ent2 = ent2->next;
				continue;
			}
			Vec3f end;

			end = ent1->nextPos;

			HalfAabbCollisionHalfAabb (ent1->bounds, ent1->pos, end, ent2->bounds, ent2->pos, res);
			if (res.hasCollided && res.fraction < result.fraction)
			{
				result = res;
				result.ent = ent2;

			}
			ent2 = ent2->next;
		}
		// TEST MAP
		map.CollisionCheck (*ent1, res);
		if (res.hasCollided && res.fraction < result.fraction)
		{
			result = res;
			result.ent = NULL;
		}

		if (result.hasCollided)
		{

			const float epsilon = .03;
			Vec3f norm = result.normal;
			float dist = fabs(norm.Dot (ent1->nextPos - ent1->pos));
			float invdist;
			if (dist == 0.0)
				invdist = 1;
			else invdist = 1./dist;
			float ep = fabs(epsilon * invdist);
			result.fraction = result.fraction - ep;
			if (result.fraction < 0.0) result.fraction = 0.;
		}
		else result.fraction = 1.;
		
		ent1->ctime -= result.fraction;

		// MOVE ENTITY
		ent1->pos += (ent1->nextPos - ent1->pos) * result.fraction;


		// ALLOW ENTITY TO RESPOND TO COLLISION
		if (result.hasCollided)
		{
			Vec3f vel1 = ent1->vel;
			if (ent1->HandleCollision)
				ent1->HandleCollision (ent1, result.ent, result.normal);
			Vec3f vel2 = ent1->vel;
			ent1->vel = vel1; // allow entity to respond to impact velocity
			if (result.ent && result.ent->HandleCollision)
				result.ent->HandleCollision (result.ent, ent1, -result.normal);
			// set back to new velocity
			ent1->vel = vel2;

			if (result.ent)
				result.ent->nextPos = result.ent->pos + result.ent->vel * result.ent->ctime;
		}
		// COMPUTE NEXT POSITION
		ent1->nextPos = ent1->pos + ent1->vel * ent1->ctime;


		ent1 = ent1->next;


	}
}

