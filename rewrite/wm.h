#ifndef __SMALLWM_WM__
#define __SMALLWM_WM__

#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include "client.h"
#include "event.h"
#include "table.h"
#include "util.h"
#include "inih/ini.h"

// Handles global window manager state
typedef struct {
    // Information about the main display
    Display *display;
    Window root;
    int width, height;
    int icon_width, icon_height;

    // Information about virtual desktops
    unsigned long long num_desktops, current_desktop;

    // A table of clients and icons (indexed by Window), and the currently focused window
    table_t *clients, *icons;
    Window focused_window;

    // The program to launch on META+LClick
    char *leftclick_launch;

    // Table of event callbacks (indexed by event)
    table_t *events;
} smallwm_t;

smallwm_t init_wm();

#endif
