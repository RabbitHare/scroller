#ifndef __LAYER_DIALOG_H__
#define __LAYER_DIALOG_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkliststore.h>

#include <Camera.h>
#include <EditTypes.h>



#define LAYER_DIALOG_TYPE		(layer_dialog_get_type())
#define LAYER_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), LAYER_DIALOG_TYPE, LayerDialog))
#define LAYER_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), LAYER_DIALOG_TYPE, LayerDialogClass))
#define IS_LAYER_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), LAYER_DIALOG_TYPE))
#define IS_LAYER_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), LAYER_DIALOG_TYPE))


struct LayerDialog
{
	GtkTable table;
	GtkListStore *layers;
	
};

struct LayerDialogClass
{
	GtkTableClass table_class;
};

GType		layer_dialog_get_type			();
GtkWidget*	layer_dialog_new			();
	
#endif //! __LAYER_DIALOG_H__

