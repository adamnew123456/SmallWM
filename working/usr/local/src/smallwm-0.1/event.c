/* 
 * Although some of the code in here came from TinyWM (the mouse button code), most
 * of this is exclusive to SmallWM. Specifically, the key shortcut handling, and the
 * addition of bordered windows.
*/
#include "event.h"

// Used for when the mouse is being used to resize/move
static XButtonEvent mouse;
static XWindowAttributes attr;

void eKeyPress(Display *dpy, XEvent ev){
	KeySym *ksym = NULL;
	int nkeys;

	// Unfortunately, X provides no direct way to map keycodes to
	// keysyms more directly than this...
	ksym = XGetKeyboardMapping(dpy, ev.xkey.keycode, 1, &nkeys);

	// Exit - ALT-Escape
	if (*ksym == XK_Escape) exit(0);

	// All of these are window operations
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
	// Root window - run xterm
	if (ev.xbutton.subwindow == None && ev.xbutton.button == 1){
		if (!fork()){
			execlp(SHELL, SHELL, NULL);
			exit(1);
		}
	}
	else if (ev.xbutton.subwindow == None)
		return;
	else {  // Start move/resize
		XGrabPointer(dpy, ev.xbutton.subwindow, True,
				PointerMotionMask | ButtonReleaseMask,
				GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
		XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
		XRaiseWindow(dpy, ev.xbutton.subwindow);
		mouse = ev.xbutton;
	}
}

void eButtonRelease(Display *dpy, XEvent ev){
	// Stop move/resize
	XUngrabPointer(dpy, CurrentTime);
}

void eMotionNotify(Display *dpy, XEvent ev){
	int xdiff, ydiff;
	while (XCheckTypedEvent(dpy, MotionNotify, &ev));
	
	// Visually move/resize
	xdiff = ev.xbutton.x_root - mouse.x_root;
	ydiff = ev.xbutton.y_root - mouse.y_root;

	if (mouse.button == 1) XMoveWindow(dpy, ev.xmotion.window, attr.x + xdiff, attr.y + ydiff);
	if (mouse.button == 3) XMoveWindow(dpy, ev.xmotion.window, MAX(1, attr.width + xdiff), MAX(1, attr.height + ydiff));
}

void eMapNotify(Display *dpy, XEvent ev){
	if (!ev.xmap.override_redirect) // Ignore anything that we're not supposed to manage
		XSetWindowBorderWidth(dpy, ev.xmap.window , 3);
}
