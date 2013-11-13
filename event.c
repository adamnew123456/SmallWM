#include "event.h"

moving_t moving_state;

// Handle a keypress which is listened to by SmallWM - see events.h for a list
// of keys handled here
CALLBACK(eKeyPress)
{
    GET_EVENT;

    int nkeys;
    KeySym *ksym = NULL;
    ksym = XGetKeyboardMapping(dpy, ev.xkey.keycode, 1, &nkeys);

    switch (*ksym) {
    case XK_Escape:
        exit(0);
        return;
    case XK_comma:
        // This is essentially a basic modulo - since I'm not sure how
        // compilers handle the case of negative modulos, just "unfold" the
        // operation to make sure current_desktop is positive
        current_desktop--;
        while (current_desktop < 0)
            current_desktop += MAX_DESKTOP;

        set_desktop();
        return;
    case XK_period:
        current_desktop = (current_desktop + 1) % MAX_DESKTOP;
        set_desktop();
        return;
    }

    client_t *cli = fromwin(ev.xkey.subwindow);
    if (!cli || cli->state != Visible)
        return;

    int i;
    for (i = 0; i < NSHORTCUTS; i++) {
        if (*ksym == SHORTCUTS[i].ksym) {
            (*SHORTCUTS[i].callback) (evp, cli);
            break;
        }
    }
}

// Handle a button press - this function has a couple of important cases:
//  a) This function will change focus if the user is just clicking on an 
//     unfocused window (in this case, the MASK will not appear in the event)
//  b) This function starts moving if the user does a left click
//  c) This function starts resizing if the user does a right click
CALLBACK(eButtonPress)
{
    GET_EVENT;

    client_t *cli = fromwin(ev.xbutton.subwindow);
    client_t *ico = fromicon(ev.xbutton.window);

    if (ico)
        unhide(ico, 0);
    // This is a really complex test to check to see if
    // we are clicking the root window. Can it be simplfied?
    else if (ev.xbutton.window == ev.xbutton.root &&
         ev.xbutton.button == 1 && !cli && !ico) {
        if (!fork()) {
            execlp(SHELL, SHELL, NULL);
            exit(1);
        }
    }

    if (moving_state.inmove || moving_state.inresz
        || !(ev.xbutton.subwindow || ev.xbutton.window != ev.xbutton.root))
        return;

    if (ev.xbutton.state == MASK) {
        if (!cli)
            return;

        if (ev.xbutton.button == MOVE) {
            moving_state.inmove = 1;
            moving_state.inresz = 0;
            beginmvrsz(cli);
            moving_state.mouse = ev.xbutton;
            moving_state.client = cli;
        } else if (ev.xbutton.button == RESZ) {
            moving_state.inmove = 0;
            moving_state.inresz = 1;
            beginmvrsz(cli);
            moving_state.mouse = ev.xbutton;
            moving_state.client = cli;
        }
    } else {
        cli = fromwin(ev.xbutton.window);

        if (!cli)
            return;

        chfocus(cli->dpy, cli->win);
    }
}

// This function just stops moving and resizing if the user was moving/resizing
// before.
CALLBACK(eButtonRelease)
{
    GET_EVENT;

    client_t *cli = fromwin(ev.xbutton.subwindow);

    if (moving_state.inmove || moving_state.inresz) {
        endmoversz(moving_state.client);
        moving_state.inmove = 0;
        moving_state.inresz = 0;
        return;
    }
}

// This function updates the window that is being moved/resized
CALLBACK(eMotionNotify)
{
    GET_EVENT;

    if (!(moving_state.inmove || moving_state.inresz))    // We don't care
        return;

    // Get the latest move event - don't update needlessly
    while (XCheckTypedEvent(dpy, MotionNotify, evp)) ;

    // Visually move/resize
    int xdiff, ydiff;
    xdiff = ev.xbutton.x_root - moving_state.mouse.x_root;
    ydiff = ev.xbutton.y_root - moving_state.mouse.y_root;

    if (moving_state.inmove)
        XMoveWindow(dpy, moving_state.client->pholder,
                moving_state.client->x + xdiff,
                moving_state.client->y + ydiff);

    if (moving_state.inresz)
        XResizeWindow(dpy, moving_state.client->pholder,
                  MAX(1, moving_state.client->w + xdiff), MAX(1,
                                      moving_state.client->h
                                      +
                                      ydiff));
}

// This function creates a new window when it is created
CALLBACK(eMapNotify)
{
    GET_EVENT;
    create(dpy, ev.xmap.window);
}
