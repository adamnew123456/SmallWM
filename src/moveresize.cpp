/** @file */
#include "clientmanager.h"

/**
 * Gets the client window corresponding to a placeholder.
 * @param window The placeholder window
 * @return The client window, or None
 */
Window ClientManager::get_from_placeholder(Window window)
{
    if (m_mvr.window == window)
        return m_mvr.client;
    else
        return None;
}

/**
 * Creates placeholders for ClientManager::begin_moving and 
 * ClientManager::begin_resizing.
 * @param attr The attributes to give the window.
 */
void ClientManager::create_placeholder(const XWindowAttributes &attr)
{
    // The placeholder is important, because it allows us to do as little
    // drawing as possible when it is resized.
    m_mvr.window = XCreateSimpleWindow(m_shared.display, m_shared.root,
            attr.x, attr.y, attr.width, attr.height, 1,
            BlackPixel(m_shared.display, m_shared.screen),
            WhitePixel(m_shared.display, m_shared.screen));

    // Ignore this window, so it doesn't become a client itself
    XSetWindowAttributes set_attr;
    set_attr.override_redirect = true;
    XChangeWindowAttributes(m_shared.display, m_mvr.window, CWOverrideRedirect, 
            &set_attr);

    // Map this window onto the screen, bind the user's mouse to this window, and
    // then make it visible
    XMapWindow(m_shared.display, m_mvr.window);
    XGrabPointer(m_shared.display, m_mvr.window, true, 
            PointerMotionMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
            None, None, CurrentTime);
    XRaiseWindow(m_shared.display, m_mvr.window);
}

/**
 * Begins moving a client.
 * @param window The client window to start moving.
 * @param attr The attributes the client window had most recently.
 */
void ClientManager::begin_moving(Window window, const XWindowAttributes &attr)
{
    create_placeholder(attr);
    m_mvr.client = window;
    
    // Getting the pointer location is complicated and provides us with many
    // unneeded parameters. We need the location so that way, when the pointer
    // is moved, the motion handler has a location relative to measure to.
    int ptr_x, ptr_y;
    Window _u1;
    int _u2;
    unsigned int _u3;
    XQueryPointer(m_shared.display, m_shared.root, &_u1, &_u1, &ptr_x, &ptr_y,
            &_u2, &_u2, &_u3);
    
    m_mvr.ptr_loc = Dimension2D((Dimension)ptr_x, (Dimension)ptr_y);
    set_state(window, CS_MOVING);
}

/**
 * Begins resizing a client.
 * @param window The client window to start resizing.
 * @param attr The attributes the cliet window had most recently.
 */
void ClientManager::begin_resizing(Window window, const XWindowAttributes &attr)
{
    create_placeholder(attr);
    m_mvr.client = window;

    // Getting the pointer location is complicated and provides us with many
    // unneeded parameters. We need the location so that way, when the pointer
    // is moved, the motion handler has a location relative to measure to.
    int ptr_x, ptr_y;
    Window _u1;
    int _u2;
    unsigned int _u3;
    XQueryPointer(m_shared.display, m_shared.root, &_u1, &_u1, &ptr_x, &ptr_y,
            &_u2, &_u2, &_u3);
    
    m_mvr.ptr_loc = Dimension2D(ptr_x, ptr_y);
    set_state(window, CS_RESIZING);
}

/**
 * Stops moving/resizing a client, and applies the attributes to the client.
 */
void ClientManager::end_move_resize()
{
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, m_mvr.window, &attr);

    end_move_resize_unsafe();

    XMoveResizeWindow(m_shared.display, m_mvr.client, attr.x, attr.y, attr.width, attr.height);
}

/**
 * Stops moving/resizing a client, but doesn't apply the previous attributes.
 */
void ClientManager::end_move_resize_unsafe()
{
    XUngrabPointer(m_shared.display, CurrentTime);
    XDestroyWindow(m_shared.display, m_mvr.window);
    m_mvr.window = None;
}
