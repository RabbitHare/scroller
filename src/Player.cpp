#include <Player.h>
#include <GL/gl.h>
#include <Map.h>
#include <GameGlobals.h>
#include <Texture.h>
#include <Sprite.h>
#include <Bullet1.h>
#include <TurtleShell.h>

Player g_players[4];
int g_numPlayers=0;

static float walkAccel = 0.5;
static float runInitAccel = 0.6;
static float runAccel = 0.7;
static float flyAccel = 1.0;
static float maxWalkSpeed = 8.0;
static float maxRunSpeed = 15.0;
static float maxAirRunSpeed = 10.0;
static float maxFallSpeed = -25.0;
static float maxFlySpeed = 5.0;
static float jumpAccel = 4.5;
static float friction = 0.1;
static float groundFriction = 0.3;
static float gravity = 0.5;


static void rightCb (int kstate, Player *player)
{

	player->ActionWalk (kstate, 1);
}
static void leftCb (int kstate, Player *player)
{

	player->ActionWalk (kstate,-1);
}
static void runCb (int kstate, Player *player)
{
	player->ActionRun (kstate);
}
static void downCb (int kstate, Player *player)
{
	player->ActionDown (kstate);
}
/*
static void flyRightCb (int kstate, Player *player)
{
	player->ActionFly (kstate, Vec3f (1.0,0,0));
}
static void flyLeftCb (int kstate, Player *player)
{
	player->ActionFly (kstate,Vec3f (-1.0,0,0));
}
static void flyDownCb (int kstate, Player *player)
{
	player->ActionFly (kstate, Vec3f (0,-1.,0));
}
*/
static void flyUpCb (int kstate, Player *player)
{
	player->ActionFly (kstate,Vec3f (0,1.,0));
}
static void jumpCb (int kstate, Player *player)
{
	player->ActionJump (kstate);
}
static void fireCb (int kstate, Player *player)
{
	player->ActionFire (kstate);
}

static void RenderCb (Entity *ent, Camera &cam) { ((PlayerEntity*)ent)->player->Render(cam); }

static void UpdateCb (Entity *ent) { ((PlayerEntity*)ent)->player->Update(); }


static void HandleCollisionCb (Entity *ent1, Entity *ent2, Vec3f norm) { ((PlayerEntity*)ent1)->player->HandleCollision(ent2, norm); }


// must be length of NUM_PLAYER_COMMANDS+1
static PlayerCommand playerCommands[] = {
	{rightCb,"right"},
	{leftCb,"left"},
	{flyUpCb,"up"},
	{downCb,"down"},
	{runCb,"run"},
	{jumpCb,"jump"},
	{fireCb,"fire"},
	{NULL,"cycle"},
	{NULL,NULL}};

PlayerCommand* GetPlayerCommands()
{
	return playerCommands;
}
Player* NewPlayer ()
{
	if (g_numPlayers == MAX_PLAYERS)
	{
		std::cerr << "MAX NUMBER OF PLAYERS REACHED\n";
		return NULL;
	}
	Player *p = g_players+g_numPlayers;
	g_numPlayers++;
	p->Init();
	return p;
}

void Player::Init ()
{
	ent = NewEntity();
	((PlayerEntity*)ent)->player = this;
	ent->type = ET_PLAYER;
	ent->collWith = ET_ENEMY | ET_MAP | ET_SHELL;
	ent->bounds.type = BT_AABB;
	ent->bounds.half = Vec2f(16,20);

	ent->Update = UpdateCb;
	ent->Render = RenderCb;
	ent->HandleCollision = HandleCollisionCb;

	SetPosition(Vec3f(0,0,0));
	SetVelocity(Vec3f(0,0,0));

	mIsMoving = false;
	mIsRunning = false;
	mIsGrounded = false;
	mIsCrouched = false;
	mJumpCounter = 0;
	mRunCounter = 0;
	mFacing = 1;
	mInputType = KEYBOARD1;
	mAnim = ANIM_NONE;
	mAnimTimer = 0;
	mSpritesLoaded = false;

}
void Player::FocusCamera ()
{
	mCamera.mat.SetTranslation (Vec3f(GetPosition().x,GetPosition().y, mCamera.mat.GetTranslation().z));
}
void Player::Render (Camera &camera)
{
	if (!mSpritesLoaded)
	{
		mSpritesLoaded = true;		
		LoadSprites ();
	}

	Sprite *s;
	SpriteFrame *f;
	int anim;
	if (mAnim == ANIM_IDLE)
	{
		s = g_sprites+JAZZ_IDLE_1;
		anim = GetAnimFrame(s->fps)%s->numFrames;
		f = s->frames + anim;
	}
	else if (mAnim == ANIM_WALK)
	{
		s = g_sprites+JAZZ_WALK;
		anim = GetAnimFrame(s->fps)%s->numFrames;
		f = s->frames + anim;
	}
	else if (mAnim == ANIM_JUMP_FORWARD)
	{
		s = g_sprites+JAZZ_JUMP_FORWARD;
		anim = GetAnimFrame(s->fps);
		if (anim >= s->numFrames)
		{
			mAnim = ANIM_FALL_FORWARD;
			mAnimTimer = 0;
			anim = s->numFrames-1;
		}
		f = s->frames + anim;
	}
	else if (mAnim == ANIM_FALL_FORWARD)
	{
		s = g_sprites+JAZZ_FALL_FORWARD;
		anim = GetAnimFrame(s->fps)%s->numFrames;
		f = s->frames + anim;
	}
	else if (mAnim == ANIM_DASH)
	{
		s = g_sprites+JAZZ_DASH;
		anim = GetAnimFrame(s->fps)%s->numFrames;
		f = s->frames + anim;
	}
	else if (mAnim == ANIM_BREAK)
	{
		s = g_sprites+JAZZ_BREAK;
		anim = GetAnimFrame(s->fps);
		if (anim >= s->numFrames)
		{
			anim = 0;
			s = g_sprites+JAZZ_IDLE_1;
			mAnim = ANIM_IDLE;
			mAnimTimer = 0;
		}
		f = s->frames + anim;
	}
	else return;



	Vec3f r,p;
	r = ent->bounds.half;
	p = GetPosition();
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

	glColor3f (1.,1.,1.);
	glBindTexture (GL_TEXTURE_2D, s->shader->tex);

	float x1,x2,y1,y2;
	x1 = p.x-s->w/2 + s->pos.x*mFacing;
	y1 = p.y-r.y+s->h + s->pos.y;
	x2 = p.x+s->w/2 + s->pos.x*mFacing;
	y2 = p.y-r.y + s->pos.y;

	//glColor3f (1.0,0.0,0.0);
	//glDisable (GL_CULL_FACE);
	glBegin (GL_QUADS);

	if (mFacing > 0)
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
void Player::Update ()
{
	
	TestGround (); // see whether there is ground right below

	HandleKeys ();


	if (GetVelocity().y > maxFallSpeed)
	{
		GetVelocity().y -= gravity;
		if (GetVelocity().y < maxFallSpeed)
			GetVelocity().y = maxFallSpeed;
	}
	if (!mIsMoving)
	{
		if (mIsGrounded && mGroundDist < 1.)
		{
			if (fabs (GetVelocity().x) <= groundFriction)
				GetVelocity().x = 0.;
			else if (GetVelocity().x < 0)
				GetVelocity().x += groundFriction;
			else if (GetVelocity().x > 0)
				GetVelocity().x -= groundFriction;
			// animation
			if (mAnim != ANIM_IDLE && mAnim != ANIM_BREAK)
			{
				mAnim = ANIM_IDLE;
				mAnimTimer = 0;
			}
		}
		if (fabs (GetVelocity().x) < 0.01)
			GetVelocity().x = 0.;
		else if (GetVelocity().x < 0)
			GetVelocity().x += friction;
		else if (GetVelocity().x > 0)
			GetVelocity().x -= friction;

	}
	//ANIMATION
	if (!mIsGrounded && !mIsMoving && mAnim != ANIM_JUMP_FORWARD)
	{
		mAnim = ANIM_FALL_FORWARD;
		mAnimTimer = 0;
	}
	// UPDATE CAMERA POSITION
	Vec3f cpos = GetPosition();
	cpos.z = mCamera.mat.GetOrigin().z;
	mCamera.mat.SetOrigin (cpos);


	ComputeNextPosition (1.0);
	mAnimTimer++;
}
void
Player::ComputeNextPosition (float time)
{
	GetNextPosition() = GetPosition() + GetVelocity()*time;
}

void Player::HandleCollision (Entity *ent2, Vec3f norm)
{
	if (norm.Dot (GetVelocity()) < 0) // heading torwards entity
	{
		if (ent2 && ent2->type & ET_ENEMY)
		{
			GetVelocity() -= norm.Project(GetVelocity())*1.2;
			float xvel = 5.0;
			float yvel = 10.0;
			ent->vel.x = norm.x*xvel;
			ent->vel.y = norm.y*yvel;
			GetVelocity().z = 0.;

		}
		else
		{

			// velocity
			if (fabs(norm.y) > 0.5) // slide along floors
				GetVelocity() -= norm.Project(GetVelocity())*1.1;
			else // bounce off walls slightly
				GetVelocity() -= norm.Project(GetVelocity())*1.2;

			GetVelocity().z = 0.;
		}
	}
}


void
Player::ActionWalk (int kstate, int dir)
{
	float s;
	float fdir = (float)dir;
	if (kstate & KS_PRESSED)
	{
		// handle animation
		if (mAnim != ANIM_WALK && mIsGrounded && !mIsRunning)
		{
			mAnim = ANIM_WALK;
			mAnimTimer=0;
		}

		if (mIsRunning)
		{
			float accel,
			      max;
			if (mIsGrounded)
			{
				s = 1.0;
				max = maxRunSpeed;
			}
			else 
			{
				s = 0.7;
				max = maxAirRunSpeed;
			}
			
			if (GetVelocity().x*fdir < maxWalkSpeed)
				accel = runInitAccel;
			else accel = runAccel*GetVelocity().x/maxRunSpeed*fdir;
			float v = GetVelocity().x + accel*fdir*s;
			if (v*fdir >= max)
			{
				if (GetVelocity().x*GetVelocity().x > v*v)
					GetVelocity().x = v;
			}
			else GetVelocity().x = v;
		}
		else // walk
		{
			if (mIsGrounded)
				s = 1.0;
			else s = 0.9;
			float v = GetVelocity().x + walkAccel*fdir*s;
			if (v >= maxWalkSpeed)
			{
				if (GetVelocity().x > v)
					GetVelocity().x = v;
				else if (GetVelocity().x < v)
					GetVelocity().x = maxWalkSpeed;
			}
			else if (v <= -maxWalkSpeed)
			{
				if (GetVelocity().x < v)
					GetVelocity().x = v;
				else if (GetVelocity().x > v)
					GetVelocity().x = -maxWalkSpeed;
			}
			else GetVelocity().x = v;
		}
		// move with the down slope
		if (mIsGrounded && mGroundDist < 0.01)
		{
			static const float MAX_SLOPE_ANGLE = 30.0/180.0 * PI;
			static const Vec2f MAX_SLOPE(cos(MAX_SLOPE_ANGLE), sin(MAX_SLOPE_ANGLE));
			Vec2f n = mGroundNorm;
			if (n.x > 0 && n.x > MAX_SLOPE.x)
				n = MAX_SLOPE;
			else if (n.x < 0 && n.x < -MAX_SLOPE.x)
			{
				n.x = -MAX_SLOPE.x;
				n.y = MAX_SLOPE.y;
			}

			float t = -n.Dot (GetVelocity());
			if (t < 0.) // down slope
			{
				Vec3f vel = GetVelocity() + (n*t);
				vel *=  GetVelocity().x/vel.x;
				GetVelocity() = vel;
			}
		}
		mIsMoving = true;
		mFacing = dir;
	}
	else if (kstate & KS_RELEASED)
	{
		mIsMoving = false;

		//ANIMATION
		if (mAnim != ANIM_BREAK && mIsGrounded)
		{
			mAnim = ANIM_BREAK;
			mAnimTimer = 0;
		}
	}
}

void
Player::ActionDown (int kstate)
{

	if (kstate & KS_PRESSED)
		mIsCrouched = true;
	else if (kstate & KS_RELEASED)
		mIsCrouched = false;
}

void
Player::ActionRun (int kstate)
{
	if (kstate & KS_PRESSED)
	{
		if (mIsRunning == false)
		{
			mIsRunning = true;
			mRunCounter = 1;
		}
		else if (mRunCounter < 100)
			mRunCounter++;
		//ANIMATION
		if (mAnim != ANIM_DASH && mIsGrounded)
		{
			mAnim = ANIM_DASH;
			mAnimTimer = 0;
		}
	}
	else if (kstate & KS_RELEASED)
	{
		mIsRunning = false;
		mRunCounter = 0;
	}
}

void
Player::ActionFly (int kstate, Vec3f dir)
{
	if (kstate & KS_PRESSED)
	{
		Vec3f vel = GetVelocity() + dir*flyAccel;
		float len = vel.Normalize ();
		if (len > maxFlySpeed)
			vel *= maxFlySpeed;
		else vel *= len;
		GetVelocity() = vel;
		mIsMoving = true;
	}
	else if (kstate & KS_RELEASED)
		mIsMoving = false;
}
void
Player::ActionJump (int kstate)
{
	if (kstate & KS_PRESSED)
	{
		if (mIsGrounded && mJumpCounter < 0)
		{
			mJumpCounter = 0;
			if (GetVelocity().y > 0)
				GetVelocity().y *= 0.3;
			else if (GetVelocity().y < 0.)
				GetVelocity().y = 0.;
			GetVelocity().y += fabs(GetVelocity().x*0.3);

		}
		if (mJumpCounter > -1 && mJumpCounter < 5)
		{



			float s = 0.2;
			if (mJumpCounter > 2)
			{
				float jmp = (float)(mJumpCounter-2);
				s = 1.0/jmp*jmp;
			}
			GetVelocity().y += jumpAccel*s;

			mJumpCounter++;
		}
		if (mJumpCounter >= 5 && mIsGrounded)
			mJumpCounter = -1;
		// ANIMATION
		if (mAnim != ANIM_JUMP_FORWARD && mIsGrounded)
		{	
			mAnim = ANIM_JUMP_FORWARD;
			mAnimTimer = 0;
		}
	}
	else if (kstate & KS_RELEASED)
		mJumpCounter = -1;
}
void
Player::ActionFire (int kstate)
{
	if (kstate & KS_PRESSED)
	{
		if (!mFireCounter)
		{
			mFireCounter=1;
			/*
			TurtleShell *b = NewTurtleShell ();
			b->ent.vel.x *= mFacing;
			b->ent.vel.x += ent->vel.x;
			b->ent.vel.y += 10.;
			b->ent.pos = Vec3f (ent->pos.x, ent->pos.y+40, ent->pos.z);
			b->ent.nextPos = b->ent.pos;
			*/
			Bullet1 *b = NewBullet1();
			b->ent.owner = ent;
			b->ent.vel.x *= mFacing;
			b->ent.vel.x += ent->vel.x;
			b->ent.pos = Vec3f (ent->pos.x, ent->pos.y, ent->pos.z);
			if (mIsCrouched) b->ent.pos.y += -10;
			b->ent.nextPos = b->ent.pos;


			/*
			float dimx = rand()%60;
			float dimy = rand()%60;
			nt->halfAabb.extents.x = dimx;
			nt->halfAabb.extents.y = dimy;
			*/

		}
	}
	else if (kstate & KS_RELEASED)
	{
		mFireCounter = 0;
	}
}
void
Player::HandleKeys ()
{
	InputDevice *dev = GetInputDevice (mInputType);
	for (int i = 0; i < NUM_PLAYER_COMMANDS; i++)
	{
		int state = dev->Key1(i) | dev->Key2(i);
		if (playerCommands[i].action)
		{
			playerCommands[i].action(state,this);
		}
	}
}
void Player::TestGround ()
{
	static const float GROUND_TEST_DIST = 5.;
	// check to see if there's ground below
	GetNextPosition() = GetPosition();
	GetNextPosition().y -= GROUND_TEST_DIST;

	CollisionResult result;
	EntityCollisionCheckWorld (ent, result);

	if (result.hasCollided)
	{
		mIsGrounded = true;
		mGroundDist = result.fraction * GROUND_TEST_DIST;
		mGroundNorm = result.normal;
	}
	else mIsGrounded = false;
}

void Player::LoadSprites ()
{
	if (!g_sprites[JAZZ_WALK].shader)
		LoadSprite (g_sprites+JAZZ_WALK, "sprites/jazz_walk.sprt");
	if (!g_sprites[JAZZ_IDLE_1].shader)
		LoadSprite (g_sprites+JAZZ_IDLE_1, "sprites/jazz_idle_1.sprt");
	if (!g_sprites[JAZZ_JUMP_FORWARD].shader)
		LoadSprite (g_sprites+JAZZ_JUMP_FORWARD, "sprites/jazz_jump_forward.sprt");
	if (!g_sprites[JAZZ_FALL_FORWARD].shader)
		LoadSprite (g_sprites+JAZZ_FALL_FORWARD, "sprites/jazz_fall_forward.sprt");
	if (!g_sprites[JAZZ_DASH].shader)
		LoadSprite (g_sprites+JAZZ_DASH, "sprites/jazz_dash.sprt");
	if (!g_sprites[JAZZ_BREAK].shader)
		LoadSprite (g_sprites+JAZZ_BREAK, "sprites/jazz_break.sprt");
}
int Player::GetAnimFrame (float fps)
{

	// NOTE: mAnimTimer is running at the physics fps
	return float(mAnimTimer)/PHYSICS_FPS * fps;
}

