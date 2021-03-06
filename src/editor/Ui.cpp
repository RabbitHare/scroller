#include <Ui.h>
#include <Notifiable.h>
#include <Map.h>
#include <MapWindow.h>
#include <TileWindow.h>
#include <LayerDialog.h>
#include <Dock.h>
#include <gtk/gtk.h>
#include <gdk/gdkgl.h>
#include <gtk/gtkgl.h>
#include <UndoSys.h>
#include <Global.h>
#include <GL/gl.h>
#include <EditorLoadSave.h>


static GSList *notifiables=NULL;

static GtkWidget *main_window=NULL;
static GtkWidget *open_file_chooser=NULL,
		 *save_file_chooser=NULL;
const char *DEFAULT_LEVEL_DIR = "./levels";

static GtkWidget* create_shared_context_widget ();
static void add_main_menu_bar (GtkWidget *box, GtkWidget *window);

/* temp */
GtkWidget *testImage = NULL;

void Ui::InitUi ()
{
	GtkWidget *window,
		  *vbox,
		  *dock,
		  *sc_widget,
		  *tile_win,
		  *map_win;

	// main window
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	//gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 320,384);
	gtk_container_set_reallocate_redraws(GTK_CONTAINER (window), TRUE);
	g_signal_connect (G_OBJECT (window), "delete_event",
		  G_CALLBACK (gtk_main_quit), NULL);
	main_window = window;


	vbox = gtk_vbox_new (false,0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	// main menu bar
	add_main_menu_bar (vbox, window);

	// get shared list context
	sc_widget = create_shared_context_widget ();
	gtk_box_pack_start (GTK_BOX (vbox), sc_widget, true, true, 0);
	gtk_widget_show_all (window);

	// set global context for shared lists and textures
	G.glContext = gtk_widget_get_gl_context (sc_widget);

	gtk_widget_set_no_show_all (sc_widget, TRUE);
	gtk_widget_hide (sc_widget);

	dock = dock_new ();
	gtk_box_pack_start (GTK_BOX (vbox), dock, true, true, 0);
	dock_destroy_on_empty (DOCK (dock), FALSE);


	// tile window
	tile_win = tile_window_new ();
	dock_append (DOCK (dock), tile_win);
	gtk_widget_show_all (window);

	gtk_window_move (GTK_WINDOW (window), 0, 0);

	

	// map window
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	//gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 640,480);
	gtk_window_set_title (GTK_WINDOW (window), "Editor");


	//Get automatically redrawn if any of its children change allocation.
	gtk_container_set_reallocate_redraws(GTK_CONTAINER (window), TRUE);

	dock = dock_new ();
	gtk_container_add (GTK_CONTAINER (window), dock);

	map_win = map_window_new ();
	dock_append (DOCK (dock), map_win);
	gtk_widget_show_all (window);

	gtk_window_move (GTK_WINDOW (window), 400, 0);

	//---- Layer Dialog -----
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Editor");
	gtk_window_set_default_size (GTK_WINDOW (window), 300,200);
	//Get automatically redrawn if any of its children change allocation.
	gtk_container_set_reallocate_redraws(GTK_CONTAINER (window), TRUE);

	dock = dock_new ();
	gtk_container_add (GTK_CONTAINER (window), dock);
	GtkWidget *lylog = layer_dialog_new ();
	dock_append (DOCK (dock), lylog);
	gtk_widget_show_all (window);

}
void
Ui::AddNotifiable (Notifiable *nf)
{
	notifiables = g_slist_prepend (notifiables, nf);
}
void Ui::RemoveNotifiable (Notifiable *nf)
{
	notifiables = g_slist_remove (notifiables, nf);
}


void
Ui::Notify (UiNotify note)
{
	Map* map = GetMap();
	if (note == UI_NEW_MAP)
	{

		if (map->layers.size())
		{
			Layer *lay = &(map->layers[0]);
			G.selLayer = lay;
			G.selTiles.Clear();
			Notify (UI_NEW_ACTIVE_LAYER);
		}

		UpdateWindowTitle ();
	}
	else if (note == UI_NEW_ACTIVE_LAYER){}
	else if (note == UI_LEVEL_EDIT) {}

	GSList *nf = notifiables;
	while (nf)
	{
		notifiable_notify (NOTIFIABLE (nf->data), note);
		nf = nf->next;
	}
}

void
Ui::UpdateWindowTitle ()
{
	if (!main_window) return;
	std::string title = "Level Editor - ";
	title += GetMap()->filename;
	if (UndoSys::HasUnsavedActions())
		title += "*";
	gtk_window_set_title (GTK_WINDOW (main_window), title.c_str());

}

/*
static void
activate_action (GtkAction *action)
{
	std::cout << gtk_action_get_name (action) << " was activated\n";
}
*/
/*
==============
 new_cb
==============
*/
static void
new_cb ()
{
	GetMap()->MakeNew ();
	Ui::Notify (UI_NEW_MAP);
	
}

/*
==============
 open_cb
==============
*/
static void
open_cb ()
{
	GtkFileFilter *ff;

	if (!open_file_chooser)
	{
		open_file_chooser = gtk_file_chooser_dialog_new ("Load Level File",
				GTK_WINDOW (main_window),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);

		g_signal_connect (G_OBJECT (open_file_chooser), "delete_event",
				  G_CALLBACK (gtk_widget_hide_on_delete), NULL);

		// Filters
		ff = gtk_file_filter_new ();
		gtk_file_filter_set_name (ff, "Level file: *.lvl");
		gtk_file_filter_add_pattern (ff,"*.lvl");
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (open_file_chooser), ff);
		ff = gtk_file_filter_new ();
		gtk_file_filter_set_name (ff, "All Files");
		gtk_file_filter_add_pattern (ff,"*");
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (open_file_chooser),ff);

		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (save_file_chooser),
				DEFAULT_LEVEL_DIR);
	
	}
	if (gtk_dialog_run (GTK_DIALOG (open_file_chooser)) == GTK_RESPONSE_ACCEPT)
	{
		
		char *fn = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (open_file_chooser));
		if (fn)
		{
			GetMap()->Clear();
			EditorLoadLevel (fn);
			Ui::Notify (UI_NEW_MAP);
			g_free (fn);
		}
		
	}
	gtk_widget_hide (open_file_chooser);

}
/*
==============
 save_as_cb
==============
*/
static void
save_as_cb ()
{
	GtkFileFilter *ff;
	std::string str;

	if (!save_file_chooser)
	{
		save_file_chooser = gtk_file_chooser_dialog_new ("Save Level",
				GTK_WINDOW (main_window),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);

		g_signal_connect (G_OBJECT (save_file_chooser), "delete_event",
				  G_CALLBACK (gtk_widget_hide_on_delete), NULL);

		// Filters
		ff = gtk_file_filter_new ();
		gtk_file_filter_set_name (ff, "Level file: *.lvl");
		gtk_file_filter_add_pattern (ff,"*.lvl");
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (save_file_chooser), ff);
		ff = gtk_file_filter_new ();
		gtk_file_filter_set_name (ff, "All Files");
		gtk_file_filter_add_pattern (ff,"*");
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (save_file_chooser),ff);
	
	}
	
	str = GetMap()->filename;
	if (str.rfind (".lvl") != str.size()-4)
	{
		str += ".lvl";
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (save_file_chooser),
				DEFAULT_LEVEL_DIR);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (save_file_chooser), str.c_str());
	}
	else
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (save_file_chooser), str.c_str());

	if (gtk_dialog_run (GTK_DIALOG (save_file_chooser)) == GTK_RESPONSE_ACCEPT)
	{
		
		char *fn = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (save_file_chooser));
		if (fn)
		{
			str = fn;
			if (str.rfind (".lvl") != str.size()-4)
				str += ".lvl";
			EditorSaveLevel (str.c_str());
			UndoSys::NoUnsavedActions();
			Ui::UpdateWindowTitle ();
			
			g_free (fn);
		}
		
	}
	gtk_widget_hide (save_file_chooser);

}
/*
=============
 save_cb
=============
*/
static void
save_cb ()
{
	Map *map = GetMap();
	if (map->filename.rfind (".lvl") != map->filename.size()-4)
		save_as_cb ();
	else 
	{
		EditorSaveLevel (map->filename.c_str());
		UndoSys::NoUnsavedActions();
		Ui::UpdateWindowTitle ();
	}
}

/*
=================
 save_and_run_cb
=================
*/
static void
save_and_run_cb ()
{
	save_cb ();
	Map *map = GetMap();
	std::string cmd("./scroller ");
	cmd += map->filename;
	system (cmd.c_str());
}

static void
create_tiles_dialog ()
{
	GtkWidget *window,
		  *tiles,
		  *dock;
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 320,384);

	dock = dock_new ();
	gtk_container_add (GTK_CONTAINER (window), dock);
	tiles = tile_window_new ();

	dock_append (DOCK(dock), tiles);
	gtk_widget_show_all (window);
}

static void
create_map_dialog ()
{
	GtkWidget *window,
		  *mapw,
		  *dock;
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	dock = dock_new ();
	gtk_container_add (GTK_CONTAINER (window), dock);
	mapw = map_window_new ();

	dock_append (DOCK(dock), mapw);
	gtk_widget_show_all (window);

	notifiable_notify (NOTIFIABLE (mapw), UI_NEW_MAP);
}

static GtkActionEntry ui_entries[] = {
	{ "FileMenu", NULL, "_File"},
	{ "New", GTK_STOCK_NEW,
	  "New", NULL,
	  "Create a new level",
	  G_CALLBACK (new_cb) },
	{ "Open", GTK_STOCK_OPEN,
	  "_Open", "<control>O",
	  "Open a level",
	  G_CALLBACK (open_cb) },
	{ "Save", GTK_STOCK_SAVE,
	  "_Save", "<control>S",
	  "Save current level",
	  G_CALLBACK (save_cb) },
	{ "SaveAs", GTK_STOCK_SAVE,
	  "Save _As...", "",
	  "Save current level",
	  G_CALLBACK (save_as_cb) },
	{ "SaveAndRun", NULL,
	  "Save & _Run", "<control>R",
	  "Save and play current level",
	  G_CALLBACK (save_and_run_cb) },
	{ "DialogsMenu", NULL, "_Dialogs"},
	{ "Tiles", NULL,
	  "_Tiles", "",
	  "Open tile selector",
	  G_CALLBACK (create_tiles_dialog) },
	{ "Map", NULL,
	  "_Map", "",
	  "Open map of level",
	  G_CALLBACK (create_map_dialog) },
	{ "Quit", GTK_STOCK_QUIT,
	  "_Quit", "<control>Q",
	  "Exit the editor",
	  G_CALLBACK (gtk_main_quit) }
};

static const int n_ui_entries = G_N_ELEMENTS (ui_entries);

static const char* ui_info =
	"<ui>\
	   <menubar name='MenuBar'>\
	     <menu action='FileMenu'>\
	       <menuitem action='New'/>\
	       <menuitem action='Open'/>\
	       <separator/>\
	       <menuitem action='Save'/>\
	       <menuitem action='SaveAs'/>\
	       <menuitem action='SaveAndRun'/>\
	       <separator/>\
	       <menu action='DialogsMenu'>\
		 <menuitem action='Tiles'/>\
		 <menuitem action='Map'/>\
	       </menu>\
	       <separator/>\
	       <menuitem action='Quit'/>\
	     </menu>\
	   </menubar>\
	 </ui>";

static void add_main_menu_bar (GtkWidget *box, GtkWidget *window)
{
	GtkUIManager *ui;
	GtkActionGroup *actions;
	GError *error = NULL;
	GtkWidget *menubar;
	
	actions = gtk_action_group_new ("Actions");
	gtk_action_group_add_actions (actions, ui_entries, n_ui_entries, NULL);

	ui = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui, actions, 0);
	g_object_unref (actions);
	gtk_window_add_accel_group (GTK_WINDOW(window), gtk_ui_manager_get_accel_group (ui));
	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}

	menubar = gtk_ui_manager_get_widget (ui,"/MenuBar");
	gtk_box_pack_start (GTK_BOX (box), menubar, false, false, 0);
	g_object_unref (ui);

}	
		    
static void
realize_cb (GtkWidget		*widget)
{
	int w = widget->allocation.width,
	    h = widget->allocation.height;
	
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	//opengl begin
	if (!gdk_gl_drawable_gl_begin (GDK_GL_DRAWABLE (gldrawable), glcontext))
		return;
	

	glClearDepth (1.0);
	glClearColor (0.15,0.15,0.5,1.0);

	glEnable (GL_COLOR_MATERIAL);
	glEnable (GL_CULL_FACE);

	glViewport (0, 0, w, h);

	glEnable(GL_BLEND); glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gdk_gl_drawable_gl_end (GDK_GL_DRAWABLE (gldrawable));
}

static GtkWidget*
create_shared_context_widget ()
{
	GtkWidget *drawing_area;

	GdkGLConfig *glconfig = gdk_gl_config_new_by_mode (GdkGLConfigMode (
							   GDK_GL_MODE_DOUBLE |
							   GDK_GL_MODE_RGBA   |
							   GDK_GL_MODE_DEPTH  ));
	if (glconfig == NULL)
	{
		std::cerr << "Can not find the double-buffered visual.\n \
			Trying single-buffer visual.\n";
		glconfig = gdk_gl_config_new_by_mode (GdkGLConfigMode (
						      GDK_GL_MODE_RGBA	|
						      GDK_GL_MODE_DEPTH ));
		if (glconfig == NULL)
		{
			std::cerr << "No appropriate OpenGL-visual found.\n";
			exit(1);
		}
	}
	
	drawing_area = gtk_drawing_area_new ();

	gtk_widget_set_gl_capability (drawing_area,
				      glconfig,
				      NULL,
				      TRUE,
				      GDK_GL_RGBA_TYPE);
	// Disable back store feature of the widget.
	gtk_widget_set_double_buffered (drawing_area, FALSE);

	g_signal_connect_after (G_OBJECT (drawing_area), "realize",
				G_CALLBACK (realize_cb), NULL);

	return drawing_area;
}


