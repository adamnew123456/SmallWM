#ifndef __ICONS__
#define __ICONS__

#include "global.h"

#define wlist_t struct wlist_s
struct wlist_s {
	Window win;
	char *title;
	wlist_t *next;
};

#define MWIDTH 75
#define MHEIGHT 20

void initList();
void hideWindow(Display*, Window);
void unHideWindow(Display*, Window);
void showMenu(Display*);

#endif
