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
    close_event.xclient = {
        .type = ClientMessage,
        .window = client->window,
        .message_type = XInternAtom(client->wm->display, "WM_PROTOCOLS", False),
        .format = 32,
        .data = {
            .l = { XInternAtom(client->wm->display, "WM_DELETE_WINDOW", False),
                   CurrentTime }
        }
    };
    XSendEvent(client->wm->display, client->window, False, NoEventMask, &close_event);
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
