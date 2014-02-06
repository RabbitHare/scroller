#include <iostream>
#include <time.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <Mat4f.h>
#include <Tileset.h>
#include <Map.h>
#include <Collision.h>
#include <Config.h>
#include <Player.h>
#include <Map.h>
#include <Renderer.h>
#include <GameGlobals.h>
#include <GameLoadSave.h>


#define SHOW_FPS 1
#define SHOW_MASK 1



static void Draw (Camera &camera)
{
	Map *map = GetMap ();
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	map->Render (camera);

	SDL_GL_SwapBuffers ();
}
static bool running = true;

static void PollKeys ()
{
	Key *keys = GetKeys ();
	Joystick *joys = GetJoysticks();
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		int k = event.key.keysym.sym;
		int b = event.jbutton.button;
		int j = event.jbutton.which;
		int axis = event.jaxis.axis;
		int jvalue = event.jaxis.value;
		bool skipjoy=false;
		if (j > NUM_JOYSTICKS || b > NUM_JOYBUTTONS)
			skipjoy = true;
		switch (event.type)
		{
			case SDL_KEYDOWN:
				if (k == SDLK_ESCAPE)
					running = false;
				keys[k].state = KS_PRESSED;
				break;
			case SDL_KEYUP:
				keys[k].state = KS_RELEASED;
				break;
			case SDL_JOYBUTTONDOWN:
				if (skipjoy) break;
				joys[j].buttons[b].state = KS_PRESSED;
				break;
			case SDL_JOYBUTTONUP:
				if (skipjoy) break;
				joys[j].buttons[b].state = KS_RELEASED;
				break;
			case SDL_JOYAXISMOTION:
				int a;
				// convert to button signals
				if (jvalue != 0)
				{
					int t = jvalue > 0?1:-1;
					a = axis*2+(1+t)/2;
					// make sure the opposite directional button gets released
					if (joys[j].buttons[JOY_DIR+a - t].state == KS_PRESSED)
						joys[j].buttons[JOY_DIR+a - t].state = KS_RELEASED;

					joys[j].buttons[JOY_DIR+a].state = KS_PRESSED;
				}
				else
				{
					a =  axis*2;
					if (joys[j].buttons[JOY_DIR+a].state == KS_PRESSED)
						joys[j].buttons[JOY_DIR+a].state = KS_RELEASED;
					if (joys[j].buttons[JOY_DIR+a+1].state == KS_PRESSED)
						joys[j].buttons[JOY_DIR+a+1].state = KS_RELEASED;
				}
				break;
			case SDL_QUIT:
				running = false;
				break;
			default:
				break;
		}
	}
}
static void
ParseArgs (int argc, char *argv[])
{
	Map &map = *GetMap();
	char *cur,
	     *next;
	for (int i=1; i < argc; i++)
	{
		cur = argv[i];
		if (i < argc-1) next = argv[i+1];
		else next = NULL;
		
		int len = strlen (cur);	
		if (!strcmp (cur+len-4, ".lvl"))
			GameLoadLevel (cur);
	}

	if (map.layers.size() == 0)
	{
		std::cout << "No Level!\nusage: scroller [NAME.lvl]\n";
		exit(0);
	}
}
int main (int argc, char *argv[])
{
	srand (time (NULL));

	Renderer renderer;

	//Add a player
	Player *player = NewPlayer();
	player->SetPosition (Vec3f (288.0, -192.0, 0.1));

	LoadConfigFile ("config.xml");
	//InitVideo (640,480);

	Map &map = *GetMap();

	ParseArgs (argc, argv);

	// Renderer
	SetRenderer (&renderer);
//	renderer.SetTileset (GetTileset (0));
//	renderer.SetShowMask (true);


	player->FocusCamera ();

//	if (map.layers.size())
//		map.layers[0].AddEntity (players);
	// Joystick
	SDL_InitSubSystem (SDL_INIT_JOYSTICK);
	if (SDL_NumJoysticks () > 0)
	{
	//	SDL_Joystick *joy = SDL_JoystickOpen (0);
		SDL_JoystickOpen (0);
	}
	int start_time,end_time,
	    total_time,
	    phy_time=0.,
	    anim_time=0.;
	float fps=0.0;
	int fps_count=0;
	int i=0;
	while (running)
	{
		start_time = SDL_GetTicks ();
		// physics loop
		
		while (phy_time > PHYSICS_FRAME_LEN)
		{
			map.Update ();
			CollisionCheckWorld ();
			phy_time -= PHYSICS_FRAME_LEN;
			ClearReleasedKeys ();
		}


		Draw (player->GetCamera());
		PollKeys ();
		end_time = SDL_GetTicks ();
		total_time = end_time - start_time;
		phy_time += total_time;
		anim_time += total_time;
		if (SHOW_FPS)
		{
			fps += total_time;
			fps_count += total_time;
			i++;
			if (fps_count > 1000)
			{
				fps /= (float)i;
				fps = 1000.0/fps;
				std::cout << fps << " fps\n";
				std::cout << g_numEntities << " num entities\n";
				i=0;
				fps = 0.;
				fps_count=0;
			}
		}
	}

	SDL_Quit ();
	return 0;
}

