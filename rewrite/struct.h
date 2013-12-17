/* This allows one module to refer to another module's structure with importing all the functions. */
#ifndef __SMALLWM_STRUCT__
#define __SMALLMW_STRUCT__
#include "table.h"
#include "x11.h"

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

// The three states a client can be in:
typedef enum {
    C_VISIBLE, // Not iconified
    C_HIDDEN, // Iconified
    C_MOVERESZ, // Currently being moved or resized
} clientstate_t;

// All information related to a particular client
typedef struct {
    // The window manager that owns the client
    smallwm_t *wm;

    // The window occupied by the client
    Window window;
    // The current visibility state of the client
    clientstate_t state;
    // The current location and size of the client
    struct {
        int x, y;
        unsigned int width, height;
    } dimension;

    // Whether or not this client is visible on all desktops
    Bool sticky;
    // The desktop that owns this client
    int desktop;
} client_t;

#endif
