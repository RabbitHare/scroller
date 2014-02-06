#include <Texture.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include <libxml/xmlreader.h>

Shader g_shaderPool[MAX_SHADERS];
Shader *g_shaders=NULL; //linked list

GLuint LoadTexture (const char *fn, int interp)
{
	GLuint gltexture=0;
	// load the image
	SDL_Surface *image = IMG_Load (fn);
	if (!image)
	{
		std::cerr << "*** IMG_Load: " << IMG_GetError() << std::endl;
		return false;
	}
	GLenum format;
	switch (image->format->BytesPerPixel)
	{
		case 1:
			format = GL_COLOR_INDEX;
			break;
		case 3:
			if (image->format->Rshift)
				format = GL_BGR;
			else format = GL_RGB;
			break;
		case 4:
			if (image->format->Rshift)
				format = GL_BGRA;
			else format = GL_RGBA;
			break;
		default:
			std::cerr << "***LoadTexture: bad bytes per pixel.";
			SDL_FreeSurface (image);
			return gltexture;
	}

	glGenTextures (1, &gltexture);
	glBindTexture (GL_TEXTURE_2D, gltexture);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interp);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interp);

	glEnable (GL_COLOR_TABLE);
	glTexImage2D (GL_TEXTURE_2D, 0, image->format->BytesPerPixel,
			image->w, image->h, 0, format, GL_UNSIGNED_BYTE,
			image->pixels);


	SDL_FreeSurface (image);

	
	
	return gltexture;
}

Shader* LoadShader (const char *fn)
{
	using namespace std;


	xmlDocPtr doc;
	xmlNodePtr root,
		   node;
	xmlChar *value;


	if (g_shaders == NULL)
	{
		// init shader
		for (int i=0; i < MAX_SHADERS; i++)
			g_shaderPool[i].tex=0;
		//create dummy
		g_shaders = g_shaderPool;
		g_shaders->next = g_shaders->prev = NULL;
	}
	//check to see if shader has already been loaded
	Shader *s=g_shaders;
	while (s)
	{
		if (!strcmp(fn, s->name))
		{

			puts ("shader found");
			return s;
		}
		s = s->next;
	}
	// else load it
	
	doc = xmlParseFile (fn);
	if (doc == NULL)
	{
		cerr << "Could not load shader: " << fn << "\n";
		return NULL;
	}
	root = xmlDocGetRootElement (doc);
	if (root == NULL)
	{
		cerr << "Empty shader file: " << fn << "\n";
		xmlFreeDoc (doc);
		return NULL;
	}


	node = root->xmlChildrenNode;

	int interp;
	string texName;

	while (node)
	{
		value = NULL;
		if (!xmlStrcmp (node->name, BAD_CAST "texture"))
		{
			if (node->xmlChildrenNode)
			{
				value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
				texName = (char*)value;
				if (value)
					xmlFree (value);
			}
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "interp"))
		{
			if (node->xmlChildrenNode)
			{
				value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
					if (!strcmp ((char*)value, "nearest"))
						interp = GL_NEAREST;
					else interp = GL_LINEAR;
				if (value)
					xmlFree (value);
			}
		}
		node = node->next;
	}
	xmlFreeDoc (doc);
	xmlCleanupParser ();
	xmlMemoryDump ();

	//ADD TO LINKED LIST
	// find unused one
	s=NULL;
	for (int i=1; i < MAX_SHADERS; i++)
	{
		if (!g_shaderPool[i].tex)
		{
			s = g_shaderPool+i;
			if (strlen(fn) > SHADER_NAME_MAX-1)
			{
				std::cerr << "ShaderLoader: filename too large\n";
				s->name[0] = '\0';
			}
			else strcpy(s->name, fn);



			// link in
			s->prev = NULL;
			s->next = g_shaders;
			g_shaders->prev = s;
			g_shaders = s;
			break;
		}

	}
	if (!s)
	{
		std::cerr << "TextureShader: MAX NUMBER OF SHADERS REACHED\n";
		exit(1);
	}

	s->interp = interp;
	s->tex = LoadTexture (texName.c_str(), interp);
	return s;
}
