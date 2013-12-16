/*
 * This is most of the abstraction for dealing with clients.
*/
#include "client.h"

client_t *head;
Window focused;
int current_desktop = 0;

// Get SmallWM to ignore this window (icons or resizing & moving)
void ignore(Display * dpy, Window win)
{
    XSetWindowAttributes attr;
    attr.override_redirect = True;
    XChangeWindowAttributes(dpy, win, CWOverrideRedirect, &attr);
}

// Get the final client in the linked list
client_t *tail()
{
    client_t *c = head;
    while (c->next)
        c = c->next;
    return c;
}

// Get the client based upon the window of the icon
client_t *fromicon(Window icon)
{
    client_t *c = head;
    while (c) {
        if (c->state == Hidden && c->icon->win == icon)
            return c;
        c = c->next;
    }
    return NULL;
}

// Get the client based upon the main window
client_t *fromwin(Window win)
{
    client_t *c = head;
    while (c) {
        if (c->state == Visible && c->win == win)
            return c;
        c = c->next;
    }
    return NULL;
}

// Refreshes the dimensions of this client.
// (needed for some clients, like stalonetray, that resize themselves)
void update_dims(client_t * cli, XWindowAttributes * use_this_attr)
{
    XWindowAttributes attr;
    if (use_this_attr != NULL)
        attr = *use_this_attr;
    else
        XGetWindowAttributes(cli->dpy, cli->win, &attr);

    cli->x = attr.x;
    cli->y = attr.y;
    cli->w = attr.width;
    cli->h = attr.height;
}

// Create a new client given the window
client_t *create(Display * dpy, Window win)
{
    // An override_redirect flag indicates that the window manager should not
    // manage this window (such as a dialog, or a window internal to SmallWM
    // itself)
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, win, &attr);

    if (attr.override_redirect)
        return NULL;

    // Make sure this window doesn't already exist
    if (fromwin(win))
        return NULL;

    client_t *cli = malloc(sizeof(client_t));
    XSetWindowBorderWidth(dpy, win, 3);

    cli->dpy = dpy;
    cli->win = win;
    cli->pholder = None;
    cli->icon = NULL;
    cli->state = Visible;
    cli->desktop = current_desktop;
    cli->next = NULL;

    update_dims(cli, &attr);

    if (!head)
        head = cli;
    else {
        client_t *tl = tail();
        tl->next = cli;
    }

    chfocus(cli->dpy, cli->win);

    return cli;
}

// Get rid of an existing client
void destroy(client_t * client, int delete_immediately)
{

    if (!delete_immediately) {
        // Be nice and follow the ICCCM instead of just destroying the window
        XEvent close_event;
        close_event.xclient.type = ClientMessage;
        close_event.xclient.window = client->win;
        close_event.xclient.message_type =
            XInternAtom(client->dpy, "WM_PROTOCOLS", False);
        close_event.xclient.format = 32;
        close_event.xclient.data.l[0] =
            XInternAtom(client->dpy, "WM_DELETE_WINDOW", False);
        close_event.xclient.data.l[1] = CurrentTime;
        XSendEvent(client->dpy, client->win, False, NoEventMask,
               &close_event);
    } else {
        /* Only delete the window and shift the linked list around if it
         * is being destroyed now, and this is not merely a request
         */

        // Just make sure that the window is actually gone
        XDestroyWindow(client->dpy, client->win);
        client_t *prec = head;
        client_t *succ = client->next;
        while (prec && prec->next != client)
            prec = prec->next;

        if (!prec)
            return;

        if (focused == client->win)
            focused = None;

        prec->next = succ;
        if (client->state == Hidden)
            unhide(client, 1);

        free(client);

    }

    updicons();
}

// Hide (iconify) a client
void hide(client_t * client)
{
    if (client->state != Visible)
        return;

    client->icon = malloc(sizeof(icon_t));

    // Note the -200's - they position the window off the screen until it is
    // repositioned.
    client->icon->win = XCreateSimpleWindow(client->dpy,
                        RootWindow(client->dpy,
                               DefaultScreen
                               (client->dpy)),
                        -200, -200, ICON_WIDTH,
                        ICON_HEIGHT, 1,
                        BLACK(client->dpy),
                        WHITE(client->dpy));

    ignore(client->dpy, client->icon->win);

    XSelectInput(client->dpy, client->icon->win,
             ButtonPressMask | ButtonReleaseMask | ExposureMask);

    XMapWindow(client->dpy, client->icon->win);

    client->icon->gc = XCreateGC(client->dpy, client->icon->win, 0, NULL);

    XWMHints *hints = XGetWMHints(client->dpy, client->win);
    if (hints && hints->flags & IconPixmapHint) {
        client->icon->graphic = malloc(sizeof(icon_graphic_t));
        client->icon->graphic->pixmap = hints->icon_pixmap;

        // Only the width and height are useful - ignore everything else
        Window _root;
        int _x, _y;
        unsigned int _border;
        unsigned int _depth;

        XGetGeometry(client->dpy,
                 client->icon->graphic->pixmap,
                 &_root,
                 &_x, &_y,
                 &client->icon->graphic->w,
                 &client->icon->graphic->h, &_border, &_depth);
    } else
        client->icon->graphic = NULL;
    XFree(hints);

    XUnmapWindow(client->dpy, client->win);

    client->state = Hidden;

    updicons();
}

// Unhide (deiconify) a client - like in destroy(), danger is set if the
// client window does not exist and should not be remapped
void unhide(client_t * client, int danger)
{
    if (client->state != Hidden)
        return;

    if (!danger) {
        XMapWindow(client->dpy, client->win);
        XRaiseWindow(client->dpy, client->win);
    }

    XDestroyWindow(client->dpy, client->icon->win);
    XFreeGC(client->dpy, client->icon->gc);
    free(client->icon);

    client->state = Visible;

    if (client->desktop != ALL_DESKTOPS)
        client->desktop = current_desktop;

    updicons();
}

// Sets the current destkop by unmapping all the windows on other desktops
// and then mapping all the viewable windows
void set_desktop()
{
    client_t *client = head;
    XWindowAttributes attr;

    while (client) {
        if (client->state == Visible) {
            XGetWindowAttributes(client->dpy, client->win, &attr);

            char should_be_viewable =
                (client->desktop == current_desktop)
                || (client->desktop == ALL_DESKTOPS);

            if (attr.map_state == IsViewable && !should_be_viewable)
                XUnmapWindow(client->dpy, client->win);
            else if (attr.map_state != IsViewable
                 && should_be_viewable)
                XMapWindow(client->dpy, client->win);
        }
        client = client->next;
    }
}

// Begins moving or resizing a client - the only difference between the two is
// the type of event that triggers this to be called, since the logic is
// exactly the same
void beginmvrsz(client_t * client)
{
    if (client->state != Visible)
        return;

    update_dims(client, NULL);

    client->state = MoveResz;

    XUnmapWindow(client->dpy, client->win);

    client->pholder = XCreateSimpleWindow(client->dpy,
                          RootWindow(client->dpy,
                             DefaultScreen
                             (client->dpy)),
                          client->x, client->y, client->w,
                          client->h, 1, BLACK(client->dpy),
                          WHITE(client->dpy));

    ignore(client->dpy, client->pholder);

    XMapWindow(client->dpy, client->pholder);

    XGrabPointer(client->dpy, client->pholder, True,
             PointerMotionMask | ButtonReleaseMask,
             GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

    XRaiseWindow(client->dpy, client->pholder);
}

// Finalize a window after moving/resizing it. As with beginmvresz(), the code
// paths are identical.
void endmoversz(client_t * client)
{
    if (client->state != MoveResz)
        return;
    client->state = Visible;

    XUngrabPointer(client->dpy, CurrentTime);

    XWindowAttributes attr;
    XGetWindowAttributes(client->dpy, client->pholder, &attr);
    XDestroyWindow(client->dpy, client->pholder);

    update_dims(client, &attr);

    XMapWindow(client->dpy, client->win);
    XMoveResizeWindow(client->dpy, client->win, client->x, client->y,
              client->w, client->h);

    raise_(client);
}

// Raise a client to the top of the view stack
void raise_(client_t * client)
{
    if (client->state != Visible)
        return;
    XRaiseWindow(client->dpy, client->win);
}

// Lower a client to the bottom of the view stack
void lower(client_t * client)
{
    if (client->state != Visible)
        return;
    XLowerWindow(client->dpy, client->win);
}

// Maximize a client. Note that SCREEN_WIDTH and SCREEN_HEIGHT (global.h) don't
// work with xrandr - they are set when SmallWM starts.
void maximize(client_t * client)
{
    client->x = 0;
    client->y = ICON_HEIGHT;
    client->w = SCREEN_WIDTH(client->dpy);
    client->h = SCREEN_HEIGHT(client->dpy) - ICON_HEIGHT;

    XMoveResizeWindow(client->dpy, client->win, client->x, client->y,
              client->w, client->h);
}

// Reflow all the icons into rows on the top of the screen
void updicons()
{
    client_t *curr = head;
    int x = 0;
    int y = 0;

    while (curr) {
        if (curr->state != Hidden) {
            curr = curr->next;
            continue;
        }

        if (x + ICON_WIDTH > SCREEN_WIDTH(curr->dpy)) {
            x = 0;
            y += ICON_HEIGHT;
        }

        curr->icon->x = x;
        curr->icon->y = y;

        XMoveWindow(curr->dpy, curr->icon->win, curr->icon->x,
                curr->icon->y);

        paint(curr);

        curr = curr->next;
        x += ICON_WIDTH;
    }
}

// Draws a specific icon, including the window title and the pixmap
void paint(client_t * client)
{
    char *title;
    XGetIconName(client->dpy, client->win, &title);

    // Some applications do not set their own icon name - use their main window
    // name as the backup
    if (title == NULL)
        XFetchName(client->dpy, client->win, &title);

    int text_offset =
        client->icon->graphic != NULL ? client->icon->graphic->w : 0;

    XClearWindow(client->dpy, client->icon->win);

    XDrawString(client->dpy, client->icon->win, client->icon->gc,
            text_offset, ICON_HEIGHT, title,
            MIN((title ? strlen(title) : 0), 10));

    if (client->icon->graphic != NULL)
        XCopyArea(client->dpy, client->icon->graphic->pixmap,
              client->icon->win, client->icon->gc, 0, 0,
              client->icon->graphic->w, client->icon->graphic->h, 0,
              0);

    XFree(title);
}

/* Change focus to the given window
 *
 * The idea for this was snatched (blatantly) from 9wm, which has a really
 * neat click-to-focus implementation that's insanely easy to follow.
 *
 * This method grabs a button on the unfocused window, and eButtonPress()
 * (event.c) responds to that button grab. The window to get the focus has
 * its button grab removed and is given the input focus.
*/
void chfocus(Display * dpy, Window win)
{
    if (focused != None) {
        XGrabButton(dpy, AnyButton, AnyModifier, focused,
                False, ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeAsync, None, None);
    }

    XWindowAttributes attr;
    XGetWindowAttributes(dpy, win, &attr);
    if (attr.class != InputOnly && attr.map_state == IsViewable) {
        XUngrabButton(dpy, AnyButton, AnyModifier, win);
        XSetInputFocus(dpy, win, RevertToPointerRoot, CurrentTime);

        Window focusr;
        int revert;
        XGetInputFocus(dpy, &focusr, &revert);
        if (focusr != win) {
            XGrabButton(dpy, AnyButton, AnyModifier,
                    win, False,
                    ButtonPressMask | ButtonReleaseMask,
                    GrabModeAsync, GrabModeAsync, None, None);
            focused = None;
        } else
            focused = win;
    }
}
