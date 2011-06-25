#ifndef __EVENT__
#define __EVENT__

#include "global.h"
#include "client.h"

typedef struct
{
    KeySym ksym;
    void (*callback) (XEvent, client_t*);
} uevent_t;

// Used to define a keyboard shortcut callback
#define UCALLBACK(name) static void name(XEvent ev, client_t* cli)

// /////////////////////////////////////////////
// Here are the keyboard shortcuts - their functions
// are implemented in the header as to be easy to edit.
// Their static definition doesn't hurt anything here,
// and this works well enough in practice. (Though it
// is bad form).
//
UCALLBACK (RaiseWindow)
{
	if (cli->state != Active) return;	
    XRaiseWindow (dpy, cli->win);
}

UCALLBACK (LowerWindow)
{
	if (cli->state != Active) return;
    XLowerWindow (dpy, cli->win);
}

UCALLBACK (Maximize)
{
	if (cli->state != Active) return;
    XMoveResizeWindow (dpy, cli->win, 0, ICON_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - ICON_HEIGHT);	
}

UCALLBACK (Close)
{
	destroy(cli, 0);
}

UCALLBACK (Hide)
{
    hide(cli);
}

#define NSHORTCUTS 5
static uevent_t SHORTCUTS[NSHORTCUTS] = {
    {XK_Page_Up, RaiseWindow},
    {XK_Page_Down, LowerWindow},
    {XK_m, Maximize},
    {XK_c, Close},
    {XK_h, Hide},
};

// Used for event loop callback (ie from X)
#define CALLBACK(name) void name(Display*, XEvent)
CALLBACK (eKeyPress);
CALLBACK (eButtonPress);
CALLBACK (eButtonRelease);
CALLBACK (eMotionNotify);
CALLBACK (eMapNotify);

#endif
