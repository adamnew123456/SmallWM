#include "event.h"

// Used for when the mouse is being used to resize/move
static XButtonEvent mouse;
static XWindowAttributes attr;

void eKeyPress(Display *dpy, XEvent ev){
	KeySym *ksym = NULL;
	int nkeys;
	ksym = XGetKeyboardMapping(dpy, ev.xkey.keycode, 1, &nkeys);

	// Exit - ALT-Escape
	if (*ksym == XK_Escape) exit(0);

	if (ev.xkey.subwindow == None) return;

	int i;
	for (i = 0; i < NSHORTCUTS; i++){
		if (*ksym == SHORTCUTS[i].ksym){
			(*SHORTCUTS[i].callback)(dpy, ev);
			break;
		}
	}
}

void eButtonPress(Display *dpy, XEvent ev){
	if (ev.xbutton.subwindow == None && ev.xbutton.button == 1){
		if (!fork()){
			execlp(SHELL, SHELL, NULL);
			exit(1);
		}
	}
	else if (ev.xbutton.subwindow == None)
		return;
	else {
		XGrabPointer(dpy, ev.xbutton.subwindow, True,
				PointerMotionMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
		XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
		XRaiseWindow(dpy, ev.xbutton.subwindow);
		mouse = ev.xbutton;
	}
}

void eButtonRelease(Display *dpy, XEvent ev){
	XUngrabPointer(dpy, CurrentTime);
	XSetInputFocus(dpy, ev.xbutton.subwindow, RevertToPointerRoot, CurrentTime);
}

void eMotionNotify(Display *dpy, XEvent ev){
	int xdiff, ydiff;
	while (XCheckTypedEvent(dpy, MotionNotify, &ev));
	
	xdiff = ev.xbutton.x_root - mouse.x_root;
	ydiff = ev.xbutton.y_root - mouse.y_root;

	XMoveResizeWindow(dpy, ev.xmotion.window,
			attr.x + (mouse.button==1 ? xdiff : 0),
			attr.y + (mouse.button==1 ? ydiff : 0),
			MAX(1, attr.width + (mouse.button==3 ? xdiff : 0)),
			MAX(1, attr.height + (mouse.button==3 ? ydiff : 0))
	);
}

void eMapNotify(Display *dpy, XEvent ev){
	if (ev.xmap.override_redirect) return;
	XSetWindowBorderWidth(dpy, ev.xmap.window , 3);
}
