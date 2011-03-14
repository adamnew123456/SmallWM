#ifndef __EVENT__
#define __EVENT__

#include "global.h"

/* Note that this is hackish, but works and is an easy place to put the functions */
typedef struct {
	KeySym ksym;
	void (*callback)(Display*, XEvent);
} uevent_t;	

#define UCALLBACK(name) static void name(Display *dpy, XEvent ev)

///////////////////////////////////////////////
UCALLBACK(RaiseWindow){
	XRaiseWindow(dpy, ev.xkey.subwindow);
}

UCALLBACK(LowerWindow){
	XLowerWindow(dpy, ev.xkey.subwindow);
}

UCALLBACK(Maximize){
	XMoveResizeWindow(dpy, ev.xkey.subwindow, 0, 0,
			XDisplayWidth(dpy, DefaultScreen(dpy)),
			XDisplayHeight(dpy, DefaultScreen(dpy))
	);
}

UCALLBACK(Close){
	XDestroyWindow(dpy, ev.xkey.subwindow);
}
////////////////////////////////////////////////
#define NSHORTCUTS 4
static uevent_t SHORTCUTS[NSHORTCUTS] = {
	{ XK_Page_Up, RaiseWindow },
	{ XK_Page_Down, LowerWindow },
	{ XK_m, Maximize },
	{ XK_c, Close },
};

#define CALLBACK(name) void name(Display*, XEvent)
CALLBACK(eKeyPress);
CALLBACK(eButtonPress);
CALLBACK(eButtonRelease);
CALLBACK(eMotionNotify);
CALLBACK(eMapNotify);

#endif
