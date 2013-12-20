/* Routines for handling icons */
#include "icon.h"

// Creates an icon from a client, and hides the client
void to_icon(client_t *client)
{
    if (client->state != C_VISIBLE)
        return;
    
    icon_t *icon = malloc(sizeof(icon_t));
    icon->wm = client->wm;
    icon->client = client;

    // Position the window off-screen until it can be moved by the WM module
    icon->window = XCreateSimpleWindow(icon->wm->display,
            icon->wm->root,
            -200, -200, icon->wm->icon_width, icon->wm->icon_height,
            1, 
            BlackPixel(icon->wm->display, icon->wm->screen),
            WhitePixel(icon->wm->display, icon->wm->screen));

    // Communicate to the WM module that this window is not a client
    XSetWindowAttributes attr;
    attr.override_redirect = True;
    XChangeWindowAttributes(icon->wm->display, icon->window, CWOverrideRedirect, &attr);

    // Hook into the click and redraw events for this icon
    XSelectInput(icon->wm->display, icon->window,
            ButtonPressMask | ButtonReleaseMask | ExposureMask);

    XMapWindow(icon->wm->display, icon->window);
    
    // Create a graphics context for drawing the text and the icon
    icon->gc = XCreateGC(icon->wm->display, icon->window, 0, NULL);

    // Get the pixmap for the icon
    XWMHints *hints = XGetWMHints(icon->wm->display, client->window);
    if (hints && hints->flags & IconPixmapHint)
    {
        icon->has_pixmap = True;
        icon->pixmap = hints->icon_pixmap;

        // Get the dimensions of the pixmap
        Window _unused1;
        int _unused2;
        unsigned int _unused3;

        XGetGeometry(icon->wm->display, icon->pixmap, 
                &_unused1, &_unused2, &_unused2, 
                &icon->pix_width, &icon->pix_height, 
                &_unused3, &_unused3);
    }
    else
        icon->has_pixmap = False;
    XFree(hints);

    XUnmapWindow(client->wm->display, client->window);
    client->state = C_HIDDEN;

    // Remove the hidden client and add the visible icon to the WM's object tables
    del_table(icon->wm->clients, client->window);
    add_table(icon->wm->icons, icon->window, icon);

    update_icons_wm(icon->wm);
}

// Moves the window associated with a particular icon
void place_icon(icon_t *icon, unsigned int x, unsigned int y)
{
    XMoveResizeWindow(icon->wm->display, icon->window, 
        x, y, icon->wm->icon_width, icon->wm->icon_height);
}

// Paints the icon's pixmap and the icon text onto the window
void paint_icon(icon_t *icon)
{
    char *title;

    // Primary attempt - just ask the client for its preferred icon window
    XGetIconName(icon->wm->display, icon->client->window, &title);

    // Secondary attempt - ask the client for its preferred main title
    if (!title)
        XFetchName(icon->wm->display, icon->client->window, &title);

    int text_offset = (icon->has_pixmap == True ?
                        icon->pix_width :
                        0);

    XClearWindow(icon->wm->display, icon->window);

    if (title)
    {
        XDrawString(icon->wm->display, icon->window, icon->gc,
                text_offset, icon->wm->icon_height, title,
                strlen(title));
    }

    if (icon->has_pixmap)
    {
        XCopyArea(icon->wm->display, icon->pixmap, icon->window, icon->gc,
                  0, 0, icon->pix_width, icon->pix_height, 0, 0);
    }

    XFree(title);
}

// Deletes an icon and restores the client
void to_client(icon_t *icon)
{
    client_t *client = icon->client;

    // Restore the window to the screen
    XMapWindow(icon->wm->display, client->window);
    XRaiseWindow(icon->wm->display, client->window);
    add_table(icon->wm->clients, client->window, client);
    client->state = C_VISIBLE;
    client->desktop = icon->wm->current_desktop;

    // Kill the icon
    XDestroyWindow(icon->wm->display, icon->window);
    XFreeGC(icon->wm->display, icon->gc);
    del_table(icon->wm->icons, icon->window);
    free(icon);

    update_icons_wm(client->wm);
}
