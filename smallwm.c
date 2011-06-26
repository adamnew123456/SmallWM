/*
 * TinyWM is written by Nick Welch <mack@incise.org>, 2005. SmallWM was
 * hacked out of TinyWM by Adam Marchetti <adamnew123456@gmail.com>, 2010.
 * This software is in the public domain and is provided AS IS, with NO
 * WARRANTY.
 */

#include "smallwm.h"

void
sigchld (int signal)
{
    int status;
    wait (&status);
}

void
getexisting(Display *dpy, Window root)
{
	Window w_;
	Window *childs;
	unsigned int nchilds;
	XQueryTree(dpy, root, &w_, &w_, &childs, &nchilds);

	int i = 0;
	for (i = 0; i < nchilds; i++)
	{
		if (childs[i] == root) continue;
		create(dpy, childs[i]);
	}
}

int
main ()
{
	// Reap the children
    signal (SIGCHLD, sigchld);

    XEvent ev;

    if (!(dpy = XOpenDisplay (NULL)))
        return 1;

    root = DefaultRootWindow (dpy);
    XSelectInput (dpy, root, KeyPressMask |
                  ButtonPressMask |
                  ButtonReleaseMask |
                  PointerMotionMask | SubstructureNotifyMask);

	getexisting(dpy, root);

    int i;
    for (i = 0; i < NSHORTCUTS; i++)
    {
        XGrabKey (dpy, XKeysymToKeycode (dpy, SHORTCUTS[i].ksym), MASK, root,
                  True, GrabModeAsync, GrabModeAsync);
    }

    // Exit key combo
    XGrabKey (dpy, XKeysymToKeycode (dpy, XK_Escape), MASK, root, True,
              GrabModeAsync, GrabModeAsync);

    // The move and resize buttons (also used for other stuff)
    XGrabButton (dpy, 1, MASK, root, True, ButtonPressMask, GrabModeAsync,
                 GrabModeAsync, None, None);
    XGrabButton (dpy, 3, MASK, root, True, ButtonPressMask, GrabModeAsync,
                 GrabModeAsync, None, None);

	// Run a shell upon startup
    if (!fork ())
    {
        execlp (SHELL, SHELL, NULL);
        exit (0);
    }

    while (1)
    {
        XNextEvent (dpy, &ev);
		client_t *cli;

        switch (ev.type)
        {
        case KeyPress:
            eKeyPress (dpy, ev);
            break;
        case ButtonPress:
            eButtonPress (dpy, ev);
            break;
        case ButtonRelease:
            eButtonRelease (dpy, ev);
            break;
        case MotionNotify:
            eMotionNotify (dpy, ev);
            break;
        case MapNotify:
            eMapNotify (dpy, ev);
            break;
        case Expose:
			cli = fromwin(ev.xexpose.window);
            paint(cli);
            break;
        case DestroyNotify:
			cli = fromwin(ev.xdestroywindow.window);
			destroy(cli, 1);
			break;
        }
    }
}
