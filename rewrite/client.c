/* Routines for handling client windows */
#include "client.h"

// Puts a client on top of the stack
void raise_client(client_t *client)
{
    if (client->state == C_VISIBLE)
        XRaiseWindow(client->wm->display, client->window);
}

// Puts a client on the bottom of the stack
void lower_client(client_t *client)
{
    if (client->state == C_VISIBLE)
        XLowerWindow(client->wm->display, client->window);
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
}

// Removes a client once the window closes
void destroy_client(client_t *client)
{
    if (client->state == C_VISIBLE)
    {
        XDestroyWindow(client->wm->display, client->window);
        del_table(client->wm->clients, client->window);
        free(client);
    }
}
