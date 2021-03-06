#include <LayerPropDialog.h>
#include <Global.h>
#include <Layer.h>

#include <libxml/xmlreader.h>
#include <string.h>
#include <gtk/gtktreestore.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

enum
{
	NAME=0,
	FILENAME
};

static int
get_tileset_name (const char *fn, char *name)
{
	using namespace std;
	xmlDocPtr doc;
	xmlNodePtr root,
		   node;


	doc = xmlParseFile ((const char*)fn);
	if (doc == NULL)
	{
		cerr << "Could not load tileset: " << fn << "\n";
		return FALSE;
	}
	root = xmlDocGetRootElement (doc);
	if (root == NULL)
	{
		cerr << "Empty tileset file: " << fn << "\n";
		xmlFreeDoc (doc);
		return FALSE;
	}

	node = root->xmlChildrenNode;
	
	xmlChar *value;
	while (node)
	{
		if (!xmlStrcmp (node->name, BAD_CAST "name"))
		{
			value = xmlNodeListGetString (doc, node->xmlChildrenNode, 1);
			strcpy (name, (char*)value);
		}

		node = node->next;
	}


	xmlFreeDoc (doc);
	xmlCleanupParser ();
	xmlMemoryDump ();
	return TRUE;
}

static void
build_tset_store (GtkTreeStore *store)
{
	GtkTreeIter iter;
	GDir *dir;
	gchar *fn, name[128];
	GError *error = NULL;

	gtk_tree_store_clear (store);

	dir = g_dir_open (".", 0, &error);
	if (error)
	{
		fprintf (stderr, "Unable to open tileset dir: %s\n", error->message);
		return;
	}
	
	fn = (char*)g_dir_read_name (dir);
	while (fn)
	{
		int len = strlen (fn);
		if (len > 5 && !strcmp (fn+len-5,".tset"))
		{
			get_tileset_name (fn, name);
			gtk_tree_store_append (store, &iter, NULL);
			gtk_tree_store_set (store, &iter, NAME, name, -1);		
			gtk_tree_store_set (store, &iter, FILENAME, fn, -1);		
		}
		fn = (char*)g_dir_read_name (dir);
	}
	g_dir_close (dir);
}
static void
tset_combo_set_active (GtkComboBox *combo)
{
	GtkTreeModel *mod;
	GtkTreeIter iter;
	gint i,n;

	mod = gtk_combo_box_get_model (combo);
	i = gtk_tree_model_get_iter_first (mod, &iter);
	n=0;
	while (i)
	{
		GValue value = {0, {{0}}};
		gtk_tree_model_get_value (mod, &iter, FILENAME, &value);
		char *name = GetTileset (G.selLayer->tileset)->name;
		if (!strcmp(g_value_get_string (&value), name))
			gtk_combo_box_set_active (combo, n);

		g_value_unset (&value);
		i = gtk_tree_model_iter_next (mod, &iter);
		n++;
	}
}

/*
====================
 open_layer_prop_dialog
====================
*/

void open_layer_prop_dialog ()
{
	GtkTreeStore *store;
	GtkWidget *win,*combo,
		  *bbox, *vbox,*frame,
		  *content_area, *btn;
	GtkCellRenderer *renderer;

	win = gtk_dialog_new_with_buttons ("Layer Properties",
						NULL,
						GTK_DIALOG_MODAL,
						GTK_STOCK_OK,
						GTK_RESPONSE_ACCEPT,
						GTK_STOCK_CANCEL,
						GTK_RESPONSE_REJECT,
						NULL);

	content_area = gtk_dialog_get_content_area (GTK_DIALOG (win));

	vbox = gtk_vbox_new (TRUE, 10);
	gtk_container_add (GTK_CONTAINER (content_area), vbox);

	frame = gtk_frame_new ("Tileset");
	gtk_container_add (GTK_CONTAINER (vbox), frame);

	bbox = gtk_hbutton_box_new ();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
	gtk_container_add (GTK_CONTAINER (frame), bbox);

	// tileset combo box
	store = gtk_tree_store_new (2,G_TYPE_STRING,G_TYPE_STRING);
	build_tset_store (store);

	combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL(store));
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer,
					"text", 0, NULL);
	tset_combo_set_active (GTK_COMBO_BOX(combo));
	gtk_container_add (GTK_CONTAINER (bbox), combo);

	// Dimensions

	frame = gtk_frame_new ("Dimensions");
	gtk_container_add (GTK_CONTAINER (vbox), frame);

	bbox = gtk_hbutton_box_new ();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
	gtk_container_add (GTK_CONTAINER (frame), bbox);

	int w=0,h=0;
	if (G.selLayer)
	{
		w = G.selLayer->width;
		h = G.selLayer->height;
	}
	GtkObject *adjw,*adjh;
	adjw = gtk_adjustment_new (w, 0, (gdouble)(1<<30), 1, 0, 0);
	adjh = gtk_adjustment_new (h, 0, (gdouble)(1<<30), 1, 0, 0);

	btn = gtk_spin_button_new (GTK_ADJUSTMENT(adjw), 1, 0);
	gtk_container_add (GTK_CONTAINER (bbox), btn);

	btn = gtk_spin_button_new (GTK_ADJUSTMENT(adjh), 1, 0);
	gtk_container_add (GTK_CONTAINER (bbox), btn);

	gtk_widget_show_all (win);
	
	gint result = gtk_dialog_run (GTK_DIALOG (win));
	switch (result)
	{
		case GTK_RESPONSE_ACCEPT:
			//update layer
			break;
		default:
			break;
	}
	gtk_widget_destroy (win);

	/*
	hbox = gtk_hbox_new (TRUE, 10);
	gkt_container_add (GTK_CONTAINER (vbox), hbox);
	btn = gtk_button_new_from_stock*/
}

