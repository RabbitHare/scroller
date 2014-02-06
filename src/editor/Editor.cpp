#include <GL/gl.h>
#include <GL/glut.h>
#include <gtk/gtk.h>
#include <gdk/gdkgl.h>
#include <gtk/gtkgl.h>

#include <Dock.h>
#include <TileWindow.h>
#include <MapWindow.h>

#include <Tileset.h>
#include <Map.h>
#include <Ui.h>
#include <Renderer.h>
#include <GObjType.h>
#include <Global.h>
#include <EditorLoadSave.h>


Global G;

static void
ParseArgs (int argc, char *argv[])
{
	char *cur,
	     *next;
	for (int i=1; i < argc; i++)
	{
		cur = argv[i];
		if (i < argc-1) next = argv[i+1];
		else next = NULL;
		
		int len = strlen (cur);	
		if (!strcmp (cur+len-4, ".lvl"))
		{
			EditorLoadLevel (cur);
		}
	}

}
int main (int argc, char *argv[])
{
	Renderer renderer;

	gtk_init (&argc, &argv);
	gdk_gl_init (&argc, &argv);

	glutInit (&argc,(char**)argv);

// load the entities, modifiers, etc. placed in maps
	LoadGObjTypes ();

	Ui::InitUi ();

	ParseArgs (argc, argv);
	//Renderer
	SetRenderer (&renderer);
	Map &map = *GetMap();
	if (map.layers.size() == 0)
	{
		map.MakeNew (); // new level

	}

	Ui::Notify (UI_NEW_MAP);	
	gtk_main ();
}

