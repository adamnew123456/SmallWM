#include "event.h"

static XButtonEvent mouse;
static int inmove = 0, inresz = 0;
client_t *moving; // Misnomer - used either (Moving and Resizing) way

CALLBACK(eKeyPress)
{
	GET_EVENT;

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
            (*SHORTCUTS[i].callback) (evp, cli);
            break;
        }
    }
}

CALLBACK(eButtonPress)
{	
	GET_EVENT;

	client_t *cli = fromwin(ev.xbutton.subwindow);
	client_t *ico = fromicon(ev.xbutton.window);

	if (ico)
	{
		unhide(ico, 0);
	}
	// This is a really complex test to check to see if
	// we are clicking the root window. Can it be simplfied?
	// (Probably - I'm guessing with `ev.xbutton.window == root`)
	else if (ev.xbutton.subwindow == None &&
            ev.xbutton.button == 1 && 
			!cli && !ico)
    {
        if (!fork())
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
	else if (ev.xbutton.button == RESZ)
	{
		inmove = 0;
		inresz = 1;
		beginmvrsz(cli);
		mouse = ev.xbutton;
		moving = cli;
	}
	else
		chfocus(cli);
}

CALLBACK(eButtonRelease)
{
	GET_EVENT;

	client_t *cli = fromwin(ev.xbutton.subwindow);
	
    if (inmove || inresz)
    {
		endmoversz(moving);
        inmove = 0;
		inresz = 0;
		return;
    }
}

CALLBACK(eMotionNotify)
{
	GET_EVENT;

    if (!(inmove || inresz)) // We don't care
        return;

    // Get the latest move event - don't update needlessly
    while (XCheckTypedEvent (dpy, MotionNotify, evp));

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

CALLBACK(eMapNotify)
{
	GET_EVENT;
	create(dpy, ev.xmap.window);
}
