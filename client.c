/* Routines for handling client windows */
#include "client.h"

// Puts a client on top of the stack
void raise_client(client_t *client)
{
    if (client->layer == 9)
        return;

    client->layer++;
    update_desktop_wm(client->wm);
}

// Puts a client on the bottom of the stack
void lower_client(client_t *client)
{
    if (client->layer == 1)
        return;

    client->layer--;
    update_desktop_wm(client->wm);
}

// Sets the layer of the client
void set_layer_client(client_t *client, int layer)
{
    if (layer < 1 || layer > 9 || client->layer == layer)
        return;

    client->layer = layer;
    update_desktop_wm(client->wm);
}

// Requests that a client close without forcing it to close
void request_close_client(client_t *client)
{
    // Being nice like this is mandated by the ICCCM
    XEvent close_event;

    XClientMessageEvent client_close = {
        .type = ClientMessage,
        .window = client->window,
        .message_type = XInternAtom(client->wm->display, "WM_PROTOCOLS", False),
        .format = 32,
        .data = {
            .l = { XInternAtom(client->wm->display, "WM_DELETE_WINDOW", False),
                   CurrentTime }
        }
    };

    close_event.xclient = client_close;
    XSendEvent(client->wm->display, client->window, False, NoEventMask, &close_event);
}

// Begins moving or resizing a client
void begin_moveresize_client(client_t *client)
{
    if (client->state != C_VISIBLE)
        return;
    client->state = C_MOVERESZ;
    
    // Update the location and size attributes before 
    XWindowAttributes attr;
    XGetWindowAttributes(client->wm->display, client->window, &attr);

    // Remove the window and create a placeholder
    XUnmapWindow(client->wm->display, client->window);
    client->mvresz_placeholder = XCreateSimpleWindow(client->wm->display,
            client->wm->root, attr.x, attr.y, attr.width, attr.height, 1, 
            BlackPixel(client->wm->display, client->wm->screen), 
            WhitePixel(client->wm->display, client->wm->screen));

    // Ask the WM module to ignore this window
    XSetWindowAttributes set_attr;
    set_attr.override_redirect = True;
    XChangeWindowAttributes(client->wm->display, client->mvresz_placeholder, CWOverrideRedirect, &set_attr);
    ignore_window_wm(client->wm, client->mvresz_placeholder);

    // Put up the window and force the pointer into the window
    XMapWindow(client->wm->display, client->mvresz_placeholder);
    XGrabPointer(client->wm->display, client->mvresz_placeholder, True,
            PointerMotionMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XRaiseWindow(client->wm->display, client->mvresz_placeholder);
}

// Stop moving or resizing the client
void end_moveresize_client(client_t *client)
{
    if (client->state != C_MOVERESZ)
        return;
    client->state = C_VISIBLE;

    // Unsubscribe from pointer events and free the pointer
    XUngrabPointer(client->wm->display, CurrentTime);

    XWindowAttributes attr;
    XGetWindowAttributes(client->wm->display, client->mvresz_placeholder, &attr);
    XDestroyWindow(client->wm->display, client->mvresz_placeholder);

    // Place the original window
    XMapWindow(client->wm->display, client->window);
    XMoveResizeWindow(client->wm->display, client->window, attr.x, attr.y, attr.width, attr.height);

    raise_client(client);
    refocus_wm(client->wm, client->window);
}

// Do the actions which are stored for a particular window's class
void do_actions_client(client_t *client)
{
    XClassHint *classhint = XAllocClassHint();
    XGetClassHint(client->wm->display, client->window, classhint);
    if (!classhint->res_class)
        goto free_classhint;

    classaction_t *actions = get_table(client->wm->classactions, string_hash(classhint->res_class));
    if (!actions)
        goto free_classhint;

    action_t *action_ptr = actions->actions;
    for (action_ptr = actions->actions; *action_ptr != ACT_END; action_ptr++)
    {
        switch (*action_ptr)
        {
            case ACT_STICK:
                client->sticky = !client->sticky;
                break;
            case ACT_MAXIMIZE:
                maximize_client(client);
                break;
            case ACT_SNAPLEFT:
                snap_left_client(client);
                break;
            case ACT_SNAPRIGHT:
                snap_right_client(client);
                break;
            case ACT_SNAPTOP:
                snap_top_client(client);
                break;
            case ACT_SNAPBOTTOM:
                snap_bottom_client(client);
                break;
            case ACT_SETLAYER:
                client->layer = actions->layer;
                break;
        }
    }

free_classhint:
    XFree(classhint);
}

/*
 * These are vairous client manipulation routines which were imported from the
 * event module, but now exist here. Their naming scheme should be obvious
 * enough, so they are not annotated.
*/

void maximize_client(client_t *client)
{
    XMoveResizeWindow(client->wm->display, client->window, 
            0, client->wm->icon_height, 
            client->wm->width, client->wm->height - client->wm->icon_height);
}

void snap_left_client(client_t *client)
{
    XMoveResizeWindow(client->wm->display, client->window, 
            0, client->wm->icon_height, 
            client->wm->width / 2, client->wm->height - client->wm->icon_height);
}

void snap_right_client(client_t *client)
{
    XMoveResizeWindow(client->wm->display, client->window, 
            client->wm->width / 2, client->wm->icon_height, 
            client->wm->width / 2, client->wm->height - client->wm->icon_height);
}

void snap_top_client(client_t *client)
{
    XMoveResizeWindow(client->wm->display, client->window, 
            0, client->wm->icon_height, client->wm->width, 
            (client->wm->height / 2) - client->wm->icon_height);
}

void snap_bottom_client(client_t *client)
{
    XMoveResizeWindow(client->wm->display, client->window, 
            0, client->wm->height / 2, client->wm->width, client->wm->height / 2);
}

// Removes a client once the window closes
void destroy_client(client_t *client)
{
    // If the moving/resizing operation is interrupted, then
    // undo the effects immediately
    if (client->state == C_MOVERESZ)
    {
        // I assume that X removes the pointer grab if the window is destroyed
        XDestroyWindow(client->wm->display, client->mvresz_placeholder);
    }

    // Make sure to remove the focus if the dead client was focused before
    if (client->wm->focused_window == client->window)
        client->wm->focused_window = None;

    del_table(client->wm->clients, client->window);
    free(client);
}
