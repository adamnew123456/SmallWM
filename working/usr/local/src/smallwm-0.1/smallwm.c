/* TinyWM is written by Nick Welch <mack@incise.org>, 2005.
 * SmallWM was hacked out of TinyWM by Adam Marchetti <adamnew123456@gmail.com>, 2010.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

#include "smallwm.h"

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

    // Loops through all the key shortcuts in event.h and grabs them
    int i;
    for (i = 0; i < NSHORTCUTS; i++){
	    XGrabKey(dpy, XKeysymToKeycode(dpy, SHORTCUTS[i].ksym), MASK, root, True, GrabModeAsync, GrabModeAsync);
   }
    // Exit key combo
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Escape), MASK, root, True, GrabModeAsync, GrabModeAsync);

    // The move and resize buttons (also used for other stuff)
    XGrabButton(dpy, 1, MASK, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, MASK, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, None, None);

    if (!fork()){
	execlp(SHELL, SHELL, NULL);
	exit(0);
    }  

    while (1)
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
        
	// Sets the focus to wherever the pointer 
	// is (avoids focus stealing and other nastiness).
	//
	// Should be rather slow, seems to work with few
	// resource consumption here.
	Window dump, child;
	int rx, ry, cx, cy;
	unsigned int mask;
	
	XQueryPointer(dpy, root, &dump, &child, 
			&rx, &ry, &cx, &cy,
			&mask);

	XSetInputFocus(dpy, dump, RevertToNone, CurrentTime); 
    }
}
