/* TinyWM is written by Nick Welch <mack@incise.org>, 2005.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

/*
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
*/

#include "smallwm.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
// Mod2Mask is NumLock - Is essential for use with SmallWM
#define MASK Mod1Mask | Mod2Mask

int main()
{
    Display * dpy;
    Window root;
    XEvent ev;

    if(!(dpy = XOpenDisplay(NULL))) return 1;

    root = DefaultRootWindow(dpy);

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
	}
			
    }
}
