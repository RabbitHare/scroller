#include <TileWindow.h>
#include <Dockable.h>
#include <Notifiable.h>

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>

#include <GL/gl.h>
#include <iostream>

#include <Tileset.h>

#include <Map.h>
#include <Renderer.h>

#include <Global.h>

#define TILE_WIN_FOV 45.0

#define TILE_WINDOW_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), TILE_WINDOW_TYPE, TileWindowPrivate))
struct TileWindowPrivate
{
	gboolean dispose_has_run;
	GtkWidget *dragger;
};
static GObjectClass *parent_class=NULL;

static void tile_window_class_init (TileWindowClass *klass);
static void tile_window_init (TileWindow *self);
static void tile_window_dispose (GObject *obj);

static void dockable_iface_init (gpointer g_iface, gpointer iface_init);
static GtkWidget* tile_window_get_dragger (Dockable *dock);

static void notifiable_iface_init (gpointer g_iface, gpointer iface_init);
static void tile_window_notify (Notifiable *nf, UiNotify note);

static void realize_cb			(GtkWidget *widget, TileWindow *tw );
static gboolean configure_cb		(GtkWidget *widget, GdkEventConfigure *event, TileWindow *tw);
static gboolean expose_cb		(GtkWidget *widget, GdkEventExpose *event, TileWindow *tw);
static gboolean enter_cb		(GtkWidget *widget, GdkEvent *event, TileWindow *tw);
static gboolean key_press_cb		(GtkWidget *widget, GdkEventKey *key, 	    TileWindow *tw);
static gboolean button_press_cb		(GtkWidget *widget, GdkEventButton *button, TileWindow *tw);
static gboolean button_release_cb		(GtkWidget *widget, GdkEventButton *button, TileWindow *tw);
static gboolean scroll_cb		(GtkWidget *widget, GdkEventScroll *scroll, TileWindow *tw);
static gboolean motion_cb		(GtkWidget *widget, GdkEventMotion *motion, TileWindow *tw);

static void scrollbar_value_changed_cb (GtkWidget *widget, TileWindow *tw);
static void update_scrollbars (TileWindow *tw);
static void redraw (GtkWidget *widget);

static void set_projection (TileWindow *tw);
static Vec2f get_mouse_tile_position (TileWindow *tw);
static void draw (TileWindow *tw);
static void draw_tiles (TileWindow *tw);
static void read_tiles (TileWindow *tw);


GType
tile_window_get_type (void)
{
	static GType tw_type = 0;

	if (!tw_type)
	{
		static const GTypeInfo tw_info =
		{
			sizeof (TileWindowClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)tile_window_class_init, /* class_init */
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (TileWindow),
			0,
			(GInstanceInitFunc) tile_window_init,
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

		tw_type = g_type_register_static (GTK_TYPE_TABLE, "TileWindow", &tw_info, GTypeFlags(0));

		g_type_add_interface_static (tw_type, DOCKABLE_TYPE, &dock_info);
		g_type_add_interface_static (tw_type, NOTIFIABLE_TYPE, &note_info);
	}

	return tw_type;
}

GtkWidget*
tile_window_new ()
{
	return GTK_WIDGET (g_object_new (TILE_WINDOW_TYPE, NULL));
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
tile_window_class_init (TileWindowClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->dispose = tile_window_dispose;

	parent_class = (GObjectClass*)g_type_class_peek_parent (klass);

	g_type_class_add_private (gobject_class, sizeof (TileWindowPrivate));
}
/*
============================
 tile_window_init
============================
*/
static void
tile_window_init (TileWindow *self)
{
	TileWindowPrivate *priv = TILE_WINDOW_GET_PRIVATE (self);
	priv->dispose_has_run = FALSE;

	Ui::AddNotifiable (NOTIFIABLE (self));

	gtk_widget_set_name (GTK_WIDGET (self), "Tiles");
	gtk_container_set_border_width (GTK_CONTAINER (self), 1);
	gtk_table_set_homogeneous (GTK_TABLE (self), FALSE);
	gtk_table_resize (GTK_TABLE (self), 3,2);

	// manually call constructors
	new (&self->camera)	Camera ();
	new (&self->selection) TileSelection ();
	self->selecting = false;
	self->camera.mat.SetTranslation(Vec3f(0,0,600.0));

	self->mouse.x = 0;
	self->mouse.y = 0;
	self->mouse.x_prev = 0;
	self->mouse.y_prev = 0;

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

	// DRAGGER
	GtkWidget *label;
	priv->dragger = gtk_event_box_new ();
	gtk_table_attach (GTK_TABLE (self), priv->dragger,
			0,1,0,1, (GtkAttachOptions)(GTK_FILL), (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_signal_connect_after (G_OBJECT (priv->dragger), "realize",
				G_CALLBACK (dragger_realize_cb), NULL);
	label = gtk_label_new (gtk_widget_get_name (GTK_WIDGET (self)));
	gtk_container_add (GTK_CONTAINER (priv->dragger), label);
	gtk_widget_show_all (priv->dragger);
}
static void
tile_window_dispose (GObject* obj)
{
	TileWindow *self = TILE_WINDOW (obj);
	TileWindowPrivate *priv = TILE_WINDOW_GET_PRIVATE (self);

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
	iface->get_dragger = tile_window_get_dragger;
}
/*
================
 tile_window_get_dragger
================
*/
static GtkWidget*
tile_window_get_dragger (Dockable *dock)
{
	TileWindowPrivate *priv = TILE_WINDOW_GET_PRIVATE (dock);
	return priv->dragger;
}
/*
================
 notifiable_iface_init
================
*/
static void
notifiable_iface_init (gpointer g_iface, gpointer iface_init)
{
	NotifiableIface *iface = (NotifiableIface*)g_iface;
	iface->notify = tile_window_notify;
}
static void
tile_window_notify (Notifiable *nf, UiNotify note)
{
	TileWindow *tw = TILE_WINDOW (nf);
	switch (note)
	{
		case UI_NEW_ACTIVE_LAYER:
			update_scrollbars (tw);
			redraw (tw->drawing_area);
			break;
		default:
			break;
	}	
}

static void
realize_cb (GtkWidget		*widget,
	    TileWindow	*tw  )
{
	int w = widget->allocation.width,
	    h = widget->allocation.height;
	
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	//opengl begin
	if (!gdk_gl_drawable_gl_begin (GDK_GL_DRAWABLE (gldrawable), glcontext))
		return;
	

	set_projection (tw);	
	glClearDepth (1.0);
	glClearColor (G.bgColor.x*.5,G.bgColor.y*.5,G.bgColor.z*.5,1.0);

	glEnable (GL_COLOR_MATERIAL);
	glEnable (GL_CULL_FACE);

	glViewport (0, 0, w, h);

	glEnable(GL_BLEND); glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gdk_gl_drawable_gl_end (GDK_GL_DRAWABLE (gldrawable));
}

static gboolean
configure_cb (GtkWidget		*widget,
	      GdkEventConfigure	*event,
	      TileWindow	*tw )
{
	int w = widget->allocation.width,
	    h = widget->allocation.height;


	GdkGLContext  *glcontext  = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	// opengl begin
	if (!gdk_gl_drawable_gl_begin (GDK_GL_DRAWABLE (gldrawable), glcontext))
		return FALSE;

	set_projection (tw);
	scrollbar_value_changed_cb (tw->hscrollbar,tw);
	scrollbar_value_changed_cb (tw->vscrollbar,tw);

	glViewport (0, 0, w, h);

	// scrollbars
	gtk_range_set_increments (GTK_RANGE (tw->hscrollbar), 1.0,w);

	gtk_range_set_increments (GTK_RANGE (tw->vscrollbar), 1.0,h);

	// opengl end
	gdk_gl_drawable_gl_end (GDK_GL_DRAWABLE (gldrawable));

	return TRUE;
}	

static gboolean
expose_cb (GtkWidget		*widget,
	   GdkEventExpose	*event,
	   TileWindow	*tw)
{
	GdkGLContext  *glcontext  = gtk_widget_get_gl_context (widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

	//opengl begin
	if (!gdk_gl_drawable_gl_begin (GDK_GL_DRAWABLE (gldrawable), glcontext))
		return FALSE;
	
	//______DRAW______
	//
	draw (tw);

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
	TileWindow 		*tw)
{
	gtk_widget_grab_focus (widget);
	return TRUE;
}

static gboolean
key_press_cb (GtkWidget			*widget,
	      GdkEventKey		*key,
	      TileWindow		*tw)
{
	/*
	switch (key->keyval)
	{
		case GDK_g:
		case GDK_n:
		case GDK_r:
		case GDK_z:
			if (key->state & GDK_CONTROL_MASK)
			{
			}
			break;
		case GDK_y:
			break;
		case GDK_F1:
			break;
		default:
			break;
	}
	*/
	return TRUE;
}
static gboolean
button_press_cb (GtkWidget		*widget,
		 GdkEventButton		*event,
		 TileWindow		*tw )
{
	
	switch (event->button)
	{
		case 1:
			if (event->type == GDK_BUTTON_PRESS)
			{
				Tileset &tset = *GetTileset(G.selLayer->tileset);
				tw->selection.Clear();
				Vec2f mpos = get_mouse_tile_position (tw);
				tw->selection.x = mpos.x;
				tw->selection.y = mpos.y;

				if (tw->selection.x > tset.width)
					tw->selection.x = tset.width;
				if (tw->selection.y > tset.height)
					tw->selection.y = tset.height;

				tw->selecting = true;
				read_tiles (tw);
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
		 TileWindow		*tw )
{
	
	switch (event->button)
	{
		case 1:
			if (event->type == GDK_BUTTON_RELEASE)
			{
				if (tw->selecting)
				{
					tw->selecting = false;
					read_tiles (tw);

				}
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
	   TileWindow	*tw)
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
	   TileWindow	*tw )

{

	tw->mouse.x_prev = tw->mouse.x;
	tw->mouse.y_prev = tw->mouse.y;
	tw->mouse.x = (int)motion->x;
	tw->mouse.y = (int)motion->y;

	//int distx = tw->mouse.x - tw->mouse.x_prev;
	//int disty = tw->mouse.y - tw->mouse.y_prev;

	
	if (motion->state & GDK_BUTTON1_MASK)
	{
		if (tw->selecting == true)
		{
			Vec2f mpos = get_mouse_tile_position (tw);
			int x = mpos.x,
			    y = mpos.y;
			tw->selection.offsetX = tw->selection.x - x;
			tw->selection.offsetY = tw->selection.y - y;
			if (tw->selection.offsetX < 0)
				tw->selection.offsetX = -1;
			else tw->selection.offsetX = 1;
			if (tw->selection.offsetY < 0)
				tw->selection.offsetY = -1;
			else tw->selection.offsetY = 1;
			tw->selection.width = abs (tw->selection.x - x)+1; 
			tw->selection.height = abs (tw->selection.y - y)+1;

			read_tiles (tw);

			redraw (widget);
			return TRUE;
		}
	}

	return TRUE;
}
/*
====================
 scrollbar callbacks
====================
*/
static void
scrollbar_value_changed_cb (GtkWidget *widget,
			    TileWindow *tw)
{

	float value = gtk_range_get_value (GTK_RANGE (widget));
	Vec3f pos = tw->camera.mat.GetTranslation ();
	Vec2f s = tw->camera.GetDrawScale (0.);
	s.x = 1.0/s.x;
	s.y = 1.0/s.y;
	if (widget == tw->hscrollbar)
	{
		pos.x = value+s.x;
	}
	else pos.y = -value-s.y;
	tw->camera.mat.SetTranslation (pos);
	redraw (tw->drawing_area);
}
static void
update_scrollbars (TileWindow *tw)
{
	if (G.selLayer == NULL) return;
	Tileset *tset = GetTileset (G.selLayer->tileset);

	Vec2f ds = tw->camera.GetDrawScale (0);
	float rx=0.,ry=0.;
	
	if (ds.x > 0.)
		rx = tset->width - 2.0/(tset->tileW*ds.x);
	if (ds.y > 0.)
		ry = tset->height - 2.0/(tset->tileH*ds.y);
	rx *= tset->tileW;
	ry *= tset->tileH;
	if (rx < 0.001) rx = 0.001;
	if (ry < 0.001) ry = 0.001;
	gtk_range_set_range (GTK_RANGE (tw->hscrollbar), 0,rx);
	gtk_range_set_range (GTK_RANGE (tw->vscrollbar), 0,ry);

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

static void set_projection (TileWindow *tw)
{
	int w = tw->drawing_area->allocation.width,
	    h = tw->drawing_area->allocation.height;

	float fw = (float)w,
	      fh = (float)h;
	float min = std::min (fw,fh),
	      max = std::max (fw,fh);
	tw->camera.SetProjection (w,h,TILE_WIN_FOV*min/max);
	float cotan = tw->camera.projMat.M[0][0];
	tw->camera.mat.SetTranslation(Vec3f(fw*0.5,-fh*0.5,cotan*w*0.5));
	update_scrollbars (tw);
}
/*
========================
 GET_MOUSE_TILE_POSITION
========================
*/
static Vec2f get_mouse_tile_position (TileWindow *tw)
{
	Vec2f ret(0.,0.);
	Layer *lay = G.selLayer;
	if (lay == NULL)
		return ret;
	Tileset *tset = GetTileset (lay->tileset);
	Vec3f cpos = tw->camera.mat.GetTranslation();
	float sw = tw->drawing_area->allocation.width,
	      sh = tw->drawing_area->allocation.height;
	Vec2f ds = tw->camera.GetDrawScale (0.);
	ret.x = ((float)tw->mouse.x*2.0/sw-1.0 + cpos.x*ds.x)/(tset->tileW*ds.x);
	ret.y = ((float)tw->mouse.y*2.0/sh-1.0 - cpos.y*ds.y)/(tset->tileH*ds.y);
	ret.x = float((int)ret.x);
	ret.y = float((int)ret.y);
	return ret;
}
/*
=================
 DRAW
=================
*/
static void draw (TileWindow *tw)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (G.selLayer == NULL) return;
	Tileset &tset = *GetTileset (G.selLayer->tileset);

	float tileW = tset.tileW,
	      tileH = tset.tileH;
	float w = (float)tset.width * tileW,
	      h = (float)tset.height * tileH;
	int tsetW = tset.width,
	    tsetH = tset.height;
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();

	Vec3f camtran = -tw->camera.mat.GetTranslation();
	glTranslatef (camtran.x,camtran.y,camtran.z);
	// draw quad the size of the tileset
	glColor3fv (G.bgColor.V);
	glBegin (GL_QUADS);
		glVertex2f (0.0,0.0);
		glVertex2f (0.0,-h);
		glVertex2f (w,-h);
		glVertex2f (w,0.0);
	glEnd ();

	glEnable (GL_DEPTH_TEST);

	draw_tiles (tw);


	glColor3f (1.0,1.0,1.0);
	if (tw->selecting)
	{
		glBlendFunc (GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		Vec2f m = get_mouse_tile_position (tw);
		float x1,x2,
		    y1,y2,
		    z1 = tw->selection.maxDepth+0.1,
		    z2 = tw->selection.minDepth;
		if (m.x < tw->selection.x)
		{
			x1 = m.x; x2 = tw->selection.x;
		}
		else{ x2 = m.x; x1 = tw->selection.x; }
		if (m.y < tw->selection.y)
		{
			y1 = m.y; y2 = tw->selection.y;
		}
		else{ y2 = m.y; y1 = tw->selection.y; }
		
		x2+=1.0;
		y2+=1.0;	
		x1 = std::max (x1,0.0f);
		x2 = std::min (x2,(float)tsetW);
		y1 = std::max (y1,0.0f);
		y2 = std::min (y2,(float)tsetH);
		x1 *= tileW;
		x2 *= tileW;
		y1 *= -tileH;
		y2 *= -tileH;

		x1 -= 0.01;
		x2 += 0.01;
		y1 += 0.01;
		y2 -= 0.01;
		glBegin (GL_QUADS);
	
		// front	
		glVertex3f (x1,y1,z1);
		glVertex3f (x1,y2,z1);
		glVertex3f (x2,y2,z1);
		glVertex3f (x2,y1,z1);
		
		// bottom
		glVertex3f (x1,y2,z1);
		glVertex3f (x1,y2,z2);
		glVertex3f (x2,y2,z2);
		glVertex3f (x2,y2,z1);

		// top
		glVertex3f (x1,y1,z1);
		glVertex3f (x2,y1,z1);
		glVertex3f (x2,y1,z2);
		glVertex3f (x1,y1,z2);

		// left
		glVertex3f (x1,y1,z1);
		glVertex3f (x1,y1,z2);
		glVertex3f (x1,y2,z2);
		glVertex3f (x1,y2,z1);

		// right
		glVertex3f (x2,y1,z1);
		glVertex3f (x2,y2,z1);
		glVertex3f (x2,y2,z2);
		glVertex3f (x2,y1,z2);
		glEnd ();
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glDisable (GL_DEPTH_TEST);
}
static void
draw_tiles (TileWindow *tw)
{
	if (G.selLayer == NULL) return;
	Tileset &tset = *GetTileset (G.selLayer->tileset);

	float tileW = tset.tileW,
	      tileH = tset.tileH;
	int w = tset.width,
	    h = tset.height;
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, tset.texture);

	glColor3f (1.0,1.0,1.0);	
	
	int tiles[w*h];
	for (int i=0; i < w*h; i++)
		tiles[i] = i+1;
	Renderer *ren = GetRenderer ();
	ClipRecti clip(0,0,w,h);
	ren->SetRenderPass (R_OPAQUE);
	ren->DrawTiles (tiles, w, h, tileW, tileH, clip);
	ren->SetRenderPass (R_ALPHA);
	ren->DrawTiles (tiles, w, h, tileW, tileH, clip);
	ren->SetRenderPass (R_ALPHA_FRONT);
	ren->DrawTiles (tiles, w, h, tileW, tileH, clip);

	glDisable (GL_TEXTURE_2D);
}
static void
read_tiles (TileWindow *tw)
{
	if (G.selLayer == NULL) return;
	Tileset &tileset = *GetTileset (G.selLayer->tileset);

	Vec2f mpos = get_mouse_tile_position(tw);
	int mx = mpos.x,
	    my = mpos.y;
	int x1,x2,
	    y1,y2;
	int tsetW = tileset.width,
	    tsetH = tileset.height;
	if (mx < tw->selection.x)
	{
		x1 = mx; x2 = tw->selection.x;
	}
	else{ x2 = mx; x1 = tw->selection.x; }
	if (my < tw->selection.y)
	{
		y1 = my; y2 = tw->selection.y;
	}
	else{ y2 = my; y1 = tw->selection.y; }

	x2++;
	y2++;	
	x1 = std::max (x1,0);
	x2 = std::min (x2,tsetW);
	y1 = std::max (y1,0);
	y2 = std::min (y2,tsetH);
	tw->selection.width = x2-x1;
	tw->selection.height = y2-y1;
	int i=0;
	float min=0,max=0;
	for (int y = y1; y < y2; y++)
		for (int x = x1; x < x2; x++,i++)
		{
			int t = x + y*tsetW + 1;
			if (tileset.tiles[t].minDepth < min)
				min = tileset.tiles[t].minDepth;	
			if (tileset.tiles[t].maxDepth > max)
				max = tileset.tiles[t].maxDepth;
		}
	tw->selection.minDepth = min;
	tw->selection.maxDepth = max;
	if (tw->selecting == false)
	{
		// copy to global
		G.selTiles = tw->selection;

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
				int t = x + y*tsetW + 1;
				G.selTiles.tiles[i] = t;
			}
	}

}

