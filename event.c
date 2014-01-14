/* Recieve and handle X events */
#include "event.h"

// Creates a new empty event table
events_t *event_init(smallwm_t *wm)
{
    events_t *events = malloc(sizeof(events_t));
    events->event_callbacks = new_table();
    events->key_callbacks = new_table();

    XGrabButton(wm->display, MOVE, MASK, wm->root, True, ButtonPressMask,
                GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(wm->display, RESZ, MASK, wm->root, True, ButtonPressMask,
                GrabModeAsync, GrabModeAsync, None, None);

    event_pair_t *event_pair = event_callbacks;
    while (event_pair->callback != NULL)
    {
        add_table(events->event_callbacks, event_pair->type, event_pair->callback);
        event_pair++;
    }

    event_pair = keysym_callbacks;
    while (event_pair->callback != NULL)
    {
        add_table(events->key_callbacks, event_pair->type, event_pair->callback);

        int keycode = XKeysymToKeycode(wm->display, event_pair->type);
        XGrabKey(wm->display, keycode, MASK, wm->root, True, GrabModeAsync, GrabModeAsync);
        
        event_pair++;
    }

    events->wm = wm;
    wm->events = events;
    return events;
}

// Adds a keyboard event handler
void add_keyboard_handler_event(events_t *events, KeySym key, event_callback_t callback)
{
    add_table(events->key_callbacks, key, callback);
}

// Adds a general event handler
void add_handler_event(events_t *events, int event, event_callback_t callback)
{
    add_table(events->event_callbacks, event, callback);
}

// Dispatches on X events
void run_loop_event(events_t *events)
{
    XEvent event;
    while (1)
    {
        XNextEvent(events->wm->display, &event);

        event_callback_t handler = get_table(events->event_callbacks, event.type);
        if (handler)
            handler(events->wm, &event);
    }
}

// Handle keyboard commands by simple table dispatch
void on_keypress_event(smallwm_t *wm, XEvent *event)
{
    int nkeys;
    KeySym *keysym = NULL;
    keysym = XGetKeyboardMapping(wm->display, event->xkey.keycode, 1, &nkeys);

    event_callback_t handler = get_table(wm->events->key_callbacks, *keysym);
    if (handler)
        handler(wm, event);
}

/* Handle a button press, which can do one of a few things:
 *  (a) Changes the focus when clicking on an unfocused window
 *      (in this case, the mask will not be equal to MASK)
 *  (b) This function starts moving or resizing
 *  (c) This function shows a previously hidden window
 *  (d) Launches a shell
*/
void on_buttonpress_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xbutton.subwindow);
    icon_t *icon = get_table(wm->icons, event->xbutton.window);

    //  click on the root window
    if (!(client || icon) && event->xbutton.button == LAUNCH && event->xbutton.state == MASK)
    {
        shell_launch_wm(wm);
    }
    else if (icon)
    {
        to_client(icon);
    }
    else if (client && event->xbutton.state == MASK)
    {
        switch (event->xbutton.button)
        {
        case MOVE:
        {
            wm->movement.state = MR_MOVE;
            wm->movement.event = event->xbutton;
            wm->movement.client = client;
            begin_moveresize_client(client);
        }; break;
        case RESZ:
        {
            wm->movement.state = MR_RESZ;
            wm->movement.event = event->xbutton;
            wm->movement.client = client;
            begin_moveresize_client(client);
        }; break;
        }
    }
    else
    {
        client_t *client = get_table(wm->clients, event->xbutton.window);
        if (!client) return;

        refocus_wm(wm, client->window);
    }
}

// Stops moving/resizing if there is anything which is being moved/resized
void on_buttonrelease_event(smallwm_t *wm, XEvent *event)
{
    if (wm->movement.state != MR_NONE)
    {
        client_t *client = get_table(wm->clients, event->xbutton.window);
        end_moveresize_client(wm->movement.client);

        wm->movement.state = MR_NONE;
    }
}

// Updates the window which is being moved or resized
void on_motionnotify_event(smallwm_t *wm, XEvent *event)
{
    if (wm->movement.state == MR_NONE)
        return;

    // Go ahead and get the final movement to avoid needless updating
    while (XCheckTypedEvent(wm->display, MotionNotify, event));

    // Do the movement
    int xdiff, ydiff;
    xdiff = event->xbutton.x_root - wm->movement.event.x_root;
    ydiff = event->xbutton.y_root - wm->movement.event.y_root;

    // Update the WM event to the most recent version
    wm->movement.event = event->xbutton;

    // Get the old location of the window
    XWindowAttributes attr;
    XGetWindowAttributes(wm->display, wm->movement.client->mvresz_placeholder, &attr);

    if (wm->movement.state == MR_MOVE)
        XMoveWindow(wm->display, wm->movement.client->mvresz_placeholder,
                attr.x + xdiff, attr.y + ydiff);

    if (wm->movement.state == MR_RESZ)
    {
        int new_width = attr.width + xdiff;
        int new_height = attr.height + ydiff;

        XResizeWindow(wm->display, wm->movement.client->mvresz_placeholder,
                (new_width > 1 ? new_width : 1), (new_height > 1 ? new_height : 1));
    }
}

// Initialize a new client
void on_mapnotify_event(smallwm_t *wm, XEvent *event)
{
    add_client_wm(wm, event->xmap.window);
}

// Redraw the exposed icon
void on_expose_event(smallwm_t *wm, XEvent *event)
{
    icon_t *icon = get_table(wm->icons, event->xexpose.window);
    if (!icon) return;

    paint_icon(icon);
}

// Destroy the client backing a removed window
void on_destroynotify_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xdestroywindow.window);
    if (!client) return;

    del_table(wm->dialogs, event->xdestroywindow.window);
    destroy_client(client);
}

// Raise a client to the top
void do_raise_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    raise_client(client);
}

// Lower a client to the bottom
void do_lower_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    lower_client(client);
}

// Maximize a client
void do_maximize_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    maximize_client(client);
}

// Request a client to close
void do_close_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    request_close_client(client);
}

// Force a client to close
void do_kill_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    XDestroyWindow(wm->display, client->window);
}

// Hide a particular client
void do_hide_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    to_icon(client);
}

// Move to the next desktop
void do_desktopnext_event(smallwm_t *wm, XEvent *event)
{
    wm->current_desktop++;
    if (wm->current_desktop == wm->num_desktops)
        wm->current_desktop = 0;

    update_desktop_wm(wm);
}

// Move to the previous desktop
void do_desktopprev_event(smallwm_t *wm, XEvent *event)
{
    wm->current_desktop--;
    
    // This works since the desktop types are unsigned
    if (wm->current_desktop >= wm->num_desktops)
        wm->current_desktop = wm->num_desktops - 1;

    update_desktop_wm(wm);
}

// Move a client to the next desktop
void do_movetodesktopnext_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    client->desktop++;
    if (client->desktop == wm->num_desktops)
        client->desktop = 0;

    update_desktop_wm(wm);
}

// Move a client to the previous desktop
void do_movetodesktopprev_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    client->desktop--;

    // As in do_desktopprev_event
    if (client->desktop >= wm->num_desktops)
        client->desktop = wm->num_desktops - 1;
    
    update_desktop_wm(wm);
}

// Stick/unstick a client to/from all desktops
void do_stick_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    client->sticky = !client->sticky;

    // Since unsticking a window could make it disappear
    update_desktop_wm(wm);
}

// Snap a client to the left side of the screen
void do_snapleft_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    snap_left_client(client);
}

// Snap a client to the right side of the screen
void do_snapright_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    snap_right_client(client);
}

// Snap a client to the top of the screen
void do_snapup_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    snap_top_client(client);
}

// Snap a client to the bottom of the screen
void do_snapdown_event(smallwm_t *wm, XEvent *event)
{
    client_t *client = get_table(wm->clients, event->xkey.subwindow);
    if (!client) return;

    snap_bottom_client(client);
}

// Kill SmallWM and all children
void do_endwm_event(smallwm_t *wm, XEvent *event)
{
    exit(0);
}

// Setting the various windows
#define SETLAYER_FUNC(layer) void do_setlayer##layer##_event(smallwm_t *wm, XEvent *event) {\
    client_t *client = get_table(wm->clients, event->xkey.subwindow); \
    if (!client) return; \
    set_layer_client(client, layer); \
}

SETLAYER_FUNC(1);
SETLAYER_FUNC(2);
SETLAYER_FUNC(3);
SETLAYER_FUNC(4);
SETLAYER_FUNC(5);
SETLAYER_FUNC(6);
SETLAYER_FUNC(7);
SETLAYER_FUNC(8);
SETLAYER_FUNC(9);
