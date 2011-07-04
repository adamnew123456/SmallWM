/*
 * Handling of Xlib events, specifically
 *  - KeyPress
 *  - ButtonPress
 *  - ButtonRelease
 *  - MotionNotify
 *  - MapNotify
 */
#include "event.h"

static XButtonEvent mouse;
static inmove = 0, inresz = 0;
client_t *moving;

void
eKeyPress (Display * dpy, XEvent ev)
{
    int nkeys;
    KeySym *ksym = NULL;
    ksym = XGetKeyboardMapping (dpy, ev.xkey.keycode, 1, &nkeys);

	if (*ksym == XK_Escape) exit(0);

	client_t *cli = fromwin(ev.xkey.subwindow);
	if (!cli) return;
	if (cli->state != Visible) return;

    int i;
    for (i = 0; i < NSHORTCUTS; i++)
    {
        if (*ksym == SHORTCUTS[i].ksym)
        {
            (*SHORTCUTS[i].callback) (ev, cli);
            break;
        }
    }
}

void
eButtonPress (Display * dpy, XEvent ev)
{
	client_t *cli = fromwin(ev.xbutton.subwindow);

    // Root window - run the SHELL
    if (ev.xbutton.subwindow == None &&
            ev.xbutton.button == 1 && !cli)
    {
        if (!fork ())
        {
            execlp (SHELL, SHELL, NULL);
            exit (1);
        }
    }
	
	if (inmove || inresz || !ev.xbutton.subwindow || ev.xbutton.state != MASK)
		return;

	if (!cli) return;

    if (ev.xbutton.button == MOVE)
    {
        inmove = 1;
		inresz = 0;
		beginmvrsz(cli);
		mouse = ev.xbutton;
		moving = cli;
    }
	else if (ev.xbutton.button = RESZ)
	{
		inmove = 0;
		inresz = 1;
		beginmvrsz(cli);
		mouse = ev.xbutton;
		moving = cli;
	}
}

void
eButtonRelease (Display * dpy, XEvent ev)
{
	client_t *cli = fromwin(ev.xbutton.subwindow);
	
    if (inmove || inresz)
    {
		endmoversz(cli);
        inmove = 0;
		inresz = 0;
		return;
    }

	if (cli) chfocus(cli);

	cli = fromicon(ev.xbutton.subwindow);
	if (cli) unhide(cli, 0);
}

void
eMotionNotify (Display * dpy, XEvent ev)
{
    if (!(inmove || inresz))
        return;

    // Get the latest move event - don't update needlessly
    while (XCheckTypedEvent (dpy, MotionNotify, &ev));

    // Visually move/resize
    int xdiff, ydiff;
    xdiff = ev.xbutton.x_root - mouse.x_root;
    ydiff = ev.xbutton.y_root - mouse.y_root;

    if (inmove)
        XMoveWindow (dpy, moving->pholder, moving->x + xdiff, moving->y + ydiff);
    if (inresz)
	{
        XResizeWindow (dpy, moving->pholder, MAX (1, moving->w + xdiff),
			MAX (1, moving->h + ydiff));
	}
}

void
eMapNotify (Display * dpy, XEvent ev)
{
	printf("A window has been mapped - %x\n", ev.xmap.window);
	create(dpy, ev.xmap.window);
}
