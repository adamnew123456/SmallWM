/* TinyWM is written by Nick Welch <mack@incise.org>, 2005.
 * SmallWM was hacked out of TinyWM by Adam Marchetti <adamnew123456@gmail.com>, 2010.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

#include "smallwm.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main()
{
    Display * dpy;
    Window root;
    XEvent ev;

    if(!(dpy = XOpenDisplay(NULL))) return 1;

    root = DefaultRootWindow(dpy);
    XSelectInput(dpy, root, KeyPressMask |
		    	    ButtonPressMask |
			    ButtonReleaseMask |
			    PointerMotionMask |
			    LeaveWindowMask |
			    SubstructureNotifyMask);

    int i;
    for (i = 0; i < NSHORTCUTS; i++){
	    XGrabKey(dpy, XKeysymToKeycode(dpy, SHORTCUTS[i].ksym), MASK, root, True, GrabModeAsync, GrabModeAsync);
   }
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Escape), MASK, root, True, GrabModeAsync, GrabModeAsync);

    XGrabButton(dpy, 1, MASK, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, MASK, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);

    if (!fork()){
	execlp(SHELL, SHELL, NULL);
	exit(0);
    }  

    Window focus = None;
    Window dump, child;
    int rx, ry, cx, cy;
    unsigned int mask;

    for(;;)
    {
	XNextEvent(dpy, &ev);
	
	switch (ev.type){
		case KeyPress:
			eKeyPress(dpy, ev);
			break;
		case ButtonPress:
			eButtonPress(dpy, ev);
			break;
		case ButtonRelease:
			eButtonRelease(dpy, ev);
			break;
		case MotionNotify:
			eMotionNotify(dpy, ev);
			break;
		case MapNotify:
			eMapNotify(dpy, ev);
			break;
	}
        
	XQueryPointer(dpy, root, &dump, &child, 
			&rx, &ry, &cx, &cy,
			&mask);
	XSetInputFocus(dpy, dump, RevertToNone, CurrentTime); 
    }
}
