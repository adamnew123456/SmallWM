/** @file */
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
 * Make sure that a client is mapped onto the display.
 * @param window The client window to check.
 * @return Whether the client window is visible or not.
 */
bool ClientManager::is_visible(Window window)
{
    ClientState state = get_state(window);
    return (state == CS_ACTIVE) || (state == CS_VISIBLE);
}

/**
 * Registers a group of actions with a particular X11 class.
 * @param x_class The window class to associate with
 * @param actions The ClassActions to run for that window
 */
void ClientManager::register_action(std::string x_class, ClassActions actions)
{
    m_actions[x_class] = actions;
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
 * Sets the state of the window.
 * @param window The window of the client
 * @param state The state to set
 */
void ClientManager::set_state(Window window, ClientState state)
{
    m_clients[window] = state;
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

    ClientState state = get_state(m_mvr.client);

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
 * @param window The client to transition (or the icon window)
 * @param new_state The new state to attempt to transition to.
 */
void ClientManager::state_transition(Window window, ClientState new_state)
{   
    if (!is_client(window))
    {
        return;
    }

    // Save the current window's attributes, should any transition need them
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, window, &attr);

    ClientState old_state = get_state(window);
    if (old_state == CS_ACTIVE)
    {
        if (new_state == CS_ICON)
        {
            unfocus(window);
            unmap(window);
            make_icon(window);
            return;
        }
        if (new_state == CS_VISIBLE)
        {
            unfocus(window);
            set_state(window, CS_VISIBLE);
            return;
        }
        if (new_state == CS_INVISIBLE)
        {
            unfocus(window);
            unmap(window);
            set_state(window, CS_INVISIBLE);
            return;
        }
        if (new_state == CS_MOVING)
        {
            unfocus(window);
            unmap(window);
            begin_moving(window, attr);
            return;
        }
        if (new_state == CS_RESIZING)
        {
            unfocus(window);
            unmap(window);
            begin_resizing(window, attr);
            return;
        }
        if (new_state == CS_DESTROY)
        {
            unfocus(window);
            destroy(window);
            return;
        }
    }

    if (old_state == CS_VISIBLE)
    {
        if (new_state == CS_ACTIVE)
        {
            focus(window);
            return;
        }
        if (new_state == CS_INVISIBLE)
        {
            unmap(window);
            set_state(window, CS_INVISIBLE);
            return;
        }
        if (new_state == CS_DESTROY)
        {
            destroy(window);
            return;
        }
    }

    if (old_state == CS_INVISIBLE)
    {
        if (new_state == CS_VISIBLE)
        {
            map(window);
            set_state(window, CS_VISIBLE);
            return;
        }
        if (new_state == CS_DESTROY)
        {
            destroy(window);
            return;
        }
    }

    if (old_state == CS_ICON)
    {
        Icon *icon = get_icon(window);
        // Save this, since the icon will become invalid later
        Window client = icon->client;

        if (!icon)
        {
            return;
        }

        if (new_state == CS_ACTIVE)
        {
            delete_icon(icon);
            map(client);
            focus(client);
            return;
        }
        if (new_state == CS_DESTROY)
        {
            delete_icon(icon);
            destroy(window);
            return;
        }
    }

    if (old_state == CS_MOVING)
    {
        if (new_state == CS_ACTIVE)
        {
            map(window);
            end_move_resize();
            focus(window);
            return;
        }
        if (new_state == CS_DESTROY)
        {
            end_move_resize_unsafe();
            destroy(window);
            return;
        }
    }

    if (old_state == CS_RESIZING)
    {
        if (new_state == CS_ACTIVE)
        {
            map(window);
            end_move_resize();
            focus(window);
            return;
        }
        if (new_state == CS_DESTROY)
        {
            end_move_resize_unsafe();
            destroy(window);
            return;
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

    set_state(window, CS_VISIBLE);
    m_layers[window] = is_transient ? 5 : DIALOG_LAYER;
    m_sticky[window] = false;
    m_desktops[window] = m_current_desktop;

    relayer();
    update_desktop();
    apply_actions(window);
    focus(window);
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
    XClientMessageEvent client_close;
    client_close.type = ClientMessage;
    client_close.window = window;
    client_close.message_type = XInternAtom(m_shared.display, "WM_PROTOCOLS", false);
    client_close.format = 32;
    client_close.data.l[0] = XInternAtom(m_shared.display, "WM_DELETE_WINDOW", false);
    client_close.data.l[1] = CurrentTime;

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
            state_transition(client_iter->first, CS_VISIBLE);
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
            set_state(window, CS_VISIBLE);
        }
        else
        {
            set_state(window, CS_ACTIVE);
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
 * Applies the actions in a ClassActions.
 * @param window The client window to apply the actions to.
 */
void ClientManager::apply_actions(Window window)
{

    XClassHint *classhint = XAllocClassHint();
    XGetClassHint(m_shared.display, window, classhint);

    std::string win_class;
    if (classhint->res_class)
        win_class = std::string(classhint->res_class);
    else
        win_class = "";

    ClassActions actions = m_actions[win_class];
    if (actions.actions & ACT_STICK)
        flip_sticky_flag(window);
    if (actions.actions & ACT_MAXIMIZE)
        maximize(window);
    if (actions.actions & ACT_SETLAYER)
        set_layer(window, actions.layer);
    if (actions.actions & ACT_SNAP)
        snap(window, actions.snap);

    XFree(classhint);
}

/**
 * Maps a window onto the screen, making it visible.
 * @param window The client window to show.
 */
void ClientManager::map(Window window)
{
    XMapWindow(m_shared.display, window);
    relayer();
}

/**
 * Unmaps a window off the screen.
 * @param window The client window to hide.
 */
void ClientManager::unmap(Window window)
{
    XUnmapWindow(m_shared.display, window);
}

/**
 * Snaps a window to a particular side of the screen.
 * @param window The client window to snap
 * @param side The side of the window to snap to
 */
void ClientManager::snap(Window window, SnapDir side)
{
    if (!is_client(window) || !is_visible(window))
        return;

    Dimension icon_height = DIM2D_HEIGHT(m_shared.icon_size);
    Dimension screen_width = DIM2D_WIDTH(m_shared.screen_size),
              screen_height = DIM2D_HEIGHT(m_shared.screen_size) - icon_height;

    Dimension x, y, w, h;
    switch (side)
    {
        case SNAP_TOP:
            x = 0;
            y = icon_height;
            w = screen_width;
            h = screen_height / 2;
            break;
        case SNAP_BOTTOM:
            x = 0;
            y = (screen_height / 2) + icon_height;
            w = screen_width;
            h = screen_height / 2;
            break;
        case SNAP_LEFT:
            x = 0;
            y = icon_height;
            w = screen_width / 2;
            h = screen_height;
            break;
        case SNAP_RIGHT:
            x = screen_width / 2;
            y = icon_height;
            w = screen_width / 2;
            h = screen_height;
            break;
    }

    XMoveResizeWindow(m_shared.display, window, x, y, w, h);
}

/**
 * Maximizes a window, showing only the icon bar.
 * @param window The client window to maximize
 */
void ClientManager::maximize(Window window)
{
    if (!is_client(window) || !is_visible(window))
        return;

    Dimension icon_height = DIM2D_HEIGHT(m_shared.icon_size);
    Dimension screen_width = DIM2D_WIDTH(m_shared.screen_size),
              screen_height = DIM2D_HEIGHT(m_shared.screen_size) - icon_height;

    XMoveResizeWindow(m_shared.display, window, 0, icon_height, 
            screen_width, screen_height);
}
