#include <Config.h>
#include <Keys.h>
#include <libxml/parser.h>
#include <iostream>
#include <Player.h>
#include <GameGlobals.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

static int rShowMask=0;

static SDL_Surface *screen=NULL;

void InitVideo (int w, int h, bool full)
{
	if (SDL_Init (SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "*** Unable to initialize SDL: " << SDL_GetError();
		exit (1);
	}

	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	if (full)
		screen = SDL_SetVideoMode (w,h, 32, SDL_OPENGL | SDL_FULLSCREEN);
	else
		screen = SDL_SetVideoMode (w,h, 32, SDL_OPENGL);
	if (!screen)
	{
		std::cerr << "*** Unable to set video mode: " << SDL_GetError();
		exit (1);
	}
	/*
	 * OpenGL
	 */
	glClearColor (0.3,0.6,1.0,1.0);
	glClearDepth (1.0);
	glDepthFunc (GL_LESS);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_CULL_FACE);
	glCullFace (GL_BACK);

	glViewport (0,0,w,h);


	
	Player *player = g_players;
	Camera &cam = player->GetCamera();
	cam.SetProjection (w,h,50.0);
	float cotan = cam.projMat.M[0][0];
	cam.mat.SetTranslation(Vec3f(w*0.5,-h*0.5,cotan*w*0.5));
	
	glMatrixMode (GL_MODELVIEW);
	glTranslatef (-64.0,0.5,-160.0);
}

static void
ParseDisplay (xmlDocPtr doc, xmlNodePtr node)
{
	xmlNodePtr dnode;
	xmlChar* value;
	int width=640,height=480;
	bool fullscreen=false;

	dnode = node->xmlChildrenNode;
	while (dnode != NULL)
	{
		value = xmlNodeListGetString (doc, dnode->xmlChildrenNode, 1);
		if (value != NULL)
		{
			if (!xmlStrcmp (dnode->name, (xmlChar*)"width"))
			{
				width = atoi ((char*)value);
			}
			else if (!xmlStrcmp (dnode->name, (xmlChar*)"height"))
			{
				height = atoi ((char*)value);
			}
			else if (!xmlStrcmp (dnode->name, (xmlChar*)"fullscreen"))
			{
				if (!xmlStrcasecmp (value, (xmlChar*)"true"))
					fullscreen=true;
			}
			xmlFree (value);


		}
		dnode = dnode->next;
	}
	InitVideo(width,height,fullscreen);
}
void SetRenderFlagi (RenderFlag flag, int value)
{
	switch (flag)
	{
		case R_SHOW_MASK:
			rShowMask = (bool)value;
			break;
		default:
			break;
	}
}
void GetRenderFlagi (RenderFlag flag, int &value)
{
	switch (flag)
	{
		case R_SHOW_MASK:
			value = rShowMask;
			break;
		default:
			break;
	}
}

static void
ParseKeyBindings (xmlDocPtr doc, xmlNodePtr node)
{
	xmlNodePtr keynode,
		   dnode;
	xmlChar* value;

	dnode = node->xmlChildrenNode;
	KeyName *knames = GetKeyNames ();
	PlayerCommand *cmds = GetPlayerCommands ();
	InputDevice *dev;
	DeviceName *devnames = GetDeviceNames ();
	while (dnode != NULL)
	{
		for (int d=0; devnames[d].name;d++)
		{
			if ((!xmlStrcmp (dnode->name, (xmlChar*)devnames[d].name)))
			{
				keynode = dnode->xmlChildrenNode;
				while (keynode != NULL)
				{

					int a=0;
					while (cmds[a].name)
					{
						if ((!xmlStrcmp (keynode->name, (xmlChar*)cmds[a].name)))
						{
							value = xmlNodeListGetString (doc, keynode->xmlChildrenNode, 1);
							int k = 0;
							int key = -1;
							dev = GetInputDevice(devnames[d].type);
							KeyBinding *binds = dev->bindings;
							if (devnames[d].type < JOYSTICK1) // keyboard
							{
								while (knames[k].name)
								{
									if ((!xmlStrcmp (value, (xmlChar*)knames[k].name)))
									{
										key = knames[k].key;
										break;
									}
									k++;
								}
							}
							else // joystick
							{
								if (!xmlStrcmp(value,(xmlChar*)"left"))
									key = JOY_LEFT;
								else if (!xmlStrcmp(value,(xmlChar*)"right"))
									key = JOY_RIGHT;
								else if (!xmlStrcmp(value,(xmlChar*)"down"))
									key = JOY_DOWN;
								else if (!xmlStrcmp(value,(xmlChar*)"up"))
									key = JOY_UP;
								else
								{
									key = atoi ((char*)value);
									if (key >= NUM_JOYBUTTONS)
										key = -1;
								}
							}
							if (key != -1)
							{
								if (binds[a].key1 < 0)
									binds[a].key1 = key;
								else binds[a].key2 = key;
							}
							if (value) xmlFree (value);


							break;
						}
						a++;
					}
					keynode = keynode->next;
				}

				break;
			}
		}
		dnode = dnode->next;
	}
}

static void
ParsePlayer (Player *player, xmlDocPtr doc, xmlNodePtr node)
{
	xmlChar *value;
	DeviceName *devnames = GetDeviceNames ();
	node = node->xmlChildrenNode;
	while (node)
	{
		if (!xmlStrcmp (node->name, (xmlChar*)"input"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			for (int i=0; devnames[i].name; i++)
			{
				if (!xmlStrcmp (value, (xmlChar*)devnames[i].name))
				{
					player->SetInputType (devnames[i].type);
					break;
				}
			}
			if (value) xmlFree (value);
		}
		node = node->next;
	}
}	


void LoadConfigFile (const char *fn)
{
	xmlDocPtr doc;
	xmlNodePtr root,
		   child;

	doc = xmlParseFile (fn);
	Player *players = g_players;

	if (doc == NULL)
	{
		std::cerr << "Could not parse config file\n";
		return;
	}

	root = xmlDocGetRootElement (doc);

	if (root == NULL)
	{
		std::cerr << "Empty config file\n";
		xmlFreeDoc (doc);
		return;
	}
	
	child = root->xmlChildrenNode;
	while (child != NULL)
	{
		if (!xmlStrcmp (child->name, (const xmlChar*)"display"))
			ParseDisplay (doc, child);
		else if (!xmlStrcmp (child->name, (const xmlChar*)"keyBindings"))
			ParseKeyBindings (doc, child);
		else if (!strncmp ((char*)child->name, "player",6))
		{
			char str[2];
			for (int i=0; i<4; i++)
			{
				str[0] = '1'+i;
				str[1] = '\0';
				if (!strcmp ((char*)child->name+6, str))
					ParsePlayer (players+i, doc, child);
			}
		}
		child = child->next;
	}

	xmlFreeDoc (doc);
	xmlCleanupParser ();
	xmlMemoryDump ();

}

