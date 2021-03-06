#include <MapWindow.h>
#include <Dockable.h>
#include <Notifiable.h>
#include <ObjWindow.h>

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>

#include <GL/gl.h>
#include <iostream>

#include <Tileset.h>
#include <Map.h>
#include <UndoSys.h>
#include <Renderer.h>
#include <Global.h>
#include <GameGlobals.h>
#include <MapObj.h>

#define MAP_WIN_FOV 50.0

#define MAP_WINDOW_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAP_WINDOW_TYPE, MapWindowPrivate))

static GtkWidget *mw_menu=NULL;

static Vec2f map_mouse_pos;

struct MapWindowPrivate
{
	gboolean dispose_has_run;
	GtkWidget *dragger;
};
static GObjectClass *parent_class=NULL;

static void map_window_class_init (MapWindowClass *klass);
static void map_window_init (MapWindow *self);
static void map_window_dispose (GObject *obj);

static void dockable_iface_init (gpointer g_iface, gpointer iface_init);
static GtkWidget* map_window_get_dragger (Dockable *dock);

static void notifiable_iface_init (gpointer g_iface, gpointer iface_init);
static void map_window_notify (Notifiable *nf, UiNotify note);

static void realize_cb			(GtkWidget *widget, MapWindow *mw );
static gboolean configure_cb		(GtkWidget *widget, GdkEventConfigure *event, MapWindow *mw);
static gboolean expose_cb		(GtkWidget *widget, GdkEventExpose *event, MapWindow *mw);
static gboolean enter_cb		(GtkWidget *widget, GdkEvent *event, MapWindow *mw);
static gboolean key_press_cb		(GtkWidget *widget, GdkEventKey *key, 	    MapWindow *mw);
static gboolean button_press_cb		(GtkWidget *widget, GdkEventButton *button, MapWindow *mw);
static gboolean button_release_cb		(GtkWidget *widget, GdkEventButton *button, MapWindow *mw);
static gboolean scroll_cb		(GtkWidget *widget, GdkEventScroll *scroll, MapWindow *mw);
static gboolean motion_cb		(GtkWidget *widget, GdkEventMotion *motion, MapWindow *mw);
static void scrollbar_value_changed_cb (GtkWidget *widget, MapWindow *mw);
static void update_scrollbars (MapWindow *mw);
static void mask_toggled (GtkWidget *widget, MapWindow *tw);

static void redraw (GtkWidget *widget);

static void set_projection (MapWindow *mw);
static Vec2f get_mouse_tile_position (MapWindow *mw);
static void draw (MapWindow *mw);
static void draw_selection_box (Vec3f min, Vec3f max);
static void flip_selection ();
static void write_tiles (MapWindow *mw);
static void read_tiles (MapWindow *mw);

static void delete_map_object (MapObj *obj);

static void create_menu ();


GType
map_window_get_type (void)
{
	static GType mw_type = 0;

	if (!mw_type)
	{
		static const GTypeInfo mw_info =
		{
			sizeof (MapWindowClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)map_window_class_init, /* class_init */
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (MapWindow),
			0,
			(GInstanceInitFunc) map_window_init,
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

		mw_type = g_type_register_static (GTK_TYPE_TABLE, "MapWindow", &mw_info, GTypeFlags(0));

		g_type_add_interface_static (mw_type, DOCKABLE_TYPE, &dock_info);
		g_type_add_interface_static (mw_type, NOTIFIABLE_TYPE, &note_info);
	}

	return mw_type;
}

GtkWidget*
map_window_new ()
{
	return GTK_WIDGET (g_object_new (MAP_WINDOW_TYPE, NULL));
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
map_window_class_init (MapWindowClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = map_window_dispose;

	parent_class = (GObjectClass*)g_type_class_peek_parent (klass);

	g_type_class_add_private (gobject_class, sizeof (MapWindowPrivate));
}
/*
============================
 map_window_init
============================
*/
static void
map_window_init (MapWindow *self)
{
	MapWindowPrivate *priv = MAP_WINDOW_GET_PRIVATE (self);
	priv->dispose_has_run = FALSE;

	Ui::AddNotifiable (NOTIFIABLE (self));

	gtk_widget_set_name (GTK_WIDGET (self), "Map");
	gtk_container_set_border_width (GTK_CONTAINER (self), 1);
	gtk_table_set_homogeneous (GTK_TABLE (self), FALSE);
	gtk_table_resize (GTK_TABLE (self), 3,2);

	// manually call constructors
	new (&self->camera)	Camera ();
	new (&self->selection) TileSelection ();

	self->mouse.x = 0;
	self->mouse.y = 0;
	self->mouse.x_prev = 0;
	self->mouse.y_prev = 0;
	self->state = ED_NONE;
	self->show_mask = false;

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
	
	self->drawing_area = gtk_drawing_area_new ();
	gtk_table_attach_defaults (GTK_TABLE (self), self->drawing_area,
			0,1,1,2);

	gtk_widget_set_gl_capability (self->drawing_area,
				      glconfig,
				      G.glContext,
				      TRUE,
				      GDK_GL_RGBA_TYPE);
	//gtk_widget_set_size_request (self->drawing_area, 200, 200);

	// Disable back store feature of the widget.
	gtk_widget_set_double_buffered (self->drawing_area, FALSE);

	g_signal_connect_after (G_OBJECT (self->drawing_area), "realize",
				G_CALLBACK (realize_cb), self);
	g_signal_connect (G_OBJECT (self->drawing_area), "configure_event",
			  G_CALLBACK (configure_cb), self);
	g_signal_connect (G_OBJECT (self->drawing_area), "expose_event",
			  G_CALLBACK (expose_cb), self);
	g_signal_connect (G_OBJECT (self->drawing_area), "enter_notify_event",
			  G_CALLBACK (enter_cb), self);
	g_signal_connect (G_OBJECT (self->drawing_area), "key_press_event",
			  G_CALLBACK (key_press_cb), self);
	g_signal_connect (G_OBJECT (self->drawing_area), "button_press_event",
			  G_CALLBACK (button_press_cb),	self);
	g_signal_connect (G_OBJECT (self->drawing_area), "button_release_event",
			  G_CALLBACK (button_release_cb), self);
	g_signal_connect (G_OBJECT (self->drawing_area), "scroll_event",
			  G_CALLBACK (scroll_cb), self);
	g_signal_connect (G_OBJECT (self->drawing_area), "motion_notify_event",
			  G_CALLBACK (motion_cb), self);


	gtk_widget_set_events (self->drawing_area, 
				  GDK_BUTTON_PRESS_MASK
				| GDK_BUTTON_RELEASE_MASK
				| GDK_KEY_PRESS_MASK
				| GDK_SCROLL_MASK
				| GDK_POINTER_MOTION_MASK
		       		| GDK_ENTER_NOTIFY_MASK	);

	GTK_WIDGET_SET_FLAGS (self->drawing_area, GTK_CAN_FOCUS);
	gtk_widget_show (self->drawing_area);

	// scrollbars
	self->hscrollbar = gtk_hscrollbar_new (NULL);
	gtk_table_attach (GTK_TABLE (self), self->hscrollbar,
			0,1,2,3, (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (self->hscrollbar), "value_changed",
			G_CALLBACK (scrollbar_value_changed_cb), self);

	self->vscrollbar = gtk_vscrollbar_new (NULL);
	gtk_table_attach (GTK_TABLE (self), self->vscrollbar,
			1,2,1,2, (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_signal_connect (G_OBJECT (self->vscrollbar), "value_changed",
			G_CALLBACK (scrollbar_value_changed_cb), self);

	// HBOX
	GtkWidget *hbox = gtk_hbox_new (false,0);
	gtk_table_attach (GTK_TABLE (self), hbox,
			0,1,0,1, (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), 0, 0);


	// DRAGGER
	GtkWidget *label;
	priv->dragger = gtk_event_box_new ();
	gtk_box_pack_start (GTK_BOX (hbox), priv->dragger, true, true, 0);
	g_signal_connect_after (G_OBJECT (priv->dragger), "realize",
				G_CALLBACK (dragger_realize_cb), NULL);
	label = gtk_label_new ("Map");
	gtk_container_add (GTK_CONTAINER (priv->dragger), label);
	gtk_widget_show_all (priv->dragger);

	//Mask button
	GtkWidget *btn = gtk_toggle_button_new_with_label ("Mask");
	gtk_box_pack_start (GTK_BOX (hbox), btn, false, true, 0);
	gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (btn), false);
	g_signal_connect (G_OBJECT (btn), "toggled",
			G_CALLBACK (mask_toggled), self);

	//POPUP MENU
	if (mw_menu == NULL)
		create_menu ();
	

}

static void
map_window_dispose (GObject* obj)
{
	MapWindow *self = MAP_WINDOW (obj);
	MapWindowPrivate *priv = MAP_WINDOW_GET_PRIVATE (self);

	if (priv->dispose_has_run)
		return;
	priv->dispose_has_run = TRUE;

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
	iface->get_dragger = map_window_get_dragger;
}
/*
================
 map_window_get_dragger
================
*/
static GtkWidget*
map_window_get_dragger (Dockable *dock)
{
	MapWindowPrivate *priv = MAP_WINDOW_GET_PRIVATE (dock);
	return priv->dragger;
}
/*
=================
 notifiable_iface_init
=================
*/
static void
notifiable_iface_init (gpointer g_iface, gpointer iface_init)
{
	NotifiableIface *iface = (NotifiableIface*)g_iface;
	iface->notify = map_window_notify;
}
static void map_window_notify (Notifiable *nf, UiNotify note)
{
	MapWindow *mw = MAP_WINDOW (nf);
	switch (note)
	{
		case UI_NEW_ACTIVE_LAYER:
			update_scrollbars (mw);
			redraw (mw->drawing_area);
			break;
		default:
			break;
	}

}


static void
realize_cb (GtkWidget		*widget,
	    MapWindow	*mw  )
{
	int w = widget->allocation.width,
	    h = widget->allocation.height;
	
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	//opengl begin
	if (!gdk_gl_drawable_gl_begin (GDK_GL_DRAWABLE (gldrawable), glcontext))
		return;
	

	set_projection (mw);	
	glClearDepth (1.0);
	glClearColor (0.15,0.15,0.5,1.0);

	glEnable (GL_COLOR_MATERIAL);
	glEnable (GL_CULL_FACE);

	glViewport (0, 0, w, h);

	gdk_gl_drawable_gl_end (GDK_GL_DRAWABLE (gldrawable));
}

static gboolean
configure_cb (GtkWidget		*widget,
	      GdkEventConfigure	*event,
	      MapWindow	*mw )
{
	int w = widget->allocation.width,
	    h = widget->allocation.height;


	GdkGLContext  *glcontext  = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	// opengl begin
	if (!gdk_gl_drawable_gl_begin (GDK_GL_DRAWABLE (gldrawable), glcontext))
		return FALSE;

	set_projection (mw);
	scrollbar_value_changed_cb (mw->hscrollbar,mw);
	scrollbar_value_changed_cb (mw->vscrollbar,mw);
	glViewport (0, 0, w, h);

	// scrollbars
	gtk_range_set_increments (GTK_RANGE (mw->hscrollbar), 1.0,w);

	gtk_range_set_increments (GTK_RANGE (mw->vscrollbar), 1.0,h);

	// opengl end
	gdk_gl_drawable_gl_end (GDK_GL_DRAWABLE (gldrawable));

	return TRUE;
}	

static gboolean
expose_cb (GtkWidget		*widget,
	   GdkEventExpose	*event,
	   MapWindow	*mw)
{
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	//opengl begin
	if (!gdk_gl_drawable_gl_begin (GDK_GL_DRAWABLE (gldrawable), glcontext))
		return FALSE;
	
	//______DRAW______
	//
	draw (mw);

	gdk_gl_drawable_gl_end (GDK_GL_DRAWABLE (gldrawable));

	if (gdk_gl_drawable_is_double_buffered (GDK_GL_DRAWABLE (gldrawable)))
		gdk_gl_drawable_swap_buffers (GDK_GL_DRAWABLE (gldrawable));
	else
		glFlush();
	// opengl end
	
	return TRUE;
}

static gboolean
enter_cb (GtkWidget		*widget,
	GdkEvent 		*event,
	MapWindow 		*mw)
{
	gtk_widget_grab_focus (widget);
	return TRUE;
}

static gboolean
key_press_cb (GtkWidget			*widget,
	      GdkEventKey		*key,
	      MapWindow		*mw)
{
	
	bool rdraw = false;
	Vec2f ms;
	if (mw->state == ED_NONE)
		switch (key->keyval)
		{
			case GDK_b:
				mw->state = ED_SELECTING;
				mw->selection.Clear();
				ms = get_mouse_tile_position (mw);
				mw->selection.x = ms.x;
				mw->selection.y = ms.y;
				read_tiles (mw);
				rdraw = true;
				break;
			case GDK_f:
				rdraw = true;
				flip_selection ();
				break;
			case GDK_y:
				if (key->state & GDK_CONTROL_MASK)
				{
					// -- REDO
					UndoSys::Redo ();
					rdraw = true;
				}
				break;
			case GDK_z:
				if (key->state & GDK_CONTROL_MASK)
				{
					// -- UNDO
					UndoSys::Undo ();
					rdraw = true;
				}
				break;
			case GDK_Z:
				if (key->state & GDK_CONTROL_MASK)
				{
					// -- REDO
					UndoSys::Redo ();
					rdraw = true;
				}
				break;
			case GDK_BackSpace:
				// clear the selection
				G.selTiles.width = 1;
				G.selTiles.height = 1;
				G.selTiles.tiles[0] = 0;
				G.selTiles.offsetX = 1;
				G.selTiles.offsetY = 1;
				rdraw = true;
				break;
			case GDK_F1:
				break;
			default:
				break;
		}
	else if (mw->state == ED_SELECTING)
		switch (key->keyval)
		{
			case GDK_b:
				mw->state = ED_NONE;
				read_tiles (mw);
				break;
			default:
				break;
		}


	if (rdraw)
		redraw (widget);

	return TRUE;
}
static gboolean
button_press_cb (GtkWidget		*widget,
		 GdkEventButton		*event,
		 MapWindow		*mw )
{
	//extern TileSelection G.selTiles;
	
	switch (event->button)
	{
		case 1:
			if (event->type == GDK_BUTTON_PRESS)
			{
				Layer *lay = G.selLayer;
				if (lay)
				{
					mw->state = ED_WRITING;
					UndoSys::TileAction *undo = new UndoSys::TileAction(lay);
					UndoSys::AddUndo (undo);
					write_tiles (mw);
				}
			}
			break;
		case 3:
			if (event->type == GDK_BUTTON_PRESS)
			{
				Layer *lay = G.selLayer;
				map_mouse_pos = get_mouse_tile_position(mw);
				map_mouse_pos.x *= lay->tileW;
				map_mouse_pos.x += lay->tileW/2;
				map_mouse_pos.y *= -lay->tileH;
				map_mouse_pos.y += -lay->tileH/2;

				GdkEventButton *bevent = (GdkEventButton*)event;
				gtk_menu_popup (GTK_MENU(mw_menu), NULL, NULL, NULL,NULL, bevent->button, bevent->time);
			}
			break;
		default:
			break;
	}

	redraw (widget);
	return TRUE;
}
static gboolean
button_release_cb (GtkWidget		*widget,
		 GdkEventButton		*event,
		 MapWindow		*mw )
{
	//extern TileSelection G.selTiles;
	
	switch (event->button)
	{
		case 1:
			if (event->type == GDK_BUTTON_RELEASE)
			{
				if (mw->state == ED_WRITING)
					Ui::Notify (UI_LEVEL_EDIT);
				mw->state = ED_NONE;
			}

			break;
		default:
			break;
	}

	redraw (widget);
	return TRUE;
}
static gboolean
scroll_cb (GtkWidget		*widget,
	   GdkEventScroll	*scroll,
	   MapWindow	*mw)
{
	/*
	if (scroll->direction == GDK_SCROLL_UP)
	{
		return TRUE;
	}
	if (scroll->direction == GDK_SCROLL_DOWN)
	{
		return TRUE;
	}
	*/
	return FALSE;
}
static gboolean
motion_cb (GtkWidget		*widget,
	   GdkEventMotion	*motion,
	   MapWindow	*mw )

{
	mw->mouse.x_prev = mw->mouse.x;
	mw->mouse.y_prev = mw->mouse.y;
	mw->mouse.x = (int)motion->x;
	mw->mouse.y = (int)motion->y;

	//int distx = mw->mouse.x - mw->mouse.x_prev;
	//int disty = mw->mouse.y - mw->mouse.y_prev;

	
	Layer *lay = G.selLayer;
	if (motion->state & GDK_BUTTON1_MASK)
	{
		if (lay)
			write_tiles (mw);
	}
	if (mw->state == ED_SELECTING)
	{

		Vec2f ms = get_mouse_tile_position (mw);
		mw->selection.offsetX = mw->selection.x - ms.x;
		mw->selection.offsetY = mw->selection.y - ms.y;
		if (mw->selection.offsetX < 0)
			mw->selection.offsetX = -1;
		else mw->selection.offsetX = 1;
		if (mw->selection.offsetY < 0)
			mw->selection.offsetY = -1;
		else mw->selection.offsetY = 1;
		mw->selection.width = abs (mw->selection.x - ms.x)+1; 
		mw->selection.height = abs (mw->selection.y - ms.y)+1;

		read_tiles (mw);

	}
	redraw (widget);

	return TRUE;
}
/*
====================
 scrollbar callbacks
====================
*/
static void
scrollbar_value_changed_cb (GtkWidget *widget,
			    MapWindow *mw)
{

	float value = gtk_range_get_value (GTK_RANGE (widget));
	Vec3f pos = mw->camera.mat.GetTranslation ();
	if (G.selLayer == NULL) return;
	Vec2f s = mw->camera.GetDrawScale (G.selLayer->depth);
	s.x = 1.0/s.x;
	s.y = 1.0/s.y;
	if (widget == mw->hscrollbar)
	{
		pos.x = value+s.x;
	}
	else pos.y = -value-s.y;
	mw->camera.mat.SetTranslation (pos);
	redraw (mw->drawing_area);
}
static void
update_scrollbars (MapWindow *mw)
{
	Layer *lay = G.selLayer;
	if (lay == NULL) return;

	Vec2f ds = mw->camera.GetDrawScale (lay->depth);
	float rx = lay->width - 2.0/(lay->tileW*ds.x),
		ry = lay->height - 2.0/(lay->tileH*ds.y);
	rx *= lay->tileW;
	ry *= lay->tileH;
	if (rx < 0) rx = 0.001;
	if (ry < 0) ry = 0.001;
	gtk_range_set_range (GTK_RANGE (mw->hscrollbar), 0,rx);
	gtk_range_set_range (GTK_RANGE (mw->vscrollbar), 0,ry);

}
static void redraw (GtkWidget	*widget)
{
	GdkRectangle area;
	area.x = 0;
	area.y = 0;
	area.width = widget->allocation.width;
	area.height = widget->allocation.height;

	gdk_window_invalidate_rect (widget->window, &area, FALSE);
}

static void set_projection (MapWindow *mw)
{
	int w = mw->drawing_area->allocation.width,
	    h = mw->drawing_area->allocation.height;

	float fw = (float)w,
	      fh = (float)h;
	mw->camera.SetProjection (w,h,MAP_WIN_FOV);
	float cotan = mw->camera.projMat.M[0][0];
	mw->camera.mat.SetTranslation(Vec3f(fw*0.5,-fh*0.5,cotan*w*0.5));
	update_scrollbars (mw);
}
static void
mask_toggled (GtkWidget *widget, MapWindow *tw)
{
	tw->show_mask = !tw->show_mask;
	redraw (tw->drawing_area);
}
static Vec2f get_mouse_tile_position (MapWindow *mw)
{
	Vec2f ret(0.,0.);
	Layer *lay = G.selLayer;
	if (lay == NULL)
		return ret;
	Vec3f cpos = mw->camera.mat.GetTranslation();
	float sw = mw->drawing_area->allocation.width,
	      sh = mw->drawing_area->allocation.height;
	Vec2f ds = mw->camera.GetDrawScale (lay->depth);
	ret.x = ((float)mw->mouse.x*2.0/sw-1.0 + cpos.x*ds.x)/(lay->tileW*ds.x);
	ret.y = ((float)mw->mouse.y*2.0/sh-1.0 - cpos.y*ds.y)/(lay->tileH*ds.y);
	ret.x = float((int)ret.x);
	ret.y = float((int)ret.y);
	return ret;
}
/*
============
 DRAW
============
*/
static void draw (MapWindow *mw)
{

	Map &map = *GetMap();
	if (G.selLayer == NULL) return;
	Tileset &tileset = *GetTileset (G.selLayer->tileset);
	Renderer *renderer = GetRenderer ();
	
	Layer *lay = G.selLayer;
	if (!lay) return;

	renderer->BindTileset (G.selLayer->tileset);
	float w = (float)lay->width * lay->tileW,
	      h = (float)lay->height * lay->tileH;
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	Vec3f camtran = -mw->camera.mat.GetTranslation();
	glTranslatef (camtran.x,camtran.y,camtran.z);
	// draw a quad the size of the layer
	glColor3fv (G.bgColor.V);
	glBegin (GL_QUADS);
	glVertex2f (0.0,0.0);
	glVertex2f (0.0,-h);
	glVertex2f (w,-h);
	glVertex2f (w,0.0);
	glEnd ();


	// Selection
	
	Vec2f ms = get_mouse_tile_position (mw);	
	float x1,y1,
	      x2 = ms.x + G.selTiles.width*G.selTiles.offsetX,
	      y2 = ms.y + G.selTiles.height*G.selTiles.offsetY,
	      z1 = G.selTiles.maxDepth,
	      z2 = G.selTiles.minDepth;
	if (G.selTiles.offsetX > 0)
		x1 = ms.x;
	else{ x1 = x2+1; x2 = ms.x+1; }
	if (G.selTiles.offsetY > 0)
		y1 = ms.y;
	else{ y1 = y2+1; y2 = ms.y+1; }
	
	x1 *= lay->tileW;
	x2 *= lay->tileW;
	y1 *= -lay->tileH;
	y2 *= -lay->tileH;
	glEnable (GL_DEPTH_TEST);

	glColor3f (1.0,1.0,1.0);
	
	glClear (GL_DEPTH_BUFFER_BIT);
	renderer->SetShowMask (mw->show_mask);
	if (mw->show_mask)
		glColor3f (0.,0.,0.);
	//---- Render Scene -------
	map.Render (mw->camera);


	glEnable (GL_BLEND);
	if (mw->state != ED_SELECTING)
	{
		// draw selection

		if (mw->show_mask == false)
		{
			Vec4f col = G.bgColor;
			if (G.selTiles.width*G.selTiles.height == 1 && G.selTiles.tiles[0] == 0)
			{
				col = col*0.7;
				int tx = (int)x1/lay->tileW,
				    ty = -(int)y1/lay->tileH;
				if (tx > lay->width-1) tx = lay->width-1;
				else if (tx < 0) tx = 0;
				if (ty > lay->height-1) ty = lay->height-1;
				else if (ty < 0) ty = 0;

				z1 = tileset.tiles[lay->tiles[ty*lay->width+tx]].maxDepth+0.01;
				z2 = tileset.tiles[lay->tiles[ty*lay->width+tx]].minDepth;
			}
			col.V[3] = 0.3;
			glColor4fv (col.V);

			draw_selection_box (Vec3f(x1,y1,z1), Vec3f(x2,y2,z2));

			glColor3f (1.0,1.0,1.0);
		}

		ClipRecti clip(0,0,G.selTiles.width,G.selTiles.height);

		glClear (GL_DEPTH_BUFFER_BIT);
		glLoadIdentity ();
		glTranslatef (camtran.x+x1,camtran.y+y1,camtran.z);
		if (mw->show_mask == false)
		{
			glEnable (GL_TEXTURE_2D);
			glBindTexture (GL_TEXTURE_2D, tileset.texture);
			glCullFace (GL_FRONT);
			glColor4f (0,0,0,0);
			for (int i = 2; i; i--)
			{
				renderer->SetRenderPass (R_OPAQUE);
				renderer->DrawTiles (G.selTiles.tiles,
						G.selTiles.width,G.selTiles.height, lay->tileW, lay->tileH, clip);

				renderer->SetRenderPass (R_ALPHA);
				renderer->DrawTiles (G.selTiles.tiles,
						G.selTiles.width,G.selTiles.height, lay->tileW, lay->tileH, clip);

				renderer->SetRenderPass (R_ALPHA_FRONT);
				renderer->DrawTiles (G.selTiles.tiles,
						G.selTiles.width,G.selTiles.height, lay->tileW, lay->tileH, clip);
				glCullFace (GL_BACK);
				glColor4f (1.0,1.0,1.0,0.6);
			}
			glDisable (GL_TEXTURE_2D);
		}
		else // show mask
		{
			glColor3f (0.,0.,0.);
			renderer->DrawMask (G.selTiles.tiles,
					G.selTiles.width,G.selTiles.height, lay->tileW, lay->tileH, clip);
			glColor3f (1.,1.,1.);
		}

	}
	// Selection Box
	if (mw->state == ED_SELECTING) 
	{
		Vec2f ms = get_mouse_tile_position (mw);
		float x1,x2,
		      y1,y2,
		      z1 = mw->selection.maxDepth+0.1,
		      z2 = mw->selection.minDepth;
		if (ms.x < mw->selection.x)
		{
			x1 = ms.x; x2 = mw->selection.x;
		}
		else{ x2 = ms.x; x1 = mw->selection.x; }
		if (ms.y < mw->selection.y)
		{
			y1 = ms.y; y2 = mw->selection.y;
		}
		else{ y2 = ms.y; y1 = mw->selection.y; }

		x2+=1.0;
		y2+=1.0;	
		x1 = std::max (x1,0.0f);
		x2 = std::min (x2,(float)lay->width);
		y1 = std::max (y1,0.0f);
		y2 = std::min (y2,(float)lay->height);
		x1 *= lay->tileW;
		x2 *= lay->tileW;
		y1 *= -lay->tileH;
		y2 *= -lay->tileH;

		x1 -= 0.01;
		x2 += 0.01;
		y1 += 0.01;
		y2 -= 0.01;

		glBlendFunc (GL_ONE_MINUS_DST_COLOR, GL_ZERO);

		glColor3f (1.,1.,1.);
		draw_selection_box (Vec3f (x1,y1,z1), Vec3f (x2,y2,z2));	

		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glDisable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);

}
static void
draw_selection_box (Vec3f min, Vec3f max)
{
	glBegin (GL_QUADS);
	
	// front	
	glVertex3f (min.x,min.y,min.z);
	glVertex3f (min.x,max.y,min.z);
	glVertex3f (max.x,max.y,min.z);
	glVertex3f (max.x,min.y,min.z);

	// bottom
	glVertex3f (min.x,max.y,min.z);
	glVertex3f (min.x,max.y,max.z);
	glVertex3f (max.x,max.y,max.z);
	glVertex3f (max.x,max.y,min.z);

	// top
	glVertex3f (min.x,min.y,min.z);
	glVertex3f (max.x,min.y,min.z);
	glVertex3f (max.x,min.y,max.z);
	glVertex3f (min.x,min.y,max.z);

	// left
	glVertex3f (min.x,min.y,min.z);
	glVertex3f (min.x,min.y,max.z);
	glVertex3f (min.x,max.y,max.z);
	glVertex3f (min.x,max.y,min.z);

	// right
	glVertex3f (max.x,min.y,min.z);
	glVertex3f (max.x,max.y,min.z);
	glVertex3f (max.x,max.y,max.z);
	glVertex3f (max.x,min.y,max.z);
	glEnd ();

}

/*
================
 flip_selection
================
*/
static void
flip_selection ()
{

	if (G.selLayer == NULL) return;
	TileSelection &sel = G.selTiles;
	Tileset &tset = *GetTileset (G.selLayer->tileset);

	int *dest = new int[sel.size];

	for (int y=0; y < sel.height; y++)
		for (int x=0; x < sel.width; x++)
		{

			int st = y*sel.width + x;
			int dt = y*sel.width + sel.width-x-1;
			dest[dt] = sel.tiles[st];
			int t = dest[dt];
			if (t && tset.tiles[t].flip == 0)
			{
				tset.AddFlippedTile (t);
				t = tset.tiles[t].flip;
			}
			else if (t && tset.tiles[t].flip < 0) // already flipped
				t = -tset.tiles[t].flip;
			else t = tset.tiles[t].flip;
			dest[dt] = t;
		}

	delete[] sel.tiles;
	sel.tiles = dest;
}
static void
write_tiles (MapWindow *mw)
{
	using namespace std;
	using namespace UndoSys;

	TileAction *undo = static_cast<TileAction*>(GetCurrentUndo ());
	
	Layer *lay = undo->layer;
	if (!lay) return;
	Vec2f ms = get_mouse_tile_position (mw);
	int x1,
	    y1,
	    x2 = ms.x+G.selTiles.width*G.selTiles.offsetX,
	    y2 = ms.y+G.selTiles.height*G.selTiles.offsetY;
	if (G.selTiles.offsetX < 0)
	{
		x1 = x2+1; x2 = ms.x+1;
	} else x1 = ms.x;
	if (G.selTiles.offsetY < 0)
	{
		y1 = y2+1; y2 = ms.y+1;
	} else y1 = ms.y;
	x2 = min (x2,lay->width);
	y2 = min (y2,lay->height);

	int ssx,s,sy;
	if (x1 < 0)
	{
		x1 = 0;
		ssx = G.selTiles.width - (x2-x1);
	}
	else ssx = 0;
	if (y1 < 0)
	{
		y1 = 0;
		sy = G.selTiles.height - (y2-y1);
	}
	else sy = 0;

	for (int y=y1; y < y2; y++,sy++)
	{
		s = G.selTiles.width*sy + ssx;
		for (int x=x1; x < x2; x++,s++)
		{
			int t = x+y*lay->width;
			if (lay->tiles[t] == G.selTiles.tiles[s])
				continue;

			undo->AddTile(t);
			lay->tiles[t] = G.selTiles.tiles[s];
		}
	}

	// remove map object if necessary
	if (G.selTiles.height*G.selTiles.width == 1 && G.selTiles.tiles[0] == 0)
	{
		Entity *e = g_entities;
		while (e)
		{
			if (int(ms.x) == int(e->pos.x/lay->tileW) && -int(ms.y) == int(e->pos.y/lay->tileH))
				delete_map_object (MAP_OBJ(e));
			e = e->next;
		}
	}

}
/*
===================
 read_tiles
===================
*/

static void
read_tiles (MapWindow *mw)
{

	Layer *lay = G.selLayer;
	if (lay == NULL) return;

	Tileset &tileset = *GetTileset (lay->tileset);

	Vec2f ms = get_mouse_tile_position (mw);
	int mx = (int)ms.x,
	    my = (int)ms.y;
	int x1,x2,
	    y1,y2;
	int mapW = lay->width,
	    mapH = lay->height;
	if (mx < mw->selection.x)
	{
		x1 = mx; x2 = mw->selection.x;
	}
	else{ x2 = mx; x1 = mw->selection.x; }
	if (my < mw->selection.y)
	{
		y1 = my; y2 = mw->selection.y;
	}
	else{ y2 = my; y1 = mw->selection.y; }

	x2++;
	y2++;	
	x1 = std::max (x1,0);
	x2 = std::min (x2,mapW);
	y1 = std::max (y1,0);
	y2 = std::min (y2,mapH);
	mw->selection.width = x2-x1;
	mw->selection.height = y2-y1;
	int i=0;
	float min=0,max=0;
	for (int y = y1; y < y2; y++)
		for (int x = x1; x < x2; x++,i++)
		{
			int t = lay->tiles[x + y*mapW];
			if (tileset.tiles[t].minDepth < min)
				min = tileset.tiles[t].minDepth;	
			if (tileset.tiles[t].maxDepth > max)
				max = tileset.tiles[t].maxDepth;
		}
	mw->selection.minDepth = min;
	mw->selection.maxDepth = max;
	if (mw->state == ED_NONE)
	{
		// copy to global
		G.selTiles = mw->selection;

		// read tiles
		int size = G.selTiles.width*G.selTiles.height;
		if (G.selTiles.size < size);
		{
			G.selTiles.size = size*2;
			if (G.selTiles.tiles)
				delete[] G.selTiles.tiles;
			G.selTiles.tiles = new int[G.selTiles.size];
		}
		i=0;
		for (int y = y1; y < y2; y++)
			for (int x = x1; x < x2; x++, i++)
			{
				int t = lay->tiles[x + y*mapW];
				G.selTiles.tiles[i] = t;
			}
	}

}

static void
delete_map_object (MapObj *obj)
{
	using namespace UndoSys;

	obj->ent.dead = true;
	RemoveMapObjAction *act = new RemoveMapObjAction (obj);
	AddUndo (act);
}

static void
object_btn_cb (GtkWidget *widget)
{
	open_object_window (map_mouse_pos);

}
/*
==================
 create_menu
==================
*/

static void
create_menu ()
{
	GtkWidget *menu;
	GtkWidget *menuitem;

	mw_menu = gtk_menu_new ();
	menu = mw_menu;
	g_object_ref (G_OBJECT (menu));

	menuitem = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	gtk_widget_show (menuitem);

	menuitem = gtk_menu_item_new_with_label ("Object");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate",
			G_CALLBACK (object_btn_cb), NULL);
	gtk_widget_show (menuitem);
}

