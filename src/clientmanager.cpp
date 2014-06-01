/** @file */
#include "clientmanager.h"

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
 * Adjusts the layer of everything - first the clients, then the icons, and
 * finally the currently moving window
 */
void ClientManager::relayer()
{
    relayer_clients();

    // Now, go back and put all of the icons on the top
    for (ClientIconIterator icon_iter = m_icons.begin();
            icon_iter != m_icons.end();
            icon_iter++)
    {
        if (!icon_iter->second)
            continue;

        XRaiseWindow(m_shared.display, icon_iter->first);
    }

    // Finally, put the placeholder on the top, if there is one
    if (m_mvr.window != None)
        XRaiseWindow(m_shared.display, m_mvr.window);

    // We just generated a lot of ConfigureNotify events. We need to get rid of
    // them so that we don't trigger anything recursive
    XEvent _;
    while (XCheckTypedEvent(m_shared.display, ConfigureNotify, &_));
}

/**
 * A wrapper around DesktopManager::update_desktop which does relayering.
 */
void ClientManager::redesktop()
{
    update_desktop();
    relayer();
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
        {
            XMoveWindow(m_shared.display, m_mvr.window, 
                    attr.x + xdiff, attr.y + ydiff);
        } break;
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
 * @param window The client to transition.
 * @param new_state The new state to attempt to transition to.
 */

void ClientManager::state_transition(Window window, ClientState new_state)
{   
    if (!is_client(window))
        return;

    ClientState old_state = get_state(window);
    StateChange state_change = m_state_changers[old_state];
    bool successful = (this->*state_change)(new_state, window);
    
    if (successful && new_state == CS_DESTROY)
        destroy(window);
    else if (successful)
        set_state(window, new_state);
}

/**
 * Handles the transition away from the ACTIVE state.
 */
bool ClientManager::from_active_state(ClientState &new_state, Window client)
{
    XWindowAttributes attr;

    switch (new_state)
    {
    case CS_ICON:
        unfocus();
        unmap(client);
        make_icon(client);
        return true;
    case CS_VISIBLE:
        unfocus();
        return true;
    case CS_INVISIBLE:
        unfocus();
        unmap(client);
        return true;
    case CS_MOVING:
        XGetWindowAttributes(m_shared.display, client, &attr);

        unfocus();
        unmap(client);
        begin_moving(client, attr);
        return true;
    case CS_RESIZING:
        XGetWindowAttributes(m_shared.display, client, &attr);

        unfocus();
        unmap(client);
        begin_resizing(client, attr);
        return true;
    case CS_WITHDRAWN:
        unfocus();
        unmap(client);
        return true;
    case CS_DESTROY:
        unfocus();
        return true;
    }

    return false;
}

/**
 * Handles the transition away from the VISIBLE state.
 */
bool ClientManager::from_visible_state(ClientState &new_state, Window client)
{
    XWindowAttributes attr;

    switch (new_state)
    {
    case CS_ICON:
        unmap(client);
        make_icon(client);
        return true;
    case CS_ACTIVE:
        return focus(client);
    case CS_INVISIBLE:
        unmap(client);
        return true;
    case CS_MOVING:
        XGetWindowAttributes(m_shared.display, client, &attr);

        unmap(client);
        begin_moving(client, attr);
        return true;
    case CS_RESIZING:
        XGetWindowAttributes(m_shared.display, client, &attr);

        unmap(client);
        begin_resizing(client, attr);
        return true;
    case CS_WITHDRAWN:
        unmap(client);
        return true;
    case CS_DESTROY:
        return true;
    }

    return false;
}

/**
 * Handles the transition away from the INVISIBLE state.
 */
bool ClientManager::from_invisible_state(ClientState &new_state, Window client)
{
    switch (new_state)
    {
    case CS_VISIBLE:
        XMapWindow(m_shared.display, client);
        // Don't explicitly relayer, since only desktop changes will trigger
        // this transition, and the desktop change function does the
        // relayering itself.
        return true;
    case CS_DESTROY:
        return true;
    }

    return false;
}

/**
 * Handles the transition away from the WITHDRAWN state.
 */
bool ClientManager::from_withdrawn_state(ClientState &new_state, Window client)
{
    switch (new_state)
    {
    case CS_VISIBLE:
        XMapWindow(m_shared.display, client);
        relayer();
        return true;
    case CS_INVISIBLE:
        // It was already hidden anyway - nothing to do, really
        return true;
    case CS_DESTROY:
        return true;
    }

    return false;
}

/**
 * Handles the transition away from the ICON state.
 */
bool ClientManager::from_icon_state(ClientState &new_state, Window client)
{
    // state_transition is required to take a client client, so the icon
    // also needs to be retrieved in order to destroy it.
    Icon *icon = get_icon_of_client(client);
    if (!icon)
        return false;

    switch (new_state)
    {
    case CS_ACTIVE:
        delete_icon(icon);
        XMapWindow(m_shared.display, client);
        reset_desktop(client);
        if (!focus(client))
            new_state = CS_VISIBLE;

        relayer(); // After focus(), since focus() may change the layer
        return true;
    case CS_WITHDRAWN:
        delete_icon(icon);
        reset_desktop(client);
        return true;
    case CS_DESTROY:
        delete_icon(icon);
        return true;
    }

    return false;
}

/**
 * Handles the transition away from the MOVING state.
 */
bool ClientManager::from_moving_state(ClientState &new_state, Window client)
{
    switch (new_state)
    {
    case CS_ACTIVE:
        XMapWindow(m_shared.display, client);
        end_move_resize();
        if (!focus(client))
            new_state = CS_VISIBLE;

        relayer(); // After focus(), since focus() may change the layer
        return true;
    case CS_WITHDRAWN:
        end_move_resize_unsafe();
        return true;
    case CS_DESTROY:
        end_move_resize_unsafe();
        return true;
    }

    return false;
}

/**
 * Handles the transition away from the RESIZING state.
 */
bool ClientManager::from_resizing_state(ClientState &new_state, Window client)
{
    switch (new_state)
    {
    case CS_ACTIVE:
        XMapWindow(m_shared.display, client);
        end_move_resize();
        if (!focus(client))
            new_state = CS_VISIBLE;

        relayer(); // After focus(), since focus() may change the layer
        return false;
    case CS_WITHDRAWN:
        end_move_resize_unsafe();
        return true;
    case CS_DESTROY:
        end_move_resize_unsafe();
        return true;
    }

    return false;
}

/**
 * Creates a new client.
 * @param window The window of the new client to create.
 */
void ClientManager::create(Window window)
{
    // Ignore any windows which cannot be managed
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, window, &attr);

    if (attr.override_redirect)
        return;

    /* So, there's an interesting wrinkle here, since support for CS_WITHDRAWN
     * was added.
     * 
     * As it turns out, a window can map itself while it is withdrawn to induce
     * a change from the Withdrawn state to the Normal state. We have to make
     * sure that, if the client already exists, that it gets taken out of its
     * CS_WITHDRAWN state and put into the proper visible/invisible state.
     *
     * Other states are irrelevant, since there were no issues with just a plain
     * 'return' here earlier.
     */
    if (is_client(window))
    {
        ClientState state = get_state(window);
        if (state != CS_WITHDRAWN)
            return;

        if (should_be_visible(window))
            set_state(window, CS_VISIBLE);
        else
            set_state(window, CS_INVISIBLE);

        return;
    }

    // Track ConfigureNotify events, so that way client-initiated relayering
    // can be undone
    XSelectInput(m_shared.display, window, StructureNotifyMask);

    // Transient is the ICCCM term for a window which is a dialog of
    // another client
    bool is_transient = false;

    Window transient_for;
    if (XGetTransientForHint(m_shared.display, window, &transient_for) && 
            transient_for != None)
        is_transient = true;

    XSetWindowBorderWidth(m_shared.display, window, m_shared.border_width);

    set_state(window, CS_VISIBLE);
    set_layer(window, is_transient ? DIALOG_LAYER : DEF_LAYER);
    add_desktop(window);

    // Okay, now that we've added everything that this client needs to be used,
    // figure out its preferences and do what it wants
    XWMHints *hints = XGetWMHints(m_shared.display, window);
    if (hints && hints->flags & StateHint)
    {
        switch (hints->initial_state)
        {
            case NormalState:
                goto do_normal;
            case WithdrawnState:
                state_transition(window, CS_WITHDRAWN);
                return;
            case IconicState:
                state_transition(window, CS_ICON);
                return;
        }
    }
    else
    {
        // If the client didn't say, assume it just wants to be shown
        goto do_normal;
    }

    // Using a GOTO here since I didn't want to duplicate this in the switch
    // and in the else branch.
do_normal:
    redesktop();
    apply_actions(window);
    focus(window);

    // If anything modifications to the new client generated ConfigureNotify
    // events, then make sure to get rid of them
    XEvent _;
    while (XCheckTypedEvent(m_shared.display, ConfigureNotify, &_));
    return;
}

/**
 * Removes a now non-existent client from the registry.
 * @param window The now non-existent window to delete.
 */
void ClientManager::destroy(Window window)
{
    delete_layer(window);
    delete_desktop(window);
    delete_state(window);
    remove_from_focus_history(window);
}

/**
 * Requests that a client close, without actually closing it directly.
 * @param window The client window to close.
 */
void ClientManager::close(Window window)
{
    // Although it would be nice to use C style constructors, GNU C++ doesn't
    // allow me to compile them when they are nested
    XEvent close_event;
    XClientMessageEvent client_close;
    client_close.type = ClientMessage;
    client_close.window = window;
    client_close.message_type = m_shared.atoms["WM_PROTOCOLS"];
    client_close.format = 32;
    client_close.data.l[0] = m_shared.atoms["WM_DELETE_WINDOW"];
    client_close.data.l[1] = CurrentTime;

    close_event.xclient = client_close;
    XSendEvent(m_shared.display, window, False, NoEventMask, &close_event);
}

/**
 * Set the focus on a particular window, unfocusing the previous window.
 * @param window The client window to focus on.
 */
bool ClientManager::focus(Window window)
{
    // Save this, since it will need to be put on the history if the focus
    // succeeds.
    Window old_focus = m_current_focus;

    if (m_current_focus != None)
    {
        m_revert_focus = False;
        state_transition(m_current_focus, CS_VISIBLE);
        m_revert_focus = True;
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
            // Since the unfocus() routine checks the current focus, make sure to
            // update the current focus to what it actually is
            m_current_focus = new_focus;
            unfocus();
            return false;
        }
        else
        {
            XSetWindowBorder(m_shared.display, window, 
                    BlackPixel(m_shared.display, m_shared.screen));

            adjust_layer(window, FOCUS_SHIFT);

            set_state(window, CS_ACTIVE);
            m_current_focus = window;

            // If this window is already in the focus history, then remove it to
            // avoid cycles in the focus history.
            remove_from_focus_history(window);

            if (old_focus != None)
                m_focus_history.push_back(old_focus);

            return true;
        }
    }
}

/**
 * Causes the client to lose its input focus.
 * @param window The client window to unfocus.
 */
void ClientManager::unfocus()
{
    if (m_current_focus != None)
    {
        XSetWindowBorder(m_shared.display, m_current_focus, 
                WhitePixel(m_shared.display, m_shared.screen));
        XGrabButton(m_shared.display, AnyButton, AnyModifier, m_current_focus, false,
                ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
                None, None);

        adjust_layer(m_current_focus, -FOCUS_SHIFT);
        
        m_current_focus = None;
    }

    // If there is a window which can accept the focus, then give the focus to it
    if (m_revert_focus && !m_focus_history.empty())
    {
        Window prev_focus = m_focus_history.back();
        m_focus_history.pop_back();
        focus(prev_focus);
    }
}

/**
 * Removes a window from the focus history.
 * @param window The window to change the focus to.
 * @note This should be used when the window is unmapped, so that the focus doesn't
 * go to invisible windows.
 */
void ClientManager::remove_from_focus_history(Window window)
{
    for (WindowIterator focus_chain = m_focus_history.begin();
            focus_chain != m_focus_history.end();
            focus_chain++)
    {
        if (*focus_chain == window)
        {
            m_focus_history.erase(focus_chain);
            break;
        }
    }
}

/**
 * Unmaps a window, and removes it from the focus history.
 * @param window The window to unmap.
 */
void ClientManager::unmap(Window window)
{
    remove_from_focus_history(window);
    XUnmapWindow(m_shared.display, window);
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
