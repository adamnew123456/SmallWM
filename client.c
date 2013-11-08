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

// Tail of the client list
client_t *tail()
{
    client_t *c = head;
    while (c->next)
        c = c->next;
    return c;
}

// Get the client based upon the icon
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

// Get the client based upon the window
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

// Updates the dimensions of a client.
void update_dims(client_t *cli, XWindowAttributes *use_this_attr)
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

// Register a new client
client_t *create(Display * dpy, Window win)
{
    // Take care of things like dialogs who we
    // shouldn't manage anyway
    XWindowAttributes attr;
    XGetWindowAttributes(dpy, win, &attr);

    if (attr.override_redirect)
        return NULL;

    // Also make sure this window doesn't already exist
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
void destroy(client_t * client, int danger)
{
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

    if (!danger)
        XDestroyWindow(client->dpy, client->win);

    free(client);

    updicons();
}

// Hide (iconify) a client
void hide(client_t * client)
{
    if (client->state != Visible)
        return;

    client->icon = malloc(sizeof(icon_t));

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

    XUnmapWindow(client->dpy, client->win);

    client->state = Hidden;

    updicons();
}

// Unhide (deiconify) a client
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

// Sets the current desktop and maps/unmaps all necessary windows
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

// Begin moving/resizing a client
void beginmvrsz(client_t * client)
{
    if (client->state != Visible)
        return;

    // Some apps, like stalonetray, resize themselves frequently; make sure to
    // keep up to date with respect to the client's dimensions.
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

// Stop moving/resizing a client
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

// Raise a client to the toplevel
void raise_(client_t * client)
{
    if (client->state != Visible)
        return;
    XRaiseWindow(client->dpy, client->win);
}

// Lower a client to the bottomlevel
void lower(client_t * client)
{
    if (client->state != Visible)
        return;
    XLowerWindow(client->dpy, client->win);
}

// Maximize a client (doesn't work with xrandr)
void maximize(client_t * client)
{
    client->x = 0;
    client->y = ICON_HEIGHT;
    client->w = SCREEN_WIDTH(client->dpy);
    client->h = SCREEN_HEIGHT(client->dpy) - ICON_HEIGHT;

    XMoveResizeWindow(client->dpy, client->win, client->x, client->y,
              client->w, client->h);
}

// Update all icon positions, and redraw them
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

// Paint a specific icon
void paint(client_t * client)
{
    char *title = malloc(200);
    XFetchName(client->dpy, client->win, &title);

    XClearWindow(client->dpy, client->icon->win);
    XDrawString(client->dpy, client->icon->win, client->icon->gc, 0,
            ICON_HEIGHT, (title ? title : " "),
            MIN((title ? strlen(title) : 0), 10));

    free(title);
}

/* Change focus
 *
 * The idea for this was snatched (blatantly) from 9wm, which has a really
 * neat click-to-focus implementation that's insanely easy to follow.
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
        } else {
            focused = win;
        }
    }
}
