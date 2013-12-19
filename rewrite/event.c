/* Recieve and handle X events */
#include "event.h"

// Creates a new empty event table
events_t *event_init(smallwm_t *smallwm)
{
    events_t *events = malloc(sizeof(events_t));
    events->event_callbacks = new_table();
    events->key_callbacks = new_table();

    int idx = 0;
    while (event_callbacks[idx])
    {
        add_table(events->event_callbacks, event_types[idx], event_callbacks[idx]);
        idx++;
    }

    idx = 0;
    while (key_callbacks[idx])
    {
        add_table(events->key_callbacks, keysym_types[idx], key_callbacks[idx]);
        idx++;
    }

    events->wm = smallwm;
    return events;
}

// Adds a keyboard event handler
void add_keyboard_handler_event(events_t *events, KeySym key, event_callback_t *callback)
{
    add_table(events->key_callbacks, key, callback);
}

// Adds a general event handler
void add_handler_event(events_t *events, int event, event_callback_t *callback)
{
    add_table(events->event_callbacks, event, callback);
}

// Dispatches on X events
void run_loop_event(events_t *events)
{
    XEvent event;
    while (1)
    {
        XNextEvent(events->wm->display, *event);

        event_callback_t *handler = get_table(events->event_callbacks, event.type);
        if (handler)
            handler(events->wm, &event);
    }
}

// Handle keyboard commands by simple table dispatch
void on_keypress_event(smallwm_t *wm, XEvent *event)
{
    int nkeys;
    KeySym *keysym = NULL;
    keysym = XGetKeyboardMapping(wm->display, event->xkey->keycode, 1, &nkeys);

    event_callback_t *handler = get_table(events->key_callbacks, *keysym);
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
    client_t *client = get_table(wm->clients, event->subwindow);
    icon_t *icon = get_table(wm->icons, event->window);

    //  click on the root window
    if (!(client || icon) && ev.xbutton.button == LAUNCH)
    {
        shell_launch_wm(wm);
    }
    else if (icon)
    {
        to_client(icon);
    }
    else if (client)
    {
        if (event->xbutton->state == MASK)
        {
            switch (event->xbutton.button)
            {
            case MOVE:
            {
                wm->movement.state = MR_MOVE;
                wm->movement.event = event->xbutton;
                wm->movement.window = wm->window;
                begin_moveresize_cleint(client);
            }; break;
            case RESZ:
            {
                wm->movement.state = MR_RESZ;
                wm->movement.event = event->xbutton;
                wm->movement.window = event->window;
                begin_moveresize_client(client);
            }; break;
            }
        }
        else
        {
            client = get_table(wm->clients, event->window);
            refocus_wm(wm, client->window);
        }
    }
}

// Stops moving/resizing if there is anything which is being moved/resized
void on_buttonrelease_event(smallwm_t *wm, XEvent *event)
{
    if (wm->movement.state != MV_NONE)
    {
        client_t *client = get_table(wm->clients, event->xbutton.window);
        end_moveresize_client(client);

        wm->movement.state = MV_NONE;
    }
}

// Updates the window which is being moved or resized
void on_motionnotify_event(smallwm_t *wm, XEvent *event)
{
    if (wm->movement.state == MV_NONE)
        return;

    // Go ahead and get the final movement to avoid needless updating
    while (XCheckTypedEvent(wm->display, MotionNotify, event));

    // Do the movement
    int xdiff, ydiff;
    xdiff = ev->xbutton.x_root - wm->movement.event.x_root;
    ydiff = ev->xbutton.y_root - wm->movement.event.y_root;

    // Get the old location of the window
    XWindowAttributes attr;
    XGetWindowAttributes(wm->display, wm->movement.client->mvresz_placeholder, &attr);

    if (wm->movement.state == MR_MOVE)
        XMoveWindow(wm->display, wm->movement.client->mvresz_placeholder,
                attr.x + xdiff, attr.y + ydiff);

    if (wm->movement.state == MR_RESZ)
    {
        int new_width = attr.w + xdiff;
        int new_height = attr.h + ydiff;

        XResizeWindow(wm->display, wm->movement.client->mvresz_placeholder,
                (new_width > 1 ? new_width : 1), (new_height > 1 ? new_height : 1));
    }
}

// Initialize a new client
void on_mapnotify_event(smallwm_t *wm, XEvent *event)
{
    add_client_wm(wm, event->xmap.window);
}
