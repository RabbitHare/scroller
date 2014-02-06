#include <Dock.h>
#include <Dockable.h>

#include <gtk/gtk.h>

#include <iostream>


#define DOCK_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), DOCK_TYPE, DockPrivate))

struct DockPrivate
{
	GtkOrientation packing_dir;
	gboolean has_run_dispose,
		 destroy_on_empty,
		 is_flippable;
};
	

static void dock_class_init	(DockClass  *klass,
				 gpointer	data);
static void dock_init		(Dock  *self);
static void dock_dispose	(GObject *gobject);

/*** GtkObject Methods ***/
static void dock_realize		(GtkWidget *widget);
static void dock_add			(GtkContainer *container,
					 GtkWidget    *widget);
static void dock_remove			(GtkContainer *container,
					 GtkWidget    *widget);
/*** DnD Methods ***/
static void drag_data_received_cb 	(GtkWidget	  *widget,
					 GdkDragContext   *context,
					 gint 		   x,
					 gint 		   y,
					 GtkSelectionData *data,
					 guint		   target_type,
					 guint 		   time,
					 GtkWidget	   *dest_widget);

static void drag_data_get_cb 		(GtkWidget	  *widget,
					 GdkDragContext   *context,
					 GtkSelectionData *selection_data,
					 guint		   target_type,
					 guint		   time,
					 GtkWidget	   *drag_child);

static gboolean drag_drop_cb		(GtkWidget	*widget,
					 GdkDragContext *context,
					 gint 		 x,
					 gint 		 y,
					 guint 		 time);
static void drag_failed_cb		(GtkWidget 	*widget,
					 GdkDragContext *context,
					 GtkDragResult   result,
					 GtkWidget	*drag_widget);

/*** Dock Methods ***/
static void dock_insert_new	(Dock		*dock,
				 GtkWidget	*child,
				 GtkWidget	*pos);
#define DOCK_GET_TOP_CHILD(dock)	(dock->child)
static void dock_add_top_child		(Dock *dock, GtkWidget *widget);
static void dock_expand			(GtkWidget *addition, GtkWidget *dest);
static void dock_connect_drag_handlers	(GtkWidget *widget, GtkWidget *source, GtkWidget *dest);
static void dock_cleanup		(GtkWidget *parent);
static void dock_attach_dests		(Dock		*dock);

/*** Flip Callback ***/
static void flip_activate_cb (GtkWidget		*widget,
			      Dock		*dock);
/*** Drag Destination Callbacks ***/
static gboolean dest_button_press_cb (GtkWidget		*widget,
				      GdkEventButton	*event,
				      gpointer	         data);

static GObjectClass *dock_parent_class;

static GtkTargetEntry targets[] = {
	{(char*)"DOCKABLE", GTK_TARGET_SAME_APP, 0}};
static int n_targets = G_N_ELEMENTS (targets);

GType
dock_get_type ()
{
	static GType d_type = 0;

	if (!d_type)
	{
		static const GTypeInfo d_info = 
		{
			sizeof (DockClass),
			NULL, // base_init
			NULL, // base_finalize
			(GClassInitFunc)dock_class_init, // class_init
			NULL, // class_finalize
			NULL, // class_data
			sizeof (Dock),
			0,
			(GInstanceInitFunc) dock_init,
		};

		d_type = g_type_register_static (GTK_TYPE_TABLE, "Dock", &d_info, GTypeFlags(0));
	}

	return d_type;
}

/*
================
 dock_new
================
*/
GtkWidget*
dock_new ()
{
	return GTK_WIDGET (g_object_new (DOCK_TYPE, NULL));
}
/*
================
 dock_append
================
*/
void
dock_append (Dock *dock, GtkWidget *widget)
{
	dock_insert_new (dock, widget, dock->dest2);
}
/*
================
 dock_prepend
================
*/
void
dock_prepend (Dock *dock, GtkWidget *widget)
{
	dock_insert_new (dock, widget, dock->dest1);

}
/*
================
 dock_stack
================
*/
void
dock_stack (Dock	*dock,
	    GtkWidget   *widget,
	    GtkWidget 	*pos)
{
	dock_insert_new (dock, widget, pos);
}
/*
================
 dock_set_orientation
================
*/
void
dock_set_orientation (Dock	    *dock,
		      GtkOrientation orient)
{
	DockPrivate *priv = DOCK_GET_PRIVATE (dock);
	if (priv->packing_dir == orient)
		return;
	else
		priv->packing_dir = orient;

	g_object_ref (dock->dest1);
	g_object_ref (dock->dest2);

	gtk_container_remove (GTK_CONTAINER (dock), dock->dest1);
	gtk_container_remove (GTK_CONTAINER (dock), dock->dest2);

	dock_attach_dests (dock);

}
/*
================
 dock_destroy_on_empty
================
*/
void
dock_destroy_on_empty (Dock *dock,
		       gboolean destroy)
{
	DockPrivate *priv = DOCK_GET_PRIVATE (dock);
	priv->destroy_on_empty = destroy;
}
/*
=================
 dock_set_flippable
=================
*/
void
dock_set_flippable (Dock *dock,
		    gboolean flip)
{
	DOCK_GET_PRIVATE (dock)->is_flippable = flip;
}
	
/*
================
 dock_class_init
================
*/
static void
dock_class_init (DockClass	*klass,
		 gpointer	 data)
{

	GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);	
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

	gobject_class->dispose = dock_dispose;

	widget_class->realize = dock_realize;

	container_class->add = dock_add;
	container_class->remove = dock_remove;

	dock_parent_class = (GObjectClass*)g_type_class_peek_parent (klass);

	g_type_class_add_private (klass, sizeof (DockPrivate));
}
/*
================
 dock_init
================
*/
static void
dock_init (Dock *self)
{
	DockPrivate *priv = DOCK_GET_PRIVATE (self);
	priv->packing_dir = GTK_ORIENTATION_VERTICAL;
	priv->has_run_dispose = FALSE;
	priv->destroy_on_empty = TRUE;
	priv->is_flippable = TRUE;
	
	gtk_table_resize (GTK_TABLE (self), 3, 3);
	gtk_table_set_homogeneous (GTK_TABLE (self), FALSE);

	self->child = NULL;
	
	// initiate drag destinations
	self->dest1 = gtk_event_box_new ();
	self->dest2 = gtk_event_box_new ();

	dock_attach_dests (self);

	gtk_event_box_set_visible_window (GTK_EVENT_BOX (self->dest1), FALSE);
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (self->dest2), FALSE);

	gtk_widget_set_size_request (self->dest1, -1, 7);
	gtk_widget_set_size_request (self->dest2, -1, 7);


	g_signal_connect (G_OBJECT (self->dest1), "button_press_event",
				    G_CALLBACK (dest_button_press_cb), NULL);
	g_signal_connect (G_OBJECT (self->dest2), "button_press_event",
				    G_CALLBACK (dest_button_press_cb), NULL);

	gtk_widget_set_events (self->dest1,
			       GDK_BUTTON_PRESS_MASK);
	gtk_widget_set_events (self->dest2,
			       GDK_BUTTON_PRESS_MASK);

	// add frame to each destination
	GtkWidget *frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (self->dest1), frame);

	frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (self->dest2), frame);

	gtk_widget_show_all (self->dest1);
	gtk_widget_show_all (self->dest2);

	gtk_widget_set_no_show_all (self->dest1, TRUE);
	gtk_widget_set_no_show_all (self->dest2, TRUE);



}
/*
================
 dock_dispose
================
*/
static void
dock_dispose (GObject *gobject)
{
	DockPrivate *priv = DOCK_GET_PRIVATE (gobject);

	if (priv->has_run_dispose)
		return;
	priv->has_run_dispose = TRUE;

	if (G_OBJECT_CLASS (dock_parent_class)->dispose)
		(*G_OBJECT_CLASS (dock_parent_class)->dispose) (gobject);
}
		
	

/*
================
 dock_realize
================
*/
static void
dock_realize (GtkWidget *widget)
{
	
	Dock *dock = DOCK (widget);

	gtk_drag_dest_set (dock->dest1,
			   GtkDestDefaults (GTK_DEST_DEFAULT_MOTION |
			   GTK_DEST_DEFAULT_HIGHLIGHT),
			   targets,
			   n_targets,
			   GDK_ACTION_MOVE);
	gtk_drag_dest_set (dock->dest2,
			   GtkDestDefaults (GTK_DEST_DEFAULT_MOTION |
			   GTK_DEST_DEFAULT_HIGHLIGHT),
			   targets,
			   n_targets,
			   GDK_ACTION_MOVE);

	dock_connect_drag_handlers (dock->dest1, NULL, dock->dest1);	
	dock_connect_drag_handlers (dock->dest2, NULL, dock->dest2);	

	if (GTK_WIDGET_CLASS (dock_parent_class)->realize)
		(*GTK_WIDGET_CLASS (dock_parent_class)->realize) (widget);
}
/*
================
 dock_add_top_child
================
*/
static void
dock_add_top_child (Dock *dd, GtkWidget *widget)
{
	dd->child = widget;
	gtk_table_attach_defaults (GTK_TABLE (dd), dd->child, 1, 2, 1, 2);
}
/*
================
 dock_add
================
*/
static void
dock_add (GtkContainer *container,
	  GtkWidget    *widget)
{
	dock_append (DOCK (container), widget);
}
/*
================
 dock_remove
================
*/
static void
dock_remove(GtkContainer *container,
	   GtkWidget	*widget)
{
	Dock *dock = DOCK (container);
	if (dock->child == widget)
		dock->child = NULL;

	if (GTK_CONTAINER_CLASS (dock_parent_class)->remove)
		(*GTK_CONTAINER_CLASS (dock_parent_class)->remove)  (container, widget);
}

/*
================
 drag_data_received_cb
================
*/
static void
drag_data_received_cb (GtkWidget        *widget, 
		       GdkDragContext   *context,
		       gint	         x, 
		       gint 	         y,
		       GtkSelectionData *data,
		       guint		 info,
		       guint 		 time,
		       GtkWidget	 *dest_widget)
{

	GtkWidget *detached_widget;

	detached_widget = *((GtkWidget**)data->data);
	if (data->target && 
	    detached_widget != dest_widget &&
	    IS_DOCKABLE (detached_widget) )
	{
		dock_expand (detached_widget, dest_widget);
		gtk_drag_finish (context, TRUE, FALSE, time);
	}
	else
		gtk_drag_finish (context, FALSE, FALSE, time);

}
/*
================
 drag_data_get_cb
================
*/
static void
drag_data_get_cb (GtkWidget	   *widget,
		  GdkDragContext   *context,
		  GtkSelectionData *data,
		  guint 	    info,
		  guint 	    time,
		  GtkWidget	    *drag_child)
{
	if (data->target == gdk_atom_intern_static_string ("DOCKABLE"))
	{
		gtk_selection_data_set (data,
					data->target,
					8,
					(guchar*)&drag_child,
					sizeof (gpointer));
	}
}
/*
================
 drag_drop_cb
================
*/
static gboolean
drag_drop_cb (GtkWidget      *widget,
	      GdkDragContext *context, 
	      gint	      x,
	      gint	      y,
	      guint	      time)
{

	GdkAtom target;

	target = gtk_drag_dest_find_target (widget, context, NULL);
	if (target)
	{
		gtk_drag_get_data (widget, context, target, time);
		return TRUE;
	}
	return FALSE;
}
/*
================
drag_failed_cb
================
*/
// creates new window with dock to hold drag_widget
static void
drag_failed_cb (GtkWidget *widget,
		GdkDragContext *context,
		GtkDragResult result,
		GtkWidget *drag_widget)
{
	GtkWidget *window,
		  *dock,
		  *parent_dock;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window),
			   drag_widget->allocation.width,
			   drag_widget->allocation.height);
	gtk_window_set_title (GTK_WINDOW (window), gtk_widget_get_name (drag_widget));

	gtk_container_set_reallocate_redraws (GTK_CONTAINER (window), TRUE);	

	dock = dock_new ();	

	parent_dock = gtk_widget_get_ancestor (drag_widget, DOCK_TYPE);
	if (parent_dock &&
	    DOCK_GET_PRIVATE (parent_dock)->packing_dir == 
	    GTK_ORIENTATION_HORIZONTAL)
	{
		dock_set_orientation (DOCK (dock), GTK_ORIENTATION_HORIZONTAL);
	}

	gtk_container_add (GTK_CONTAINER (window), dock);

	dock_expand (drag_widget, DOCK (dock)->dest1);
	
	gtk_widget_show_all (window);
}
/*
================
 dock_insert_new
================
*/
static void
dock_insert_new (Dock      *dock,
	     	 GtkWidget *widget,
	     	 GtkWidget *pos)
{
	GtkWidget *drag_source = NULL,
		  *drag_dest = NULL;

	dock_expand (widget,pos);

	// DnD
	if (IS_DOCKABLE (widget))
	{
		drag_source = dockable_get_dragger (DOCKABLE (widget));
		drag_dest = widget;
	}
	dock_connect_drag_handlers (widget, drag_source, drag_dest);

}
/*
================
 dock_expand
================
*/
static void
dock_expand (GtkWidget *addition,
	     GtkWidget *dest)
{
	GtkWidget *dest_parent,
		  *add_parent;
	Dock *dock = NULL;
	int src_page_n = -1;

	add_parent = gtk_widget_get_parent (addition);
	if (add_parent)
	{
		if (GTK_IS_NOTEBOOK (add_parent))
			src_page_n = gtk_notebook_page_num (GTK_NOTEBOOK (add_parent),
					addition);

		g_object_ref (addition);
		gtk_container_remove (GTK_CONTAINER (add_parent), addition);
	}

	dest_parent = gtk_widget_get_parent (dest);
	if (IS_DOCK (dest_parent))
		dock = DOCK (dest_parent);

	// add addition to a notebook
	if (!dock || (dock->dest1 != dest && dock->dest2 != dest))
	{
		GtkWidget *label,
			  *box;

		// add it to an existing notebook
		if (GTK_IS_NOTEBOOK (dest_parent))
		{
			int page_n = gtk_notebook_page_num (GTK_NOTEBOOK (dest_parent), dest);
			if (page_n == src_page_n)
				page_n++;

			label = gtk_label_new (gtk_widget_get_name (addition));
			box = gtk_event_box_new ();
			gtk_event_box_set_visible_window (GTK_EVENT_BOX (box), FALSE);
			gtk_container_add (GTK_CONTAINER (box), label);
			dock_connect_drag_handlers (addition, box, box);

			gtk_widget_show_all (box);

			gtk_notebook_insert_page (GTK_NOTEBOOK (dest_parent),
						  addition,
						  box,
						  page_n);
		}
		else // create notebook
		{
			GtkWidget *notebook;
			
			notebook = gtk_notebook_new ();

			g_object_ref (dest);
			if (GTK_IS_PANED (dest_parent))
			{
				GtkWidget *child1;
				child1 = gtk_paned_get_child1 (GTK_PANED (dest_parent));
				gtk_container_remove (GTK_CONTAINER (dest_parent), dest);

				if (dest == child1)
					gtk_paned_add1 (GTK_PANED (dest_parent), notebook);
				else 
					gtk_paned_add2 (GTK_PANED (dest_parent), notebook);
			}
			else // is top child
			{
				gtk_container_remove (GTK_CONTAINER (dest_parent), dest);
				dock_add_top_child (DOCK (dest_parent), notebook);
			}


			// Page1
			label = gtk_label_new (gtk_widget_get_name (dest));
			box = gtk_event_box_new ();
			gtk_event_box_set_visible_window (GTK_EVENT_BOX (box), FALSE);
			gtk_container_add (GTK_CONTAINER (box), label);
			dock_connect_drag_handlers (dest, box, box);

			gtk_widget_show_all (box);
			gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
					dest, box);


			// Page2	
			label = gtk_label_new (gtk_widget_get_name (addition));
			box = gtk_event_box_new ();
			gtk_event_box_set_visible_window (GTK_EVENT_BOX (box), FALSE);
			gtk_container_add (GTK_CONTAINER (box), label);
			dock_connect_drag_handlers (addition, box, box);

			gtk_widget_show_all (box);
			gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
					addition, box);

			gtk_widget_show (notebook);
		}
	}
	else // dest is dest1 or dest2
	{
		GtkWidget *dock_child = DOCK_GET_TOP_CHILD (dock);

		if (dock_child)
		{
			GtkWidget *paned;

			if (DOCK_GET_PRIVATE (dock)->packing_dir == GTK_ORIENTATION_VERTICAL)
				paned = gtk_vpaned_new ();
			else
				paned = gtk_hpaned_new ();

			g_object_ref (dock_child);
			gtk_container_remove (GTK_CONTAINER (dock), dock_child);

			if (dest == dock->dest1)
			{
				gtk_paned_add1 (GTK_PANED (paned), addition);
				gtk_paned_add2 (GTK_PANED (paned), dock_child);
			}
			else
			{
				gtk_paned_add2 (GTK_PANED (paned), addition);
				gtk_paned_add1 (GTK_PANED (paned), dock_child);
			}

			gtk_widget_show (paned);

			dock_add_top_child (dock, paned);

			// set divider position
			if (GTK_IS_VPANED (paned))
				gtk_paned_set_position (GTK_PANED (paned), GTK_WIDGET (dock)->allocation.height/2);
			else
				gtk_paned_set_position (GTK_PANED (paned), GTK_WIDGET (dock)->allocation.width/2);

		}
		else // only child
			dock_add_top_child (dock, addition);
	}

	if (add_parent) // addition had parent
	{
		// handle empty space left by removed child
		dock_cleanup (add_parent);
	}

}
/*
================
 dock_connect_drag_handlers
================
*/
static void 
dock_connect_drag_handlers (GtkWidget *widget, GtkWidget *source, GtkWidget *dest)
{
	if (!widget) return;

	if (source)
	{
		gtk_drag_source_set (source,
				GDK_BUTTON1_MASK,
				targets,
				n_targets,
				GDK_ACTION_MOVE);
		// DRAG-DATA-GET
		// disconnect old handlers if any
		g_signal_handlers_disconnect_matched (source,
				G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
				(void*)drag_data_get_cb, NULL);
		g_signal_connect (G_OBJECT (source), "drag_data_get",
				G_CALLBACK (drag_data_get_cb), widget);

		// DRAG-FAILED
		g_signal_handlers_disconnect_matched (source, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
				(void*)drag_failed_cb, NULL);
		g_signal_connect (G_OBJECT (source), "drag_failed",
				G_CALLBACK (drag_failed_cb), widget);

	}
	if (dest)
	{
		gtk_drag_dest_set (dest,
				GtkDestDefaults (GTK_DEST_DEFAULT_MOTION |
					GTK_DEST_DEFAULT_HIGHLIGHT),
				targets,
				n_targets,
				GDK_ACTION_MOVE);
		// DRAG_DATA_RECEIVED
		g_signal_handlers_disconnect_matched (dest, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
				(void*)drag_data_received_cb, NULL);
		g_signal_connect (G_OBJECT (dest), "drag_data_received",
				G_CALLBACK (drag_data_received_cb), widget);	
		// DRAG_DROP
		g_signal_handlers_disconnect_matched (dest, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
				(void*)drag_drop_cb, NULL);
		g_signal_connect (G_OBJECT (dest), "drag_drop",
				G_CALLBACK (drag_drop_cb), NULL);
	}
}
/*
================
 dock_cleanup
================
*/
// cleanup after a child has been dragged away
static void
dock_cleanup (GtkWidget *parent)
{
	GList *children;
	int n_children;
	GtkWidget *child,
		  *grandparent;

	grandparent = gtk_widget_get_parent (parent);

	if (IS_DOCK (parent))
	{
		if (DOCK (parent)->child == NULL && DOCK_GET_PRIVATE (parent)->destroy_on_empty)
		{
			gtk_widget_destroy (parent);
			if (GTK_IS_WINDOW (grandparent))
			{
				gtk_widget_destroy (grandparent);
			}
		}
		return;
	}

	children = gtk_container_get_children (GTK_CONTAINER (parent));
	
	if (children)
	{
		n_children = g_list_length (children);
		child = GTK_WIDGET (children->data);

		g_list_free (children);
	}
	else return;

	if (n_children > 1)
		return;
	
	
	g_object_ref (G_OBJECT (child));
	gtk_container_remove (GTK_CONTAINER (parent), child);

	if (GTK_IS_PANED (grandparent))
	{
		if (parent == gtk_paned_get_child1 (GTK_PANED (grandparent)))
		{
			gtk_container_remove (GTK_CONTAINER (grandparent), parent);
			gtk_paned_add1 (GTK_PANED (grandparent), child);
		}
		else
		{
			gtk_container_remove (GTK_CONTAINER (grandparent), parent);
			gtk_paned_add2 (GTK_PANED (grandparent), child);
		}
	}
	else // grandparent is a dock
	{
		gtk_container_remove (GTK_CONTAINER (grandparent), parent);
		dock_add_top_child (DOCK (grandparent), child);
	}
}
/*
================
dock_attach_dests
================
*/
static void
dock_attach_dests (Dock		*dock)
{

	DockPrivate *priv = DOCK_GET_PRIVATE (dock);

	if (priv->packing_dir == GTK_ORIENTATION_VERTICAL)
	{
		gtk_widget_set_size_request (dock->dest1, -1, 7);
		gtk_widget_set_size_request (dock->dest2, -1, 7);

		gtk_table_attach (GTK_TABLE (dock), dock->dest1, 0, 3, 0, 1,
				(GtkAttachOptions)(GTK_EXPAND|GTK_FILL),
				(GtkAttachOptions)0,
				 0, 0);
		gtk_table_attach (GTK_TABLE (dock), dock->dest2, 0, 3, 2, 3,
				(GtkAttachOptions)(GTK_EXPAND|GTK_FILL),
				(GtkAttachOptions)0,
				 0, 0);
	}
	else
	{
		gtk_widget_set_size_request (dock->dest1, 7, -1);
		gtk_widget_set_size_request (dock->dest2, 7, -1);

		gtk_table_attach (GTK_TABLE (dock), dock->dest1, 0, 1, 0, 3,
				 (GtkAttachOptions)0,
				 (GtkAttachOptions)(GTK_EXPAND|GTK_FILL),
				 0, 0);
		gtk_table_attach (GTK_TABLE (dock), dock->dest2, 2, 3, 0, 3,
				 (GtkAttachOptions)0,
				 (GtkAttachOptions)(GTK_EXPAND|GTK_FILL),
				 0, 0);
	}
}

/*
================
 flip_activate_cb
================
*/
static void
flip_activate_cb (GtkWidget *widget,
		  Dock	    *dock)
{
	DockPrivate *priv = DOCK_GET_PRIVATE (dock);
	if (priv->packing_dir == GTK_ORIENTATION_VERTICAL)
		priv->packing_dir = GTK_ORIENTATION_HORIZONTAL;
	else 
		priv->packing_dir = GTK_ORIENTATION_VERTICAL;
	g_object_ref (dock->dest1);
	g_object_ref (dock->dest2);

	gtk_container_remove (GTK_CONTAINER (dock), dock->dest1);
	gtk_container_remove (GTK_CONTAINER (dock), dock->dest2);

	dock_attach_dests (dock);
}
static void
menu_selection_done_cb (GtkWidget *widget,
		    	gpointer   data)
{
	gtk_widget_destroy (widget);
}
/*
================
 dest_button_press_cb
================
*/
static gboolean
dest_button_press_cb (GtkWidget		*widget,
		      GdkEventButton	*event,
		      gpointer		 data)
{
	if (event->button == 3 && DOCK_GET_PRIVATE (widget->parent)->is_flippable)
	{
		GtkWidget *menu_item,
			  *menu;

		menu = gtk_menu_new ();

		menu_item = gtk_menu_item_new_with_label ("Flip");

		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
		
		g_signal_connect (G_OBJECT (menu_item), "activate",
				  G_CALLBACK (flip_activate_cb), widget->parent);

		g_signal_connect (G_OBJECT (menu), "selection_done",
				  G_CALLBACK (menu_selection_done_cb), NULL);

		gtk_widget_show_all (menu);
		gtk_menu_popup (GTK_MENU (menu),
				NULL,NULL,NULL,NULL,
				3, event->time);
		return TRUE;
	}
	return FALSE;
}

