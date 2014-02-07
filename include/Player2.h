#ifndef __PLAYER2_H__
#define __PLAYER2_H__

#include <Entity.h>
#include <Camera.h>
#include <Keys.h>


// used for parsing config file
struct Player2;
typedef void (*KeyCB) (int key, Player2 *player);

struct PlayerCommand
{
	KeyCB action;
	const char *name;
};



class Player2
{
public:
	Player2 ();
	virtual ~Player2 () { };

		
	KeyBinding* GetKeyBindings ()
	{	return mKeyBindings; }
	void ActionWalk (int kstate, int dir);
	void ActionRun (int kstate);
	void ActionFly (int kstate, Vec3f dir);
	void ActionJump (int kstate);
	void HandleKeys ();
	void Update ();

	// it's entity
	Entity *entity;
private:

	HalfAabb mHalfAabb;
	KeyBinding mKeyBindings[NUM_PLAYER_COMMANDS];
	InputType mInputType;
	Camera mCamera;
};


Player2*	GetPlayer2s();
PlayerCommand* GetPlayer2Commands ();

#endif

