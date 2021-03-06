#include <Notifiable.h>
#include <gtk/gtk.h>

GType
notifiable_get_type ()
{
	static GType type = 0;
	if (type == 0)
	{
		static const GTypeInfo info =
		{
			sizeof (NotifiableIface),
			NULL, // base_init
			NULL, // base_finalize
			NULL, // class_init
			NULL, // class_finalize
			NULL, // class_data
			0,
			0,
			NULL // instance_init
		};
		type = g_type_register_static (G_TYPE_INTERFACE, "NotifiableIface", &info, (GTypeFlags)0);
	}
	return type;
}

void
notifiable_notify (Notifiable *nf, UiNotify note)
{
	return NOTIFIABLE_GET_IFACE (nf)->notify (nf, note);
}

