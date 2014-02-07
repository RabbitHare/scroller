#include <Player2.h>
#include <GL/gl.h>
#include <Map.h>

// NOT USED YET
static Player2 players[4];

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
// must be length of NUM_PLAYER_COMMANDS+1
static PlayerCommand playerCommands[] = {
	{rightCb,"right"},
	{leftCb,"left"},
	{flyUpCb,"up"},
	{NULL,"down"},
	{runCb,"run"},
	{jumpCb,"jump"},
	{NULL,"fire"},
	{NULL,"cycle"},
	{NULL,NULL}};

Player*
GetPlayer2s ()
{
	return players;
}
PlayerCommand* GetPlayer2Commands()
{
	return playerCommands;
}

static void InitPlayerEntity (Entity &ent);

Player2::Player2 ()
{
	mPosition = Vec3f(0,0,0);
	mVelocity = Vec3f(0,0,0);
	mHalfAabb.extents = Vec2f(16.0,20.0);
	mCamera.mat.SetTranslation (Vec3f(mPosition.x,mPosition.y, 0.));
	mIsMoving = false;
	mIsRunning = false;
	mIsGrounded = false;
	mJumpCounter = 0;
	mRunCounter = 0;
	mFacing = 1;
	mInputType = KEYBOARD1;
	InitPlayerEntity (mEntity);
}
void
Player2::Update ()
{

	HandleKeys ();
}
static void P_Render (Entity *self, Camera &camera)
{
	glDisable (GL_TEXTURE_2D);
	Vec3f r = self->halfAabb.extents;
	Vec3f p = self->position;

	glColor3f (1.0,0.0,0.0);
	glBegin (GL_QUADS);

	glVertex3f (p.x-r.x, p.y+r.y, p.z);
	glVertex3f (p.x-r.x, p.y-r.y, p.z);
	glVertex3f (p.x+r.x, p.y-r.y, p.z);
	glVertex3f (p.x+r.x, p.y+r.y, p.z);

	glEnd ();
	glEnable (GL_TEXTURE_2D);
	glColor3f (1.,1.,1.);
}
 //
 // TestGround
 //
static void TestGround (Entity *self)
{
	static const float GROUND_TEST_DIST = 2.;
	CollisionResult result;
	// check to see if there's ground below
	self->nextPosition = self->position;
	self->nextPosition.y -= GROUND_TEST_DIST;
	Map &map = *GetMap();
	map.CollisionCheck (*self, result);
	if (result.hasCollided)
	{
		mIsGrounded = true;
		mGroundDist = result.fraction * GROUND_TEST_DIST;
		mGroundNorm = result.normal;
	}
	else mIsGrounded = false;
}
void void P_Update ()
{
	
	TestGround (); // see whether there is ground right below

	if (self->velocity.y > maxFallSpeed)
	{
		mVelocity.y -= gravity;
		if (mVelocity.y < maxFallSpeed)
			mVelocity.y = maxFallSpeed;
	}
	if (!mIsMoving)
	{
		if (mIsGrounded && mGroundDist < 1.)
		{
			if (fabs (mVelocity.x) <= groundFriction)
				mVelocity.x = 0.;
			else if (mVelocity.x < 0)
				mVelocity.x += groundFriction;
			else if (mVelocity.x > 0)
				mVelocity.x -= groundFriction;
		}
		if (fabs (mVelocity.x) < 0.01)
			mVelocity.x = 0.;
		else if (mVelocity.x < 0)
			mVelocity.x += friction;
		else if (mVelocity.x > 0)
			mVelocity.x -= friction;
	}
	ComputeNextPosition (1.0);
}
static void
P_ComputeNextPosition (float time)
{
	mNextPosition = mPosition + mVelocity*time;
}
static void 
P_MoveToNextPosition (CollisionResult &result)
{
	if (result.hasCollided)
	{
		mPosition += (mNextPosition - mPosition)*result.fraction;
		// velocity
		if (result.normal.y > 0.5) // slide along floors
			mVelocity -= result.normal.Project(mVelocity)*1.0;
		else // bounce off walls slightly
			mVelocity -= result.normal.Project(mVelocity)*1.2;
		mVelocity.z = 0.;
		ComputeNextPosition (1.0 - result.fraction);
	}
	else mPosition = mNextPosition;
	// camera position
	Vec3f cpos = mPosition;
	cpos.z = mCamera.mat.GetOrigin().z;
	mCamera.mat.SetOrigin (cpos);
}
//////////////////
// InitPlayerEntity
//////////////////
static void InitPlayerEntity (Entity *ent)
{
	ent->velocity = Vec3f (0,0,0);
	ent->position = Vec3f (0,0,0);
	ent->Update = P_Update;
	ent->Render = P_Render;
	ent->ComputeNextPosition = P_ComputeNextPosition;
	ent->MoveToNextPosition = P_MoveToNextPosition;
}

void
Player::ActionWalk (int kstate, int dir)
{
	float s;
	float fdir = (float)dir;
	if (kstate & KS_PRESSED)
	{
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
			
			if (mVelocity.x*fdir < maxWalkSpeed)
				accel = runInitAccel;
			else accel = runAccel*mVelocity.x/maxRunSpeed*fdir;
			float v = mVelocity.x + accel*fdir*s;
			if (v*fdir >= max)
			{
				if (mVelocity.x*mVelocity.x > v*v)
					mVelocity.x = v;
			}
			else mVelocity.x = v;
		}
		else // walk
		{
			if (mIsGrounded)
				s = 1.0;
			else s = 0.9;
			float v = mVelocity.x + walkAccel*fdir*s;
			if (v >= maxWalkSpeed)
			{
				if (mVelocity.x > v)
					mVelocity.x = v;
				else if (mVelocity.x < v)
					mVelocity.x = maxWalkSpeed;
			}
			else if (v <= -maxWalkSpeed)
			{
				if (mVelocity.x < v)
					mVelocity.x = v;
				else if (mVelocity.x > v)
					mVelocity.x = -maxWalkSpeed;
			}
			else mVelocity.x = v;
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

			float t = -n.Dot (mVelocity);
			if (t < 0.) // down slope
			{
				Vec3f vel = mVelocity + (n*t);
				vel *=  mVelocity.x/vel.x;
				mVelocity = vel;
			}
		}
		mIsMoving = true;
		mFacing = dir;
	}
	else if (kstate & KS_RELEASED)
		mIsMoving = false;
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
		Vec3f vel = mVelocity + dir*flyAccel;
		float len = vel.Normalize ();
		if (len > maxFlySpeed)
			vel *= maxFlySpeed;
		else vel *= len;
		mVelocity = vel;
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
			if (mVelocity.y > 0)
				mVelocity.y *= 0.3;
			else if (mVelocity.y < 0.)
				mVelocity.y = 0.;
			mVelocity.y += fabs(mVelocity.x*0.3);
		}
		if (mJumpCounter > -1 && mJumpCounter < 5)
		{



			float s = 0.2;
			if (mJumpCounter > 2)
			{
				float jmp = (float)(mJumpCounter-2);
				s = 1.0/jmp*jmp;
			}
			mVelocity.y += jumpAccel*s;

			mJumpCounter++;
		}
		if (mVelocity.y < 0)
			mJumpCounter = -1;
	}
	else if (kstate & KS_RELEASED)
		mJumpCounter = -1;
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

