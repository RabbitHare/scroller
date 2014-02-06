#ifndef __MAP_WINDOW_H__
#define __MAP_WINDOW_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>

#include <Camera.h>
#include <EditTypes.h>



#define MAP_WINDOW_TYPE		(map_window_get_type())
#define MAP_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), MAP_WINDOW_TYPE, MapWindow))
#define MAP_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), MAP_WINDOW_TYPE, MapWindowClass))
#define IS_MAP_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAP_WINDOW_TYPE))
#define IS_MAP_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), MAP_WINDOW_TYPE))


enum EdState
{
	ED_NONE = 0,
	ED_WRITING,
	ED_SELECTING
};
struct MapWindow
{
	GtkTable table;
	
	GtkWidget 	*drawing_area,
			*vscrollbar,
			*hscrollbar;
	struct
	{
		int x,y,
		    x_prev,y_prev;
	}mouse;
	Camera camera;
	EdState state;
	TileSelection selection;
	bool show_mask;

};

struct MapWindowClass
{
	GtkTableClass table_class;
};

GType		map_window_get_type			();
GtkWidget*	map_window_new			();

	
#endif //! __MAP_WINDOW_H__

