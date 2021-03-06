#ifndef __TILE_WINDOW_H__
#define __TILE_WINDOW_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>

#include <Camera.h>
#include <EditTypes.h>


//G_BEGIN_DECLS

#define TILE_WINDOW_TYPE		(tile_window_get_type())
#define TILE_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), TILE_WINDOW_TYPE, TileWindow))
#define TILE_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), TILE_WINDOW_TYPE, TileWindowClass))
#define IS_TILE_WINDOW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), TILE_WINDOW_TYPE))
#define IS_TILE_WINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), TILE_WINDOW_TYPE))

struct TileWindow
{
	GtkTable table;
	
	GtkWidget 	*drawing_area,
			*hscrollbar,
			*vscrollbar;
	struct
	{
		int x,y,
		    x_prev,y_prev;
	}mouse;
	Camera camera;
	TileSelection selection;
	bool selecting;

};

struct TileWindowClass
{
	GtkTableClass table_class;
};

GType		tile_window_get_type			();
GtkWidget*	tile_window_new			();

	
#endif //! __TILE_WINDOW_H__

