#ifndef __ICONS__
#define __ICONS__

#include "global.h"

#define wlist_t struct wlist_s
struct wlist_s {
	char *title;
	GC gc;
	Window icon;
	Window win;
	wlist_t *next;
};

void initList();
wlist_t* findList(Window);
void hideWindow(Display*, Window);
void unHideWindow(Display*, Window);
void paintIcons(Display *);
void paintIcon(Display *, Window);

#endif
