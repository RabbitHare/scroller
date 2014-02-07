#include <NormTurt.h>
#include <Collision.h>
#include <GL/gl.h>
#include <Sprite.h>
#include <GameGlobals.h>
#include <Texture.h>
#include <TurtleShell.h>

static float gravity = 0.5;
static float maxFallSpeed = -25.0;
static float walkSpeed = 1.;

static void ComputeNextPosition (Entity *ent, float time);

static void TestGround (Entity *ent)
{
	static const float GROUND_TEST_DIST = 5.;
	// check to see if there's ground below but a ways ahead and turn around if ground ends
	float posx = ent->pos.x;
	int facing = 1;
	if (ent->vel.x < 0) facing = -1;
	ent->pos.x += ent->bounds.half.x*2*facing;
	ent->nextPos = ent->pos;
	ent->nextPos.y -= GROUND_TEST_DIST;

	CollisionResult result;
	EntityCollisionCheckMap (ent, result);
	ent->pos.x = posx; // return to original position

	if (!result.hasCollided)
	{
		// change directions
		ent->vel.x = -ent->vel.x;
	}


}

static void UpdateCb (Entity *ent)
{
	TestGround (ent);

	if (ent->vel.y > maxFallSpeed)
	{
		ent->vel.y -= gravity;
		if (ent->vel.y < maxFallSpeed)
			ent->vel.y = maxFallSpeed;
	}
	// move at constant speed
	if (ent->vel.x < 0.) ent->vel.x = -walkSpeed;
	else ent->vel.x = walkSpeed;
	ComputeNextPosition (ent, 1);
	NORM_TURT (ent)->animTimer++;
}

static void ComputeNextPosition (Entity *ent, float time)
{
	ent->nextPos = ent->pos + ent->vel*time;
}

static void
HandleCollision (Entity *ent, Entity *ent2, Vec3f norm)
{

	if (ent2 && ent2->type & ET_BULLET && !ent->destroy)
	{

		ent->destroy = true;
		// create empty shell
		TurtleShell *ts = NewTurtleShell ();
		ts->ent.pos = ent->pos;
		ts->ent.nextPos = ts->ent.pos;
		ts->ent.vel = ent2->vel * .5;
		ts->ent.vel.y += 5.;
		return;
	}

	if (norm.Dot (ent->vel) < 0) // moving torwards entity
	{
		// velocity
		if (fabs(norm.y) > 0.5) // slide along floors
			ent->vel -= norm.Project(ent->vel)*1.5;
		else // bounce off walls
			ent->vel -= norm.Project(ent->vel)*1.2;

		ent->vel.z = 0.;
	}
}

int static GetAnimFrame (NormTurt *nt, float fps)
{

	// NOTE: mAnimTimer is running at the physics fps
	return float(nt->animTimer)/PHYSICS_FPS * fps;
}
static void RenderCb (Entity *ent, Camera &cam)
{
	//NormTurt *self = NORM_TURT (ent);
	Vec3f r,p;
	r = ent->bounds.half;
	p = ent->pos;
	if (DRAW_BOUNDS)
	{
		glDisable (GL_TEXTURE_2D);
		glColor3f (1.0,0.0,0.0);
		glBegin (GL_QUADS);

		glVertex3f (p.x-r.x, p.y+r.y, p.z);
		glVertex3f (p.x-r.x, p.y-r.y, p.z);
		glVertex3f (p.x+r.x, p.y-r.y, p.z);
		glVertex3f (p.x+r.x, p.y+r.y, p.z);

		glEnd ();
		glEnable (GL_TEXTURE_2D);
	}
	if (!g_sprites[TURTLE_WALK].shader)
		LoadSprite (g_sprites+TURTLE_WALK, "sprites/turtle_walk.sprt");

	Sprite *s;
	SpriteFrame *f;
	int anim;
	s = g_sprites+TURTLE_WALK;
	anim = GetAnimFrame(NORM_TURT (ent), s->fps)%s->numFrames;
	f = s->frames + anim;
	glColor3f (1.,1.,1.);
	glBindTexture (GL_TEXTURE_2D, s->shader->tex);

	float x1,x2,y1,y2;
	int facing = 1;
	if (ent->vel.x < 0) facing = -1;
	x1 = p.x-s->w/2 + s->pos.x*facing;
	y1 = p.y-r.y+s->h + s->pos.y;
	x2 = p.x+s->w/2 + s->pos.x*facing;
	y2 = p.y-r.y + s->pos.y;

	//glColor3f (1.0,0.0,0.0);
	//glDisable (GL_CULL_FACE);
	glBegin (GL_QUADS);

	if (facing > 0)
	{
		glTexCoord2f (f->p1.x,f->p1.y);glVertex3f (x1, y1, p.z);	

		glTexCoord2f (f->p1.x,f->p2.y);glVertex3f (x1, y2, p.z);

		glTexCoord2f (f->p2.x,f->p2.y);glVertex3f (x2, y2, p.z);

		glTexCoord2f (f->p2.x,f->p1.y);glVertex3f (x2, y1, p.z);
	}
	else
	{
		glTexCoord2f (f->p2.x,f->p1.y);glVertex3f (x1, y1, p.z);	

		glTexCoord2f (f->p2.x,f->p2.y);glVertex3f (x1, y2, p.z);

		glTexCoord2f (f->p1.x,f->p2.y);glVertex3f (x2, y2, p.z);

		glTexCoord2f (f->p1.x,f->p1.y);glVertex3f (x2, y1, p.z);
	}

	glEnd ();

}
NormTurt* NewNormTurt ()
{

	Entity *ent = NewEntity ();
	NormTurt *self = NORM_TURT(ent);

	ent->type = ET_ENEMY;
	ent->collWith = ET_PLAYER | ET_MAP;

	ent->Update = UpdateCb;
	ent->Render = RenderCb;
	ent->HandleCollision = HandleCollision;
	NORM_TURT (ent)->animTimer=0;

	ent->vel = Vec3f (2.5,0.,0.);
	ent->bounds.type = BT_AABB;
	ent->bounds.half = Vec2f(30,20);


	return self;
}

