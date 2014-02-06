#include <Bullet1.h>
#include <Collision.h>
#include <GL/gl.h>
#include <Sprite.h>
#include <GameGlobals.h>
#include <Texture.h>

static float speed = 20;
static float range = 300;
static float lifespan = range/(speed/PHYSICS_FRAME_LEN); // miliseconds


static void ComputeNextPosition (Entity2 *ent, float time);


static void UpdateCb (Entity2 *ent)
{
	if (ent->age > lifespan)
	{
		ent->destroy = true;
		return;
	}
	// move at constant speed
	ComputeNextPosition (ent, 1);
	BULLET1 (ent)->animTimer++;
}

static void ComputeNextPosition (Entity2 *ent, float time)
{
	ent->nextPos = ent->pos + ent->vel*time;
}

static void
HandleCollision (Entity2 *ent, Entity2 *ent2, Vec3f norm)
{

	//ent->vel -= norm.Project(ent->vel)*2;
	/*
	ent->vel.y += (float)(rand()%10);
	if (rand()%2)
		ent->vel.y *= -1;*/

	// delete
	ent->destroy = true;

}


int static GetAnimFrame (Bullet1 *nt, float fps)
{

	// NOTE: mAnimTimer is running at the physics fps
	return float(nt->animTimer)/PHYSICS_FPS * fps;
}
static void RenderCb (Entity2 *ent, Camera &cam)
{
	//Bullet1 *self = BULLET1 (ent);
	Vec3f r,p;
	r = ent->bounds.half;
	p = ent->pos;
	/*
	glDisable (GL_TEXTURE_2D);
	glColor3f (1.0,0.0,0.0);
	glBegin (GL_QUADS);

	glVertex3f (p.x-r.x, p.y+r.y, p.z);
	glVertex3f (p.x-r.x, p.y-r.y, p.z);
	glVertex3f (p.x+r.x, p.y-r.y, p.z);
	glVertex3f (p.x+r.x, p.y+r.y, p.z);

	glEnd ();
	glEnable (GL_TEXTURE_2D);*/

	if (!g_sprites[BULLET1_FLY].shader)
		LoadSprite (g_sprites+BULLET1_FLY, "sprites/bullet1_fly.sprt");

	Sprite *s;
	SpriteFrame *f;
	int anim;
	s = g_sprites + BULLET1_FLY;
	anim = GetAnimFrame(BULLET1 (ent), s->fps)%s->numFrames;
	f = s->frames + anim;
	glColor3f (1.,1.,1.);
	glBindTexture (GL_TEXTURE_2D, s->shader->tex);
	
	float x1,x2,y1,y2;
	int facing = 1;
	if (ent->vel.x < 0) facing = -1;
	float w = s->w/2,
	      h = s->h/2;
	x1 = p.x-w + s->pos.x*facing;
	y1 = p.y+h + s->pos.y;
	x2 = p.x+w + s->pos.x*facing;
	y2 = p.y-h + s->pos.y;

	//glColor3f (1.0,0.0,0.0);
	//glDisable (GL_CULL_FACE);
	//glBlendFunc (GL_DST_COLOR, GL_ONE);
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
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}
Bullet1* NewBullet1 ()
{

	Entity2 *ent = NewEntity ();
	Bullet1 *self = BULLET1(ent);

	ent->type = ET_BULLET;
	ent->collWith = ET_ENEMY | ET_MAP;

	ent->Update = UpdateCb;
	ent->Render = RenderCb;
	ent->HandleCollision = HandleCollision;
	BULLET1 (ent)->animTimer=0;

	ent->vel = Vec3f (speed,0.,0.);
	ent->bounds.type = BT_AABB;
	ent->bounds.half = Vec2f(2,2);


	return self;
}

