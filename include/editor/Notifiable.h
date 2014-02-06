#ifndef __NOTIFIABLE_H__
#define __NOTIFIABLE_H__
#include <glib-object.h>
#include <gtk/gtkwidget.h>
#include <Ui.h>

#define NOTIFIABLE_TYPE			(notifiable_get_type ())
#define NOTIFIABLE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), NOTIFIABLE_TYPE, Notifiable))
#define IS_NOTIFIABLE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), NOTIFIABLE_TYPE))
#define NOTIFIABLE_GET_IFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), NOTIFIABLE_TYPE, NotifiableIface))

struct Notifiable; // Dummy

struct NotifiableIface
{
	GTypeInterface g_iface;

	// virtual table
	void (*notify) (Notifiable *nf, UiNotify note);
};

GType notifiable_get_type	();

void notifiable_notify (Notifiable *nf, UiNotify note);

#endif // !__NOTIFIABLE_H__

