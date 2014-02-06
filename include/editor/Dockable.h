#ifndef __DOCKABLE_H__
#define __DOCKABLE_H__
#include <glib-object.h>
#include <gtk/gtkwidget.h>

#define DOCKABLE_TYPE			(dockable_get_type ())
#define DOCKABLE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCKABLE_TYPE, Dockable))
#define IS_DOCKABLE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCKABLE_TYPE))
#define DOCKABLE_GET_IFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), DOCKABLE_TYPE, DockableIface))

struct Dockable; // Dummy

struct DockableIface
{
	GTypeInterface g_iface;

	// virtual table
	GtkWidget* (*get_dragger) (Dockable *self);
};

GType dockable_get_type	();

GtkWidget* dockable_get_dragger (Dockable *self);

#endif // !__DOCKABLE_H__

