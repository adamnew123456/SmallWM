#ifndef __EVENT__
#define __EVENT__

#include "global.h"
#include "client.h"

typedef struct
{
    KeySym ksym;
    void (*callback) (XEvent, client_t*);
} uevent_t;

#define GET_CLIENT(name) client_t *name = fromevent(ev)

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
	raise(cli);
}

UCALLBACK (LowerWindow)
{
	lower(cli);
}

UCALLBACK (Maximize)
{
	maximize(cli);
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
