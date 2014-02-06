#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <GL/gl.h>
/*
 * LoadTexture
 *
 * params:
 * 	fn - filename
 * 	interp - GL_LINEAR, GL_NEAREST,...
 */
GLuint LoadTexture (const char *fn, int interp);

#define MAX_SHADERS 5000
#define SHADER_NAME_MAX 64
struct Shader
{
	GLuint tex;
	int interp;
	char name[SHADER_NAME_MAX];
	Shader *next,*prev;
};

Shader* LoadShader (const char *fn);

#endif

