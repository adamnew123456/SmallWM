#include "clientmodel-events.h"

/**
 * Handles all the currently queued change events, returning when the
 * ClientModel change list is exhausted.
 */
void ClientModelEvents::handle_queued_changes()
{
    m_should_relayer = false;
    m_should_reposition_icons = false;

    for (ClientModel::change_iter change_iter = m_clients.changes_begin();
            change_iter != m_clients.changes_end();
            change_iter++)
    {
        m_change = *change_iter;

        if (m_change->is_layer_change())
            handle_layer_change();
        else if (m_change->is_focus_change())
            handle_focus_change();
        else if (m_change->is_client_desktop_change())
            handle_client_desktop_change();
        else if (m_change->is_current_desktop_change())
            handle_current_desktop_change();
        else if (m_change->is_location_change())
            handle_location_change();
        else if (m_change->is_size_change())
            handle_size_change();
    }
    m_clients.flush_changes();

    if (m_should_relayer)
        do_relayer();

    if (m_should_reposition_icons)
        reposition_icons();
}

/**
 * Sets a flag so that relayering occurs later - this avoid relayering on
 * every ChangeLayer event.
 */
void ClientModelEvents::handle_layer_change()
{
    m_should_relayer = true;
}

/**
 * Changes the focus from one window to another.
 *
 * The focus model used by SmallWM is click-to-focus, so clicks must be
 * captured on unfocused clients, while focused clients should not be captured.
 */
void ClientModelEvents::handle_focus_change()
{
    ChangeFocus const *change_event = dynamic_cast<ChangeFocus const*>(m_change);

    // First, unfocus whatever the model says is unfoucsed. Note that the
    // client which is being unfocused may not exist anymore.
    Window unfocused_client = change_event->prev_focus;
    if (m_clients.is_client(unfocused_client))
    {
        // Since this window will possibly be focused later, capture the clicks
        // going to it so we know when it needs to be focused again
        m_xdata.set_border_color(unfocused_client, X_WHITE);
        m_xdata.grab_mouse(unfocused_client);
    }

    Window focused_client = change_event->next_focus;
    if (focused_client == None)
    {
        // We can't set the border color of a nonexistent window, so just set
        // the focus to None
        m_xdata.set_input_focus(None);
    }
    else
    {
        // Since this is now focused, let the client process events in by 
        // ungrabbing the mouse and setting the keyboard focus
        if (m_xdata.set_input_focus(focused_client))
        {
            m_xdata.set_border_color(unfocused_client, X_BLACK);
            m_xdata.ungrab_mouse(focused_client);
        }
        else
        {
            // If we failed to actually do the focus, then we have to
            // update the client model to keep it in sync with what our idea
            // of the focus is
            m_clients.unfocus();
        }
    }
}

/**
 * This changes the desktop of a client whose desktop should be changed.
 *
 * This method consists mostly of dispatches to other methods which handle
 * each case. These cases are:
 *
 *   UserDesktop -> UserDesktop
 *   UserDesktop -> AllDesktops
 *   UserDesktop -> IconDesktop
 *   UserDesktop -> MovingDesktop
 *   UserDesktop -> ResizingDesktop
 *
 *   AllDesktops -> UserDesktop
 *   AllDesktops -> IconDesktop
 *   AllDesktops -> MovingDesktop
 *   AllDesktops -> ResizingDesktop
 *
 *   IconDesktop -> UserDesktop
 *
 *   MovingDesktop -> UserDesktop
 *
 *   ResizingDesktop -> UserDesktop
 */
void ClientModelEvents::handle_client_desktop_change()
{
    ChangeClientDesktop const *change =
        dynamic_cast<ChangeClientDesktop const*>(m_change);

    const Desktop *old_desktop = change->prev_desktop;
    const Desktop *new_desktop = change->next_desktop;
    Window client = change->window;
    
    // The previous desktop can be NULL if this client has been freshly mapped
    if (!old_desktop)
        handle_new_client_desktop_change(new_desktop, client);
    else if (old_desktop->is_user_desktop())
        handle_client_change_from_user_desktop(old_desktop, new_desktop,
                                               client);
    else if (old_desktop->is_all_desktop())
        handle_client_change_from_all_desktop(old_desktop, new_desktop,
                                              client);
    else if (old_desktop->is_icon_desktop())
        handle_client_change_from_icon_desktop(old_desktop, new_desktop,
                                               client);
    else if (old_desktop->is_moving_desktop())
        handle_client_change_from_moving_desktop(old_desktop, new_desktop,
                                                 client);
    else if (old_desktop->is_resizing_desktop())
        handle_client_change_from_resizing_desktop(old_desktop, new_desktop,
                                                   client);
    else
        m_logger.set_priority(LOG_WARNING) <<
            "Unanticipated switch by " << client << " from " <<
            old_desktop  << " to " << new_desktop << SysLog::endl;
}

/**
 * Sets the desktop of a newly created client.
 *
 * In this state, the only possibilities are either a UserDesktop or an
 * IconDesktop if the window starts out minimized.
 */
void ClientModelEvents::handle_new_client_desktop_change(const Desktop *new_desktop,
                                                         Window client)
{
    if (new_desktop->is_user_desktop())
    {
        bool will_be_visible = m_clients.is_visible_desktop(new_desktop);
        if (will_be_visible)
        {
            m_should_relayer = true;
            m_clients.focus(client);
        }
    }
    else if (new_desktop->is_icon_desktop())
        register_new_icon(client, true);
    else
    {
        m_logger.set_priority(LOG_WARNING) <<
            "New client " << client << " asked to start on desktop " <<
            new_desktop << "- making an icon instead"
            << SysLog::endl;

        // Since the API doesn't allow us to get the current desktop, and
        // we can't reset it since `ClientModel::client_reset_desktop` requires
        // that the client have a previous desktop, just make it an icon.
        handle_new_client_desktop_change(m_clients.ICON_DESKTOP, client);
    }
}

/**
 * Changes the desktop of a client from a user desktop to some other kind of
 * desktop.
 *
 * Windows can generally move from user desktops to any other kind of desktop,
 * since user desktops are the starting point for every window.
 */
void ClientModelEvents::handle_client_change_from_user_desktop(
                        const Desktop *old_desktop,
                        const Desktop *new_desktop,
                        Window client)
{
    if (new_desktop->is_user_desktop())
    {
        bool is_currently_visible = m_clients.is_visible_desktop(old_desktop);
        bool will_be_visible = m_clients.is_visible_desktop(new_desktop);

        if (is_currently_visible && !will_be_visible)
        {
            m_clients.unfocus_if_focused(client);
            m_xdata.unmap_win(client);
        }
        else if (!is_currently_visible && will_be_visible)
        {
            m_xdata.map_win(client);
            m_clients.focus(client);
        }
        else if (!is_currently_visible && !will_be_visible)
        {
            // Do nothing here - the client will still be invisible and
            // thus will not alter the focus
        }
        else
            m_logger.set_priority(LOG_WARNING) <<
                "If client is switched from a " << old_desktop << " to "
                << new_desktop << " then it cannot be visible in both places."
                << SysLog::endl;
            /*
             * This is because there is only ever one visible desktop - to
             * have a window be visible on more than one desktop would
             * somehow break that invariant.
             */
    }
    else if (new_desktop->is_all_desktop())
    {
        bool is_visible = m_clients.is_visible_desktop(old_desktop);

        if (!is_visible)
        {
            m_xdata.map_win(client);
            m_clients.focus(client);
        }
    } 
    else if (new_desktop->is_icon_desktop())
    {
        bool is_visible = m_clients.is_visible_desktop(old_desktop);
        register_new_icon(client, is_visible);
    }
    else if (new_desktop->is_moving_desktop())
        start_moving(client);
    else if (new_desktop->is_resizing_desktop())
        start_resizing(client);
}

/**
 * Changes the desktop of a client from the 'all' desktop to some other kind
 * of desktop.
 *
 * Most of this code is the same as in the user desktop changes, since
 * windows from the 'all' desktop can generally move to any other kind of
 * desktop as well.
 */
void ClientModelEvents::handle_client_change_from_all_desktop(
                        const Desktop *old_desktop,
                        const Desktop *new_desktop,
                        Window client)
{
    if (new_desktop->is_user_desktop())
    {
        bool will_be_visible = m_clients.is_visible_desktop(new_desktop);

        if (!will_be_visible)
        {
            m_clients.unfocus_if_focused(client);
            m_xdata.unmap_win(client);
        }
    }
    else if (new_desktop->is_icon_desktop())
        register_new_icon(client, true);
    else if (new_desktop->is_moving_desktop())
        start_moving(client);
    else if (new_desktop->is_resizing_desktop())
        start_resizing(client);
}

/**
 * Changes the desktop of a client from the icon desktop to some other kind of
 * desktop. In fact, the only target desktop that this method will do accept
 * is a user desktop.
 */
void ClientModelEvents::handle_client_change_from_icon_desktop(
                        const Desktop *old_desktop,
                        const Desktop *new_desktop,
                        Window client)
{
    if (new_desktop->is_user_desktop())
    {
        // Get the relevant icon information, and destroy it
        Icon *icon = m_xmodel.find_icon_from_client(client);

        if (!icon)
            m_logger.set_priority(LOG_ERR) <<
                "Tried to de-iconify a client (" << client << ") "
                "that is not currently iconified." << SysLog::endl;
        else
        {
            m_xdata.unmap_win(icon->icon);
            delete icon->gc;
            delete icon;

            m_xdata.map_win(client);
            m_should_reposition_icons = true;
        }
    }
}

/**
 * This changes the currently visible desktop, which involves figuring out
 * which windows are visible on the current desktop, which are not, and then
 * showing those that are visible and hiding those that are not.
 */
void ClientModelEvents::handle_current_desktop_change()
{
    ChangeCurrentDesktop const *change = 
        dynamic_cast<ChangeCurrentDesktop const*>(m_change);

    std::vector<Window> old_desktop_list;
    std::vector<Window> new_desktop_list;

    // The output size will have, at most, the size of its largest input
    size_t max_output_size = std::max(old_desktop_list.size(), 
        new_desktop_list.size());

    // std::set_difference requires sorted inputs
    std::sort(old_desktop_list.begin(), old_desktop_list.end());
    std::sort(new_desktop_list.begin(), new_desktop_list.end());

    std::vector<Window> to_make_invisible;
    std::vector<Window> to_make_visible;
   
    // Fill out all of the output vectors to the length of the input iterators,
    // so that std::set_difference has somewhere to put the data
    to_make_invisible.resize(max_output_size, None);
    to_make_visible.resize(max_output_size, None);

    // The old -> new set difference will produce the list of windows
    // which are on the old desktop, but not this one - these need to be
    // hidden
    std::set_difference(
        old_desktop_list.begin(), old_desktop_list.end(),
        new_desktop_list.begin(), new_desktop_list.end(),
        to_make_invisible.begin());

    for (std::vector<Window>::iterator to_hide = to_make_invisible.begin();
            /* The extra check that the iterator is not None is necessary,
             * since the vector was padded with None when it was resized.
             */
            to_hide != to_make_invisible.end() && *to_hide != None; 
            to_hide++)
        m_xdata.unmap_win(*to_hide);

    // The new -> old set difference will produce the list of windows which
    // are on the new desktop, but not on the old - these need to be made
    // visible
    std::set_difference(
        new_desktop_list.begin(), new_desktop_list.end(),
        old_desktop_list.begin(), old_desktop_list.end(),
        to_make_invisible.begin());

    for (std::vector<Window>::iterator to_show = to_make_visible.begin();
            to_show != to_make_visible.end() && *to_show != None;
            to_show++)
        m_xdata.map_win(*to_show);

    // Since we've made some windows visible and some others invisible, we've
    // invalidated the previous stacking order, so restack everything according
    // to what is now visible
    m_should_relayer = true;
}

/**
 * Handles a change in location for a particular window.
 */
void ClientModelEvents::handle_location_change()
{
    ChangeLocation const *change = 
        dynamic_cast<ChangeLocation const*>(m_change);

    m_xdata.move_window(change->window, change->x, change->y);
}

/**
 * Handles a change in size for a particular window.
 */
void ClientModelEvents::handle_size_change()
{
    ChangeSize const *change = 
        dynamic_cast<ChangeSize const*>(m_change);

    m_xdata.resize_window(change->window, change->w, change->h);
}

/**
 * Iconifies a client window, creating and registering a new icon while
 * hiding the client.
 *
 * @param client The client to create the icon for.
 * @param do_unmap `true` to unmap the client, `false` to not unmap it.
 *                 Useful if the client is already unmapped for some reason.
 */
void ClientModelEvents::register_new_icon(Window client, bool do_unmap)
{
    Window icon_window = m_xdata.create_window(true);
    m_xdata.resize_window(icon_window, m_config.icon_width, 
                          m_config.icon_height);
    m_xdata.map_win(icon_window);

    XGC *gc = m_xdata.create_gc(icon_window);
    Icon *the_icon = new Icon(client, icon_window, gc);

    m_clients.unfocus_if_focused(client);
    if (do_unmap)
        m_xdata.unmap_win(client);

    m_xmodel.register_icon(the_icon);
}

/**
 * Creates and configures a placeholder window, used for moving/resizing a
 * client.
 *
 * @param client The client to create the placeholder for.
 * @return The placeholder window.
 */
Window ClientModelEvents::create_placeholder(Window client)
{
    XWindowAttributes client_attrs;
    m_xdata.get_attributes(client, client_attrs);

    // The placeholder should be ignored (create_window(true)) because it
    // is not an actual client, but an internal window that doesn't need
    // to be managed
    Window placeholder = m_xdata.create_window(true);
    m_xdata.move_window(client, client_attrs.x, client_attrs.y);
    m_xdata.resize_window(client, client_attrs.width, client_attrs.height);

    // With the window in place, show it and make sure that the cursor is
    // glued to it, to make sure that all of the movements are captured
    m_xdata.map_win(placeholder);
    m_xdata.confine_pointer(placeholder);

    // Since we need the placeholder to move up, go ahead and schedule a
    // relayering
    m_should_relayer = true;

    return placeholder;
}

/**
 * Handles the necessary work to start moving a client.
 *
 * @param client The client window to start moving.
 */
void ClientModelEvents::start_moving(Window client)
{
    Window placeholder = create_placeholder(client);

    // The placeholder needed the client's position and size - now that the
    // placeholder is open, we can hide the client
    m_clients.unfocus_if_focused(client);
    m_xdata.unmap_win(client);

    Dimension pointer_x, pointer_y;
    m_xdata.get_pointer_location(pointer_x, pointer_y);

    m_xmodel.enter_move(client, placeholder, Dimension2D(pointer_x,
                                                         pointer_y));
}

/**
 * Handles the necessary work to start resizing a client.
 *
 * @param client The client window to start resizing.
 */
void ClientModelEvents::start_resizing(Window client)
{
    Window placeholder = create_placeholder(client);

    // The placeholder needed the client's position and size - now that the
    // placeholder is open, we can hide the client
    m_clients.unfocus_if_focused(client);
    m_xdata.unmap_win(client);

    Dimension pointer_x, pointer_y;
    m_xdata.get_pointer_location(pointer_x, pointer_y);

    m_xmodel.enter_resize(client, placeholder, Dimension2D(pointer_x, 
                                                           pointer_y));
}

/**
 * Actually does the relayering.
 *
 * This involves sorting the clients, and then sticking the icons and 
 * move/resize placeholder on the top.
 */
void ClientModelEvents::do_relayer()
{
    std::vector<Window> ordered_windows;
    m_clients.get_visible_in_layer_order(ordered_windows);

    // Figure out the currently focused client, and where it's at. We'll need
    // this information in order to place it above its peers.
    Window focused_client = m_clients.get_focused();
    Layer focused_layer;
    if (focused_client != None)
        focused_layer = m_clients.find_layer(focused_client);

    for (std::vector<Window>::iterator client_iter = ordered_windows.begin();
            client_iter != ordered_windows.end();
            client_iter++)
    {
        Window current_client = *client_iter;
        Layer current_layer = m_clients.find_layer(current_client);

        // We have to check if we're at the point where we can put up the 
        // focused window - this happens when we've passed the layer that the 
        // focused window is on. We want to put the focused window above all of
        // its peers, so before putting up the first client on the next layer,
        // put up the focused window
        if (focused_client != None &&
            current_layer == focused_layer + 1)
        {
            m_xdata.raise(focused_client);

            // Make sure to erase the focused client, so that we don't raise
            // it more than once
            focused_client = None;
        }

        if (current_client != focused_client)
            m_xdata.raise(current_client);
    }

    // If we haven't cleared the focused window, then we need to raise it before
    // moving on
    if (focused_client != None)
        m_xdata.raise(focused_client);

    // Now, raise all the icons since they should always be above all other
    // windows so they aren't obscured
    std::vector<Icon*> icon_list;
    m_xmodel.get_icons(icon_list);

    for (std::vector<Icon*>::iterator icon = icon_list.begin();
            icon != icon_list.end();
            icon++)
    {
        Window icon_win = (*icon)->icon;
        m_xdata.raise(icon_win);
    }

    // Don't obscure the placeholder, since the user is actively working with it
    Window placeholder_win = m_xmodel.get_move_resize_placeholder();
    if (placeholder_win != None)
        m_xdata.raise(placeholder_win);
}
