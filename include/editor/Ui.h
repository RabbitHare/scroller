#ifndef __UI_H__
#define __UI_H__

#include <gtk/gtk.h>

struct Notifiable;

enum UiNotify
{
	UI_NEW_MAP,
	UI_NEW_ACTIVE_LAYER,
	UI_LEVEL_EDIT
};
namespace Ui
{
void InitUi ();
void AddNotifiable (Notifiable *nf);
void RemoveNotifiable (Notifiable *nf);

void Notify (UiNotify note);
void UpdateWindowTitle ();

};
#endif

