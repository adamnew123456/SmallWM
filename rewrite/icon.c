#include "icon.h"

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
    XGetIconName(icon->wm->display, icon->window, &title);

    // Secondary attempt - ask the client for its preferred main title
    if (!title)
        XFetchName(icon->wm->display, icon->window, &title);

    int text_offset = (icon->has_pixmap == True ?
                        icon->pix_width :
                        0);

    XClearWindow(icon->wm->display, icon->window);

    if (title)
    {
        int max_len = 10;
        if (strlen(title) < max_len) 
            max_len = strlen(title);

        XDrawString(icon->wm->display, icon->window, icon->gc,
                text_offset, icon->wm->icon_height, title,
                strlen(title), max_len);
    }

    // Paint the pixmap onto the upper-left of the icon window if it exists
    if (icon->has_pixmap)
    {
        XCopyArea(icon->wm->display, icon->pixmap, icon->window, icon->gc,
                  0, 0, icon->pix_width, icon->pix_height, 0, 0);
    }

    XFree(title);
}
