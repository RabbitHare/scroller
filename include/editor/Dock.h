#ifndef __DOCK_H__
#define __DOCK_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>

#define DOCK_TYPE		(dock_get_type())
#define DOCK(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), DOCK_TYPE, Dock))
#define DOCK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), DOCK_TYPE, DockClass))
#define IS_DOCK(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), DOCK_TYPE))
#define IS_DOCK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), DOCK_TYPE))

struct Dock
{
	GtkTable table;

	GtkWidget *box,
		  *dest1, // drag destination 1
		  *dest2, // drag destination 2
		  *child;
};

struct DockClass
{
	GtkTableClass table_class;
};
GType		dock_get_type		();
GtkWidget*	dock_new		();
void		dock_append		(Dock *dock, GtkWidget *child);
void		dock_prepend		(Dock *dock, GtkWidget *child);
void		dock_stack		(Dock *dock, GtkWidget *child,
						   GtkWidget *pos);
void		dock_set_orientation	(Dock *dock, GtkOrientation orient);
// destroy when child has been dragged away
void 		dock_destroy_on_empty (Dock *dock, gboolean destroy);
void		dock_set_flippable    (Dock *dock, gboolean flip);

#endif // !__DOCK_H__
