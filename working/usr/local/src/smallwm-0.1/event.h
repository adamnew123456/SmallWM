#ifndef __EVENT__
#define __EVENT__

#include "global.h"
#include "icons.h"

typedef struct {
	KeySym ksym;
	void (*callback)(Display*, XEvent);
} uevent_t;	

// Used to define a keyboard shortcut callback
#define UCALLBACK(name) static void name(Display *dpy, XEvent ev)

///////////////////////////////////////////////
// Here are the keyboard shortcuts - their functions
// are implemented in the header as to be easy to edit.
// Their static definition doesn't hurt anything here,
// and this works well enough in practice. (Though it
// is bad form).
//
UCALLBACK(RaiseWindow){
	XRaiseWindow(dpy, ev.xkey.subwindow);
}

UCALLBACK(LowerWindow){
	XLowerWindow(dpy, ev.xkey.subwindow);
}

UCALLBACK(Maximize){
	XMoveResizeWindow(dpy, ev.xkey.subwindow, 0, IHEIGHT,
			SWIDTH,
			SHEIGHT - IHEIGHT // Leave room for a row of icons
	);
}

UCALLBACK(Close){
	XDestroyWindow(dpy, ev.xkey.subwindow);
}

UCALLBACK(Hide){
	hideWindow(dpy, ev.xkey.subwindow);
}

////////////////////////////////////////////////
// Remember to update NSHORTCUTS when you add any
// new keyboard functions
#define NSHORTCUTS 5
static uevent_t SHORTCUTS[NSHORTCUTS] = {
	{ XK_Page_Up, RaiseWindow },
	{ XK_Page_Down, LowerWindow },
	{ XK_m, Maximize },
	{ XK_c, Close },
	{ XK_h, Hide},
};

// Used for event loop callback (ie from X)
#define CALLBACK(name) void name(Display*, XEvent)
CALLBACK(eKeyPress);
CALLBACK(eButtonPress);
CALLBACK(eButtonRelease);
CALLBACK(eMotionNotify);
CALLBACK(eMapNotify);

#endif
