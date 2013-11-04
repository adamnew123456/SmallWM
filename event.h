#ifndef __EVENT__
#define __EVENT__

#include "global.h"
#include "client.h"

// A key-value mapping to shrink code
typedef struct {
	KeySym ksym;
	void (*callback) (XEvent *, client_t *);
} uevent_t;

typedef struct {
	XButtonEvent mouse;
	int inmove, inresz;
	client_t *client;
} moving_t;

// Used to define a keyboard shortcut callback
#define UCALLBACK(name) static void name(XEvent* ev, client_t* cli)

// Resolve the event pointer into an actual struct
#define GET_EVENT XEvent ev = *evp

// ////////////////////////////////////////////////////
// Here are the keyboard shortcuts - their functions
// are implemented in the header as to be easy to edit.
// Their static definition doesn't hurt anything here,
// and this works well enough in practice. (Though it
// is bad form).
//
UCALLBACK(RaiseWindow)
{
	raise_(cli);
}

UCALLBACK(LowerWindow)
{
	lower(cli);
}

UCALLBACK(Maximize)
{
	maximize(cli);
}

UCALLBACK(Close)
{
	destroy(cli, 0);
}

UCALLBACK(Hide)
{
	hide(cli);
}

UCALLBACK(Refresh)
{
	// Apparently, taking a window and re-mapping it allows
	// it to gain back the ability to focus. Huh.
	XUnmapWindow(cli->dpy, cli->win);
	XMapWindow(cli->dpy, cli->win);
}

#define NSHORTCUTS 6
static uevent_t SHORTCUTS[NSHORTCUTS] = {
	{XK_Page_Up, RaiseWindow},
	{XK_Page_Down, LowerWindow},
	{XK_m, Maximize},
	{XK_c, Close},
	{XK_h, Hide},
	{XK_r, Refresh},
};

// Used for event loop callback (ie from X)
#define CALLBACK(name) void name(Display* dpy, XEvent* evp)
CALLBACK(eKeyPress);
CALLBACK(eButtonPress);
CALLBACK(eButtonRelease);
CALLBACK(eMotionNotify);
CALLBACK(eMapNotify);

#endif
