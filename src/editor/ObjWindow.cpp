#include <ObjWindow.h>
#include <MapObj.h>
#include <GameGlobals.h>
#include <UndoSys.h>

#include <gtk/gtk.h>
#include <iostream>

static Vec2f map_pos;
GSList *btn_group=NULL;

/*
==============
 place_map_object
==============
*/
static void
place_map_object ()
{
	using namespace UndoSys;
	GtkWidget *btn;
	GSList *b=btn_group;
	while (b)
	{
		btn = GTK_WIDGET(b->data);
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(btn)))
		{
			const char *name = gtk_button_get_label (GTK_BUTTON (btn));
			int i;
			for (i=0; i < g_numObjTypes; i++)
				if (!strcmp(name, g_objTypes[i].desc))
				{

					MapObj *obj = NewMapObj (i);
					obj->ent.pos = Vec3f (map_pos.x, map_pos.y, 0.1);
					// add undo
					AddMapObjAction *act = new AddMapObjAction (obj);
					AddUndo (act);
				}

			break;
		}

		b = b->next;
	}
}

static void ok_button_cb (GtkWidget *widget, GtkWidget *window)
{
	gtk_widget_hide_all (window);
	place_map_object ();
}
/*
===============
 open_object_window
===============
*/

void 
open_object_window (Vec2f _map_pos)
{

	using namespace std;

	map_pos = _map_pos;
	static GtkWidget *window = NULL;

	GtkWidget *expander, *expbox,
		  *vbox,*btn;
	GSList *group=NULL;



	if (window)
	{
		gtk_widget_show_all (window);
		return;
	}

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 200,200);
	g_signal_connect (window, "delete_event",
			G_CALLBACK (gtk_widget_hide_on_delete), &window);
	
	vbox = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	

	btn = gtk_button_new_with_label ("OK");
	g_signal_connect (btn, "clicked",
			G_CALLBACK (ok_button_cb), window);
	gtk_box_pack_start (GTK_BOX (vbox), btn, FALSE, TRUE, 10);


	int c=0;
	const char** cat = g_objTypeCats;
	while (cat[c])
	{
		expander = gtk_expander_new (cat[c]);
		gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 10);
		expbox = gtk_vbox_new (FALSE, 5);
		gtk_container_add (GTK_CONTAINER (expander), expbox);

		for (int i=0; i < g_numObjTypes; i++)
		{
			if (!strcmp(g_objTypes[i].cat, cat[c]))
			{
				btn = gtk_radio_button_new_with_label (group, (const char*)g_objTypes[i].desc);
				gtk_box_pack_start (GTK_BOX (expbox), btn, FALSE, FALSE, 0);
				group = gtk_radio_button_get_group (GTK_RADIO_BUTTON(btn));
			}
		}
		c++;
	}



	btn_group = group;
	gtk_widget_show_all (window);
}

