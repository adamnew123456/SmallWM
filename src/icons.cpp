/** @file */
#include "clientmanager.h"

/**
 * Creates a new icon window, and then reorganizes all the icons.
 * @param window The client window to make an icon for.
 */
void ClientManager::make_icon(Window window)
{
    XUnmapWindow(m_shared.display, window);

    Icon *icon = new Icon;
    icon->window = XCreateSimpleWindow(m_shared.display, m_shared.root,
            -200, -200, DIM2D_WIDTH(m_shared.icon_size), 
            DIM2D_HEIGHT(m_shared.icon_size), 1,
            BlackPixel(m_shared.display, m_shared.screen),
            WhitePixel(m_shared.display, m_shared.screen));
    icon->client = window;

    // Ensure that this icon is not treated as a client
    XSetWindowAttributes attr;
    attr.override_redirect = true;
    XChangeWindowAttributes(m_shared.display, icon->window, CWOverrideRedirect,
            &attr);

    XSelectInput(m_shared.display, icon->window, 
            ButtonPressMask | ButtonReleaseMask | ExposureMask);
    XMapWindow(m_shared.display, icon->window);

    icon->gc = XCreateGC(m_shared.display, icon->window, 0, NULL);

    // Gets the pixmap for this icon, if one actually exists, as well as its
    // dimensions
    XWMHints *hints = XGetWMHints(m_shared.display, icon->client);
    if (hints && hints->flags & IconPixmapHint)
    {
        icon->has_pixmap = true;
        icon->pixmap = hints->icon_pixmap;

        Window _u1;
        int _u2;
        unsigned int _u3;

        Dimension pix_width, pix_height;
        XGetGeometry(m_shared.display, icon->pixmap, &_u1, &_u2, &_u2,
                &pix_width, &pix_height, &_u3, &_u3);
        icon->pixmap_size = Dimension2D(pix_width, pix_height);
    }
    else
    {
        icon->has_pixmap = false;
    }
    XFree(hints);

    m_clients[window] = CS_ICON;
    m_icons[window] = icon;

    // Move all the icons into the proper location
    reflow_icons();
}

/**
 * Repositions icon windows, to avoid gaps and to make all icons visible.
 */
void ClientManager::reflow_icons()
{
    Dimension x = 0, y = 0;
    Dimension icon_width = DIM2D_WIDTH(m_shared.icon_size),
              icon_height = DIM2D_HEIGHT(m_shared.icon_size),
              screen_width = DIM2D_WIDTH(m_shared.screen_size);

    for (std::map<Window,Icon*>::iterator icon_iter = m_icons.begin();
            icon_iter != m_icons.end();
            icon_iter++)
    {
        if (x + icon_width > screen_width)
        {
            x = 0;
            y += icon_height;
        }

        XMoveWindow(m_shared.display, icon_iter->second->window, x, y);
        x += icon_width;
    }
}

/**
 * Removes an icon from the icon row and reflows all the icons.
 * @param window The window representing the icon itself
 */
void ClientManager::delete_icon(Window window)
{
    Icon *icon = m_icons[window];
    XDestroyWindow(m_shared.display, window);
    XFreeGC(m_shared.display, icon->gc);
    m_icons.erase(window);
    free(icon);

    reflow_icons();
}
