#include <Sprite.h>
#include <GameGlobals.h>
#include <libxml/xmlwriter.h>
#include <Texture.h>

Sprite g_sprites[NUM_SPRITES];

Sprite::Sprite()
{
	shader=NULL;
	numFrames = 0;
}

void LoadSprite (Sprite *sprite, const char *fn)
{

	using namespace std;


	xmlDocPtr doc;
	xmlNodePtr root,
		   node;
	xmlChar *value;
	// defualt fps
	sprite->fps = 10.0;

	doc = xmlParseFile (fn);
	if (doc == NULL)
	{
		cerr << "Could not load sprite file: " << fn << "\n";
		return;
	}
	root = xmlDocGetRootElement (doc);
	if (root == NULL)
	{
		cerr << "Empty sprite file: " << fn << "\n";
		xmlFreeDoc (doc);
		return;
	}

	int numRows=0, numFramesPerRow=0,
	    firstFrame=0;
	float img_w, img_h;
	node = root->xmlChildrenNode;
	while (node)
	{
		value = NULL;
		if (!xmlStrcmp (node->name, BAD_CAST "shader"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			sprite->shader = LoadShader ((char*)value);

		}
		else if (!xmlStrcmp (node->name, BAD_CAST "width"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			sprite->w = atoi ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "height"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			sprite->h = atoi ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "width"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			sprite->w = atoi ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "offsetX"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			sprite->pos.x = atof ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "offsetY"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			sprite->pos.y = atof ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "numFrames"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			sprite->numFrames = atoi ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "framesPerRow"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			numFramesPerRow = atoi ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "numRows"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			numRows = atoi ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "firstFrame"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			firstFrame = atoi ((char*)value);
		}
		else if (!xmlStrcmp (node->name, BAD_CAST "fps"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			sprite->fps = atof ((char*)value);
		}
		if (value)
			xmlFree (value);
		node = node->next;
	}
	xmlFreeDoc (doc);
	xmlCleanupParser ();
	xmlMemoryDump ();
	if (!sprite->shader)
	{
		std::cerr << "SpriteLoader: No shader for " << fn << "\n";
		return;
	}
	if (sprite->shader->tex)
	{
		glBindTexture (GL_TEXTURE_2D, sprite->shader->tex);
		glGetTexLevelParameterfv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &img_w);
		glGetTexLevelParameterfv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &img_h);
	}

	float w = sprite->w/img_w,
		h = sprite->h/img_h;
	float offH = 0;
	float offV = 0;
	int i=0;
	int y= firstFrame/numFramesPerRow;
	int x = firstFrame%numFramesPerRow;
	int lastFrame = sprite->numFrames+firstFrame;
	for (; y<numRows; y++)
	{
		int len = numFramesPerRow;
		if ((y+1)*numFramesPerRow > lastFrame)
			len = lastFrame - y*numFramesPerRow;
	for (; x<len; x++)
	{
		sprite->frames[i].p1.x = offH + x*w;
		sprite->frames[i].p2.x = offH + x*w+w;
		sprite->frames[i].p1.y = offV + y*h;
		sprite->frames[i].p2.y = offV + y*h+h;
		i++;
	}
	x=0;
	}

		
}

