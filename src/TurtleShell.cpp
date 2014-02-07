#include <TurtleShell.h>
#include <Collision.h>
#include <GL/gl.h>
#include <Sprite.h>
#include <GameGlobals.h>
#include <Texture.h>

static float gravity = 0.25;
static float maxFallSpeed = -25.0;


static void ComputeNextPosition (Entity *ent, float time);


static void UpdateCb (Entity *ent)
{
	if (ent->vel.y > maxFallSpeed)
	{
		ent->vel.y -= gravity;
		if (ent->vel.y < maxFallSpeed)
			ent->vel.y = maxFallSpeed;
	}

	ComputeNextPosition (ent, 1);
	TURTLE_SHELL (ent)->animTimer++;
}

static void ComputeNextPosition (Entity *ent, float time)
{
	ent->nextPos = ent->pos + ent->vel*time;
}

static void
HandleCollision (Entity *ent, Entity *ent2, Vec3f norm)
{

	if (ent2 && fabs(norm.x) > .8)
	{
		ent->vel.x += ent2->vel.x;
	}
	if (norm.Dot (ent->vel) < 0) // moving torwards entity
	{
		// velocity
		if (fabs(norm.y) > 0.5) // slide along floors
			ent->vel -= norm.Project(ent->vel)*1.5;
		else // bounce off walls
			ent->vel -= norm.Project(ent->vel)*1.2;
		if (ent2 && (ent2->type & ET_PLAYER) && fabs(norm.x) > .8)
		{
			ent->vel.x += ent2->vel.x*1.2;
		}

		ent->vel.z = 0.;
	}

}



int static GetAnimFrame (TurtleShell *nt, float fps)
{

	// NOTE: mAnimTimer is running at the physics fps
	return float(nt->animTimer)/PHYSICS_FPS * fps;
}
static void RenderCb (Entity *ent, Camera &cam)
{
	//TurtleShell *self = TURTLE_SHELL (ent);
	Vec3f r,p;
	r = ent->bounds.half;
	p = ent->pos;

	if (DRAW_BOUNDS)
	{
		// draw bounds
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

	if (!g_sprites[TURTLE_SHELL].shader)
		LoadSprite (g_sprites+TURTLE_SHELL, "sprites/turtle_shell.sprt");

	Sprite *s;
	SpriteFrame *f;
	int anim;
	s = g_sprites + TURTLE_SHELL;
	anim = GetAnimFrame(TURTLE_SHELL (ent), s->fps)%s->numFrames;
	f = s->frames + anim;
	glColor3f (1.,1.,1.);
	glBindTexture (GL_TEXTURE_2D, s->shader->tex);

	float x1,x2,y1,y2;
	float w = s->w/2,
	      h = s->h/2;
	x1 = p.x-w + s->pos.x;
	y1 = p.y+h + s->pos.y;
	x2 = p.x+w + s->pos.x;
	y2 = p.y-h + s->pos.y;

	//glColor3f (1.0,0.0,0.0);
	//glDisable (GL_CULL_FACE);
	//glBlendFunc (GL_DST_COLOR, GL_ONE);
	glBegin (GL_QUADS);

	glTexCoord2f (f->p1.x,f->p2.y);glVertex3f (x1, y1, p.z);	

	glTexCoord2f (f->p1.x,f->p1.y);glVertex3f (x1, y2, p.z);

	glTexCoord2f (f->p2.x,f->p1.y);glVertex3f (x2, y2, p.z);

	glTexCoord2f (f->p2.x,f->p2.y);glVertex3f (x2, y1, p.z);

	glEnd ();
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}
TurtleShell* NewTurtleShell ()
{

	Entity *ent = NewEntity ();
	TurtleShell *self = TURTLE_SHELL(ent);

	ent->type = ET_BULLET | ET_SHELL;
	ent->collWith = ET_ENEMY | ET_PLAYER | ET_MAP | ET_BULLET;

	// callbacks
	ent->Update = UpdateCb;
	ent->Render = RenderCb;
	ent->HandleCollision = HandleCollision;

	TURTLE_SHELL (ent)->animTimer=0;

	ent->bounds.type = BT_AABB;
	ent->bounds.half = Vec2f(20,8);


	return self;
}

