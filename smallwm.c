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
		client_t *cli = fromwin(ev.xany.window);

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
            paint(cli);
            break;
        case DestroyNotify:
			destroy(cli, 1);
			break;
        }
    }
}
