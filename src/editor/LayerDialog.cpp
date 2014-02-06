#include <LayerDialog.h>
#include <Dockable.h>
#include <Notifiable.h>
#include <LayerPropDialog.h>


#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>

#include <GL/gl.h>

#include <Tileset.h>
#include <Map.h>
#include <Renderer.h>
#include <Global.h>



static const float THUMB_SIZE=100.;
enum
{
	ITEM_THUMB,
	ITEM_NAME,
	N_COLUMNS
};
#define LAYER_DIALOG_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), LAYER_DIALOG_TYPE, LayerDialogPrivate))
struct LayerDialogPrivate
{
	GtkWidget *dragger;
	bool dispose_has_run;
};
static GObjectClass *parent_class=NULL;

static void layer_dialog_class_init (LayerDialogClass *klass);
static void layer_dialog_init (LayerDialog *self);
static void layer_dialog_dispose (GObject *obj);

static void dockable_iface_init (gpointer g_iface, gpointer iface_init);
static GtkWidget* layer_dialog_get_dragger (Dockable *dock);

static void notifiable_iface_init (gpointer g_iface, gpointer iface_init);
static void layer_dialog_notify (Notifiable *nf, UiNotify note);

static void update_thumb (GdkPixbuf *pixbuf,Layer *layer);
static void build_layers_store (LayerDialog *ld);
static void properties_clicked_cb (GtkWidget *btn, LayerDialog *ld);

GType
layer_dialog_get_type (void)
{
	static GType ld_type = 0;

	if (!ld_type)
	{
		static const GTypeInfo ld_info =
		{
			sizeof (LayerDialogClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)layer_dialog_class_init, /* class_init */
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (LayerDialog),
			0,
			(GInstanceInitFunc) layer_dialog_init,
		};
		static const GInterfaceInfo dock_info = 
		{
			dockable_iface_init, // interface_init
			NULL, // interface finalize
			NULL // interface_data
		};
		static const GInterfaceInfo note_info = 
		{
			notifiable_iface_init, // interface_init
			NULL, // interface finalize
			NULL // interface_data
		};

		ld_type = g_type_register_static (GTK_TYPE_TABLE, "LayerDialog", &ld_info, GTypeFlags(0));

		g_type_add_interface_static (ld_type, DOCKABLE_TYPE, &dock_info);
		g_type_add_interface_static (ld_type, NOTIFIABLE_TYPE, &note_info);
	}

	return ld_type;
}

GtkWidget*
layer_dialog_new ()
{
	return GTK_WIDGET (g_object_new (LAYER_DIALOG_TYPE, NULL));
}
void
layer_dialog_selected_layer_changed (LayerDialog *mw)
{
}
void layer_dialog_level_changed (LayerDialog *ld)
{
	build_layers_store (ld);
}

static void
dragger_realize_cb (GtkWidget *widget,
		gpointer data)
{
	GdkCursor *cursor = gdk_cursor_new (GDK_HAND2);
	gdk_window_set_cursor (widget->window, cursor);
	gdk_cursor_unref (cursor);
}

static void
layer_dialog_class_init (LayerDialogClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = layer_dialog_dispose;

	parent_class = (GObjectClass*)g_type_class_peek_parent (klass);

	g_type_class_add_private (gobject_class, sizeof (LayerDialogPrivate));
}
/*
============================
 layer_dialog_init
============================
*/
static void
layer_dialog_init (LayerDialog *self)
{
	LayerDialogPrivate *priv = LAYER_DIALOG_GET_PRIVATE (self);
	priv->dispose_has_run = false;

	Ui::AddNotifiable (NOTIFIABLE (self));

	gtk_widget_set_name (GTK_WIDGET (self), "Map");
	gtk_container_set_border_width (GTK_CONTAINER (self), 1);
	gtk_table_set_homogeneous (GTK_TABLE (self), FALSE);
	gtk_table_resize (GTK_TABLE (self), 3,2);

	// TreeView
	GtkWidget *tree,
		  *sw;

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
			GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
	gtk_table_attach_defaults (GTK_TABLE (self), sw, 0,1,1,2);


	// create tree view
	self->layers = gtk_list_store_new (N_COLUMNS,GDK_TYPE_PIXBUF,G_TYPE_STRING);

	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL(self->layers));
	gtk_container_add (GTK_CONTAINER (sw), tree);
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree)), GTK_SELECTION_SINGLE);

	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree),
						-1, NULL, renderer,
						"pixbuf", ITEM_THUMB,
						NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree),
						-1, NULL, renderer,
						"text", ITEM_NAME,
						NULL);
	g_object_set (G_OBJECT (renderer), "editable", TRUE, NULL);

	
	GtkWidget *box, *btn;
	box = gtk_hbox_new (TRUE, 10);

	gtk_table_attach (GTK_TABLE (self), box, 0,1,2,3,
			(GtkAttachOptions)GTK_FILL, (GtkAttachOptions)GTK_FILL, 0, 0);
	btn = gtk_button_new_with_label ("new");
	gtk_container_add (GTK_CONTAINER (box), btn);
	btn = gtk_button_new_with_label ("properties");
	gtk_container_add (GTK_CONTAINER (box), btn);

	g_signal_connect (G_OBJECT (btn), "clicked",
			G_CALLBACK (properties_clicked_cb), self);
	
	gtk_widget_show_all (box);


	// DRAGGER
	GtkWidget *label;
	priv->dragger = gtk_event_box_new ();
	gtk_table_attach (GTK_TABLE (self), priv->dragger, 0,1,0,1,
			(GtkAttachOptions)GTK_FILL, (GtkAttachOptions)GTK_FILL, 0, 0);
	g_signal_connect_after (G_OBJECT (priv->dragger), "realize",
				G_CALLBACK (dragger_realize_cb), NULL);
	label = gtk_label_new ("Layers");
	gtk_container_add (GTK_CONTAINER (priv->dragger), label);
	gtk_widget_show_all (priv->dragger);


}
static void
layer_dialog_dispose (GObject* obj)
{
	LayerDialog *self = LAYER_DIALOG (obj);
	LayerDialogPrivate *priv = LAYER_DIALOG_GET_PRIVATE (self);

	if (priv->dispose_has_run)
		return;
	priv->dispose_has_run = TRUE;
	
	g_object_unref (self->layers);

	Ui::RemoveNotifiable (NOTIFIABLE (self));

	// chain up to the parent class
	if (G_OBJECT_CLASS (parent_class)->dispose)
		G_OBJECT_CLASS (parent_class)->dispose (obj);
}

/*
================
dockable_iface_init
================
*/
static void
dockable_iface_init (gpointer g_iface,
		     gpointer iface_init)
{
	DockableIface *iface = (DockableIface*)g_iface;
	iface->get_dragger = layer_dialog_get_dragger;
}
/*
================
 layer_dialog_get_dragger
================
*/
static GtkWidget*
layer_dialog_get_dragger (Dockable *dock)
{
	LayerDialogPrivate *priv = LAYER_DIALOG_GET_PRIVATE (dock);
	return priv->dragger;
}
/*
=============
 notifiable_iface_init
=============
*/
static void
notifiable_iface_init (gpointer g_iface,
		     gpointer iface_init)
{
	NotifiableIface *iface = (NotifiableIface*)g_iface;
	iface->notify = layer_dialog_notify;
}
static void
layer_dialog_notify (Notifiable *nf, UiNotify note)
{
	LayerDialog *ld = LAYER_DIALOG (nf);
	switch (note)
	{
		case UI_NEW_MAP:
			build_layers_store (ld);
			break;
		case UI_LEVEL_EDIT:
		{
			// Update Thumbnail
			GtkTreeIter iter;
			GdkPixbuf *pixbuf;

			gtk_tree_model_get_iter_first (GTK_TREE_MODEL (ld->layers), &iter);
			Map &map = *GetMap();
			Layer *lay;
			for (size_t i=0; i < map.layers.size(); i++)
			{
				lay = &map.layers[i];
				if (lay == G.selLayer)
					break;
				if(!gtk_tree_model_iter_next (GTK_TREE_MODEL (ld->layers), &iter))
					break;
			}
			Tileset *tset = GetTileset (lay->tileset);
			float w = lay->width*tset->tileW,
			      h = lay->height*tset->tileH;
			float size = std::max (w,h);
			int pix_w = w/size * THUMB_SIZE,
			    pix_h = h/size * THUMB_SIZE;
			pixbuf = gdk_pixbuf_new (
					GDK_COLORSPACE_RGB,
					FALSE,8, pix_w,pix_h);
			update_thumb (pixbuf, lay);
			gtk_list_store_set (ld->layers, &iter,
					ITEM_THUMB, pixbuf,
					-1);
			g_object_unref (pixbuf);
		}
		break;					
		default:
		break;
	}
}
/*
=============
 update_thumb
=============
*/
static void
update_thumb (GdkPixbuf *pixbuf, Layer *lay)
{
	int pix_w,
	    pix_h;
	pix_w = gdk_pixbuf_get_width (pixbuf);
	pix_h = gdk_pixbuf_get_height (pixbuf);
	
	GdkGLContext *glcontext  = G.glContext;
	GdkGLDrawable *gldrawable = gdk_gl_context_get_gl_drawable (glcontext);

	//opengl begin
	if (!gdk_gl_drawable_gl_begin (GDK_GL_DRAWABLE (gldrawable), glcontext))
		return;

	glViewport (0,0,pix_w,pix_h);
	glClearColor (G.bgColor.x,G.bgColor.y,G.bgColor.z, 1.0);
	glClear (GL_COLOR_BUFFER_BIT);
	if (G.selLayer)
	{
		// Set Projection Matrix
		Tileset *tset = GetTileset (G.selLayer->tileset);
		float lw = G.selLayer->width*tset->tileW,
		      lh = G.selLayer->height*tset->tileH;
		float fw = (float)pix_w,
		      fh = (float)pix_h;
		Camera cam;
		cam.SetProjection (fw,fh, 50.0);
		float cotan = cam.projMat.M[0][0];
		cam.mat.SetTranslation(Vec3f(lw*0.5,-lh*0.5, cotan*lw*0.5));
		glMatrixMode (GL_MODELVIEW);
		glLoadIdentity();

		// Render Layer
		GetMap()->Render (cam);
	}
	gdk_gl_drawable_gl_end (GDK_GL_DRAWABLE (gldrawable));

	if (gdk_gl_drawable_is_double_buffered (GDK_GL_DRAWABLE (gldrawable)))
		gdk_gl_drawable_swap_buffers (GDK_GL_DRAWABLE (gldrawable));
	else
		glFlush();
	// opengl end
	int bpp = 3;
	unsigned char pixels[pix_w*pix_h*bpp];
	glReadPixels (0,0,pix_w,pix_h,
			GL_RGB,GL_UNSIGNED_BYTE,
			pixels);
	guchar *p = gdk_pixbuf_get_pixels (pixbuf);
	int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	for (int y=0; y < pix_h; y++)
		for (int x=0; x < pix_w; x++)
		{
			for (int i=0; i < 3; i++)
				p[y*rowstride + x*3+i] = pixels[(pix_h-y-1)*pix_w*3+x*3+i];
		}
}

static void
build_layers_store (LayerDialog *ld)
{
	GtkTreeIter iter;
	GdkPixbuf *pixbuf;
	gtk_list_store_clear (ld->layers);
	Map &map = *GetMap();
	for (size_t i=0; i < map.layers.size(); i++)
	{
		Layer *l = &map.layers[i];

		Tileset *tset = GetTileset (l->tileset);
		float w = l->width*tset->tileW,
		      h = l->height*tset->tileH;
		float size = std::max (w,h);
		int pix_w = w/size * THUMB_SIZE,
		    pix_h = h/size * THUMB_SIZE;
		pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
					FALSE,8, pix_w,pix_h);
		update_thumb (pixbuf, l);
		gtk_list_store_append (ld->layers, &iter);

		gtk_list_store_set (ld->layers, &iter,
				ITEM_THUMB, pixbuf,
				ITEM_NAME, l->name.c_str(),
				-1);
		g_object_unref (pixbuf);
	}

}

static void
properties_clicked_cb (GtkWidget *btn, LayerDialog *ld)
{
	open_layer_prop_dialog ();

}


