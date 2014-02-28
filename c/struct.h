/* This allows one module to refer to another module's structure with importing all the functions. */
#ifndef __SMALLWM_STRUCT__
#define __SMALLWM_STRUCT__
#include "table.h"
#include "x11.h"

// Whether or not a window is being moved/resized
typedef enum {
    MR_MOVE,
    MR_RESZ,
    MR_NONE,
} mvresz_state_t;

struct client_s;
struct events_s;

// Handles global window manager state
typedef struct {
    // Information about the main display
    Display *display;
    Window root;
    int screen;
    unsigned int width, height;
    unsigned int icon_width, icon_height;
    unsigned int border_width;

    // Information about virtual desktops
    unsigned long long num_desktops, current_desktop;

    // A table of clients, icons and dialogs (indexed by Window), and the currently focused window
    table_t *clients, *icons, *dialogs;
    Window focused_window;

    // A table of class actions to run when making a window
    table_t *classactions;

    // The event offset to register for XRandR events
    int xrandr_event_offset;

    // The program to launch on META+LClick
    char *leftclick_launch;

    // The event handler that is being used
    struct events_s *events;

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
    unsigned long long desktop;

    // The layer of the window (from 1 to 9, 1 is the lowest and 9 is the highest)
    char layer;
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
    unsigned int pix_width, pix_height;
} icon_t;

// Information about event handlers
typedef struct events_s {
    // The owning WM instance
    smallwm_t *wm;
    // The core callbacks respond to e.g. KeyPress, ButtonPress, etc.
    table_t *event_callbacks;
    // The key combination event callbacks
    table_t *key_callbacks;
} events_t;

// All the actions that a window class can have
typedef enum {
    ACT_END, // This terminates the list of actions inside classaction_t

    ACT_STICK,
    ACT_MAXIMIZE,
    ACT_SETLAYER,
    ACT_SNAPTOP,
    ACT_SNAPBOTTOM,
    ACT_SNAPLEFT,
    ACT_SNAPRIGHT,
} action_t;

// Actions for particular window classes
typedef struct {
    // The action types
    action_t *actions;
    // The layer, if SETLAYER is an action
    unsigned int layer;
} classaction_t;
#endif
