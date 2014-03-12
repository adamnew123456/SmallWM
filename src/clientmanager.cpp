#include "clientmanager.h"

/**
 * Check to see if a window is a registered client window.
 * @param window The window to check.
 * @return Whether or not the window represents a client.
 */
bool ClientManager::is_client(Window window)
{
    return m_clients.count(window) > 0;
}

/**
 * Gets the icon structure which is owns the given window.
 * @param window The window to find.
 * @return Either hte owning Icon, or NULL.
 */
Icon *ClientManager::get_icon(Window window)
{
    for (std::map<Window,Icon*>::iterator icon_iter = m_icons.begin();
            icon_iter != m_icons.end();
            icon_iter++)
    {
        if (icon_iter->second->window == window)
            return icon_iter->second;
    }

    return NULL;
}

/**
 * Gets the state of a particular client.
 * @param window The window of the client.
 * @return The state of the client which owns the window.
 */
ClientState ClientManager::get_state(Window window)
{
    return m_clients[window];
}

/** 
 * Handle a motion event, which updates the client which is currently being
 * moved or resized.
 * @param event The X MotionNotify event.
 */
void ClientManager::handle_motion(const XEvent &event)
{
    if (m_mvr.client == None)
        return;

    ClientState state = m_clients[m_mvr.client];

    // Get the most recent event, to skip needless updates
    XEvent latest = event;
    while (XCheckTypedEvent(m_shared.display, MotionNotify, &latest));

    // Find out the pointer delta, relative to where it was previously
    Dimension xdiff = latest.xbutton.x_root - DIM2D_X(m_mvr.ptr_loc),
              ydiff = latest.xbutton.y_root - DIM2D_Y(m_mvr.ptr_loc);

    m_mvr.ptr_loc = Dimension2D(latest.xbutton.x_root, latest.xbutton.y_root);

    // Find out where the placeholder is now; the xdiff and ydiff are used 
    // relative to this
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, m_mvr.window, &attr);

    switch (state)
    {
        case CS_MOVING:
            XMoveWindow(m_shared.display, m_mvr.window, 
                    attr.x + xdiff, attr.y + ydiff);
            break;
        case CS_RESIZING:
        {
            Dimension width = std::max<Dimension>(attr.width + xdiff, 1),
                      height = std::max<Dimension>(attr.height + ydiff, 1);
            XResizeWindow(m_shared.display, m_mvr.window, width, height);
        }; break;
    }
}

/**
 * Transitions from one state to another, given a client window.
 * @param window The client to transition (or, possibly an icon window)
 * @param new_state The new state to attempt to transition to.
 */
void ClientManager::state_transition(Window window, ClientState new_state)
{   
    if (!is_client(window))
        return;

    // Save the current window's attributes, should any transition need them
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, window, &attr);

    ClientState old_state = m_clients[window];

    if (old_state == CS_ACTIVE)
    {
        if (new_state == CS_ICON)
        {
            unfocus(window);
            unmap(window);
            make_icon(window);
        }
        if (new_state == CS_VISIBLE)
        {
            unfocus(window);
            m_clients[window] = CS_VISIBLE;
        }
        if (new_state == CS_INVISIBLE)
        {
            unfocus(window);
            unmap(window);
            m_clients[window] = CS_INVISIBLE;
        }
        if (new_state == CS_MOVING)
        {
            unfocus(window);
            unmap(window);
            begin_moving(window, attr);
        }
        if (new_state == CS_RESIZING)
        {
            unfocus(window);
            unmap(window);
            begin_resizing(window, attr);
        }
        if (new_state == CS_DESTROY)
        {
            destroy(window);
        }
    }

    if (old_state == CS_VISIBLE)
    {
        if (new_state == CS_ACTIVE)
        {
            focus(window);
        }
        if (new_state == CS_INVISIBLE)
        {
            unmap(window);
            m_clients[window] = CS_INVISIBLE;
        }
        if (new_state == CS_DESTROY)
        {
            destroy(window);
        }
    }

    if (old_state == CS_INVISIBLE)
    {
        if (new_state == CS_VISIBLE)
        {
            map(window);
            m_clients[window] = CS_VISIBLE;
        }
        if (new_state == CS_DESTROY)
        {
            destroy(window);
        }
    }

    if (old_state == CS_ICON)
    {
        Icon *icon = get_icon(window);

        if (new_state == CS_ACTIVE)
        {
            delete_icon(icon->window);
            map(window);
            focus(window);
        }
        if (new_state == CS_DESTROY)
        {
            delete_icon(icon->window);
            destroy(window);
        }
    }

    if (old_state == CS_MOVING)
    {
        if (new_state == CS_ACTIVE)
        {
            map(window);
            end_move_resize();
            focus(window);
        }
        if (new_state == CS_DESTROY)
        {
            end_move_resize_unsafe();
            destroy(window);
        }
    }

    if (old_state == CS_RESIZING)
    {
        if (new_state == CS_ACTIVE)
        {
            map(window);
            end_move_resize();
            focus(window);
        }
        if (new_state == CS_DESTROY)
        {
            end_move_resize_unsafe();
            destroy(window);
        }
    }
}

/**
 * Creates a new client.
 * @param window The window of the new client to create.
 */
void ClientManager::create(Window window)
{
    // Ignore any non-managable windows
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, window, &attr);

    if (attr.override_redirect)
        return;

    // Transient is the ICCCM term for a window which is a dialog of
    // another client.
    bool is_transient = false;

    Window transient_for;
    if (XGetTransientForHint(m_shared.display, window, &transient_for) && 
            transient_for != None)
        is_transient = true;

    XSetWindowBorder(m_shared.display, window, 
            WhitePixel(m_shared.display, m_shared.screen));
    XSetWindowBorderWidth(m_shared.display, window, m_shared.border_width);

    focus(window);
    m_layers[window] = is_transient ? 5 : DIALOG_LAYER;
    m_sticky[window] = false;
    m_desktops[window] = m_current_desktop;

    relayer();
    update_desktop();
}

/**
 * Removes a now non-existant client from the registry.
 * @param window The now non-existant window to delete.
 */
void ClientManager::destroy(Window window)
{
    m_layers.erase(window);
    m_desktops.erase(window);
    m_sticky.erase(window);
    m_clients.erase(window);
}

/**
 * Requests that a client close, without actually closing it directly.
 * @param window The client window to close.
 */
void ClientManager::close(Window window)
{
    XEvent close_event;
    XClientMessageEvent client_close = {
        .type = ClientMessage,
        .window = window,
        .message_type = XInternAtom(m_shared.display, "WM_PROTOCOLS", false),
        .format = 32,
        .data = {
            .l = static_cast<long>
                (XInternAtom(m_shared.display, "WM_DELETE_WINDOW", false),CurrentTime)
        }
    };

    close_event.xclient = client_close;
    XSendEvent(m_shared.display, window, False, NoEventMask, &close_event);
}

/**
 * Set the focus on a particular window, unfocusing the previous window.
 * @param window The client window to focus on.
 */
void ClientManager::focus(Window window)
{
    // Find the currently focused window
    for (std::map<Window,ClientState>::iterator client_iter = m_clients.begin();
            client_iter != m_clients.end();
            client_iter++)
    {
        if (client_iter->second == CS_ACTIVE)
        {
            // Make sure to unfocus this client before moving on
            state_transition(window, CS_VISIBLE);
            break;
        }
    }

    // It is guaranteed that no window is focused now - make sure this window can
    // accept focus before transferring the focus over
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, window, &attr);
    
    if (attr.c_class != InputOnly && attr.map_state == IsViewable)
    {
        XUngrabButton(m_shared.display, AnyButton, AnyModifier, window);
        XSetInputFocus(m_shared.display, window, RevertToPointerRoot, CurrentTime);

        Window new_focus;
        int _unused;
        XGetInputFocus(m_shared.display, &new_focus, &_unused);

        // If this window isn't actually focused, then re-execute the grab
        if (new_focus != window)
        {
            unfocus(window);
            m_clients[window] = CS_VISIBLE;
        }
        else
        {
            m_clients[window] = CS_ACTIVE;
        }
    }
}

/**
 * Causes the client to lose its input focus.
 * @param window The client window to unfocus.
 */
void ClientManager::unfocus(Window window)
{
    XGrabButton(m_shared.display, AnyButton, AnyModifier, window, false,
            ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
            None, None);
}

/**
 * Maps a window onto the screen, making it visible.
 * @param window The client window to show.
 */
void ClientManager::map(Window window)
{
    XMapWindow(m_shared.display, window);
}

/**
 * Unmaps a window off the screen.
 * @param window The client window to hide.
 */
void ClientManager::unmap(Window window)
{
    XUnmapWindow(m_shared.display, window);
}
