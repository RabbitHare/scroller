#include <Dockable.h>
#include <gtk/gtk.h>

GType
dockable_get_type ()
{
	static GType type = 0;
	if (type == 0)
	{
		static const GTypeInfo info =
		{
			sizeof (DockableIface),
			NULL, // base_init
			NULL, // base_finalize
			NULL, // class_init
			NULL, // class_finalize
			NULL, // class_data
			0,
			0,
			NULL // instance_init
		};
		type = g_type_register_static (G_TYPE_INTERFACE, "DockableIface", &info, (GTypeFlags)0);
	}
	return type;
}

GtkWidget*
dockable_get_dragger (Dockable *self)
{
	return DOCKABLE_GET_IFACE (self)->get_dragger (self);
}

