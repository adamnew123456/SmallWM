/* This allows one module to refer to another module's structure with importing all the functions. */
#ifndef __SMALLWM_STRUCT__
#define __SMALLMW_STRUCT__
#include "table.h"
#include "x11.h"

// Whether or not a window is being moved/resized
typedef enum {
    MR_MOVE,
    MR_RESZ,
    MR_NONE,
} mvresz_state_t;

struct client_s;

// Handles global window manager state
typedef struct {
    // Information about the main display
    Display *display;
    Window root;
    int screen;
    int width, height;
    int icon_width, icon_height;

    // Information about virtual desktops
    unsigned long long num_desktops, current_desktop;

    // A table of clients and icons (indexed by Window), and the currently focused window
    table_t *clients, *icons;
    Window focused_window;

    // The event offset to register for XRandR events
    int xrandr_event_offset;

    // The program to launch on META+LClick
    char *leftclick_launch;

    // The state of the currently moving/resizing window
    struct {
        mvresz_state_t state;
        XButtonEvent event;
        struct client_s *client;
    }  movement;
} smallwm_t;

// The three states a client can be in:
typedef enum {
    C_VISIBLE, // Not iconified
    C_HIDDEN, // Iconified
    C_MOVERESZ, // Currently being moved or resized
} clientstate_t;

// All information related to a particular client
typedef struct client_s {
    // The window manager that owns the client
    smallwm_t *wm;

    // The window occupied by the client
    Window window;
    // The current visibility state of the client
    clientstate_t state;
    // The placeholder window, in case this client is being moved or resized
    Window mvresz_placeholder;

    // Whether or not this client is visible on all desktops
    Bool sticky;
    // The desktop that owns this client
    int desktop;
} client_t;

// All information about an icon
typedef struct {
    // The window manager that owns the icon
    smallwm_t *wm;
    // The client which is launched by clicking this icon
    client_t *client;

    // The window used by the icon
    Window window;
    // The graphics context used to draw the text and pixmap
    GC gc;

    // Whether or not this icon has a pixmap assigned to it
    Bool has_pixmap;
    // The Pixmap used as an icon
    Pixmap pixmap;
    // The dimensions of the pixmap
    unsigned int width, height;
} icon_t;

// Information about event handlers
typedef struct {
    // The owning WM instance
    smallwm_t *wm;
    // The core callbacks respond to e.g. KeyPress, ButtonPress, etc.
    table_t *event_callbacks;
    // The key combination event callbacks
    table_t *key_callbacks;
} events_t;
#endif
