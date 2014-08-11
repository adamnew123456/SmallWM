#include "clientmodel-events.h"

/**
 * Handles all the currently queued change events, returning when the
 * ClientModel change list is exhausted.
 */
void ClientModelEvents::handle_queued_changes()
{
    m_should_relayer = false;

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

    if (m_should_relayer)
        do_relayer();
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
    do_relayer();
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

    // Now, raise all the clients since they should always be above all other
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

    // Finally, raise the placeholder, if there is one
    Window placeholder_win = m_xmodel.get_move_resize_placeholder();
    if (placeholder_win != None)
        m_xdata.raise(placeholder_win);
}
