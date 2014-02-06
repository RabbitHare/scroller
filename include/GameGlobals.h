#ifndef __G_GLOBALS_H__
#define __G_GLOBALS_H__

#include <Sprite.h>
#include <Entity2.h>
#include <GObjType.h>

struct Player;

#define MAX_PLAYERS 4
extern Player g_players[4]; // defined in Player.cpp
extern int g_numPlayers; // defined in Player.cpp

#define NUM_SPRITES 100
extern Sprite g_sprites[NUM_SPRITES]; // defined in Sprite.cpp
#define MAX_ENTITIES 200
extern EntityMem g_entityPool[MAX_ENTITIES]; // defined in Entity.cpp
extern int g_numEntities;
extern Entity2 *g_entities; // defined in Entity.cpp

#define MAX_GOBJ_TYPES 500
extern GObjType g_objTypes[MAX_GOBJ_TYPES]; //defined in GObjType.h
extern int g_numObjTypes;
extern const char* g_objTypeCats[]; // defined in GObjType.h

static const float PHYSICS_FPS = 60;
static const float PHYSICS_FRAME_LEN = 1000.0/PHYSICS_FPS;

#define DRAW_BOUNDS 0

enum EntityType
{
	ET_INVALID = 0,
	ET_MAP = 1,
	ET_PLAYER = 1<<1,
	ET_ENEMY = 1<<2,
	ET_BULLET = 1<<3,
	ET_SHELL = 1<<4,
	ET_ALL = 0xFFFFFFFF
};

enum SpriteAnim
{

// Jazz Animations
 JAZZ_WALK = 0,
 JAZZ_IDLE_1,
 JAZZ_JUMP_FORWARD,
 JAZZ_FALL_FORWARD,
 JAZZ_DASH,
 JAZZ_BREAK,
 // Norm Turtle
 TURTLE_WALK,
 TURTLE_SHELL,
 // Weapons
 BULLET1_FLY
};

#endif
