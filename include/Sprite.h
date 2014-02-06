#ifndef __SPRITE_H___
#define __SPRITE_H___

#include <GL/gl.h>
#include <Vec2f.h>

#define MAX_SPRITE_FRAMES 100

struct Shader;

struct SpriteFrame
{
	Vec2f p1,p2; //texture coords
};

struct Sprite
{
	Shader *shader;
	SpriteFrame frames[MAX_SPRITE_FRAMES];
	int numFrames;
	float w,h;
	float fps;
	Vec2f pos; // offset from center of image
	Sprite();
};
void LoadSprite (Sprite *s, const char* fn);
	

#endif

