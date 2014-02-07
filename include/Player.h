#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <Entity.h>
#include <Camera.h>
#include <Keys.h>


// used for parsing config file
struct Player;
typedef void (*KeyCB) (int key, Player *player);

struct PlayerCommand
{
	KeyCB action;
	const char *name;
};

enum PlayerAnim
{
	ANIM_NONE=0,
	ANIM_IDLE,
	ANIM_WALK,
	ANIM_JUMP_UP,
	ANIM_JUMP_FORWARD,
	ANIM_JUMP_FORWARD_IDLE,
	ANIM_FALL_FORWARD,
	ANIM_DASH,
	ANIM_BREAK
};
struct PlayerEntity
{
	Entity ent; //must be first

	Player *player; // owner of entity
};

class Player
{
public:
	Entity *ent;

	Player () { }; // use InitPlayer instead
	void Init();
	void FocusCamera();
	virtual ~Player () { };

	inline Vec3f& GetPosition ()
	{
		return ent->pos;
	}
	inline Vec3f& SetPosition (const Vec3f &pos)
	{
		return ent->pos = pos;
	}
	inline Vec3f& GetNextPosition ()
	{ return ent->nextPos; }

	inline Vec3f& SetNextPosition (const Vec3f &pos)
	{ return ent->nextPos = pos; }

	inline Vec3f& GetVelocity (){ return ent->vel; }
	inline Vec3f& SetVelocity (const Vec3f &v){ return ent->vel=v; }

	Camera& GetCamera ()
	{ return mCamera; }

	void SetInputType (InputType t)
	{	mInputType = t; }
	InputType GetInputType ()
	{ return mInputType; }
	
	virtual void Update ();
	virtual void Render (Camera &camera);

	virtual void ComputeNextPosition (float time);
	void HandleCollision (Entity *ent, Vec3f norm);
		
	KeyBinding* GetKeyBindings ()
	{	return mKeyBindings; }
	void ActionWalk (int kstate, int dir);
	void ActionRun (int kstate);
	void ActionDown (int kstate);
	void ActionFly (int kstate, Vec3f dir);
	void ActionJump (int kstate);
	void ActionFire (int kstate);
	void HandleKeys ();
	void LoadSprites ();
	int GetAnimFrame (float fps);

	void TestGround (); // test to see whether there is ground right below
	KeyBinding mKeyBindings[NUM_PLAYER_COMMANDS];
	InputType mInputType;
	Camera mCamera;
	int mJumpCounter,
	    mRunCounter,
	    mFireCounter;
	float mRunAccel;
	int mFacing; // dir along the x axis either 1 or -1
	bool mIsMoving;
	bool mIsRunning;
	bool mIsGrounded;
	bool mIsCrouched;
	float mGroundDist;
	Vec2f mGroundNorm;
	unsigned int mAnimTimer;
	bool mSpritesLoaded;
	PlayerAnim mAnim;
};

Player* NewPlayer ();
PlayerCommand* GetPlayerCommands ();

#endif

