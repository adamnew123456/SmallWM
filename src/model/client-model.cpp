/** @file */
#include "client-model.h"

/**
 * Returns whether or not a client exists.
 */
bool ClientModel::is_client(Window client)
{
    return m_desktops.is_member(client);
}

/**
 * Returns whether or not a client is visible.
 */
bool ClientModel::is_visible(Window client)
{
    desktop_ptr desktop_of = m_desktops.get_category_of(client);
    if (desktop_of->is_all_desktop())
        return true;

    if (desktop_of->is_user_desktop())
        return *desktop_of == *m_current_desktop;

    return false;
}

/**
 * Returns whether a particular desktop as a whole is visible.
 */
bool ClientModel::is_visible_desktop(desktop_ptr desktop)
{
    if (desktop->is_all_desktop())
        return true;

    if (desktop->is_user_desktop())
        return *desktop == *m_current_desktop;

    return false;
}

/**
 * Gets a list of all of the clients on a desktop.
 */
void ClientModel::get_clients_of(desktop_ptr desktop,
    std::vector<Window> &return_clients)
{
    for (client_iter iter = m_desktops.get_members_of_begin(desktop);
            iter != m_desktops.get_members_of_end(desktop);
            iter++)
        return_clients.push_back(*iter);
}

/**
 * Gets a list of all of the visible clients.
 */
void ClientModel::get_visible_clients(std::vector<Window> &return_clients)
{
    for (client_iter iter = 
                m_desktops.get_members_of_begin(m_current_desktop);
            iter != m_desktops.get_members_of_end(m_current_desktop);
            iter++)
        return_clients.push_back(*iter);

    for (client_iter iter = 
                m_desktops.get_members_of_begin(ALL_DESKTOPS);
            iter != m_desktops.get_members_of_end(ALL_DESKTOPS);
            iter++)
        return_clients.push_back(*iter);
}

/**
 * Gets all of the visible windows, but sorted by layer from bottom to
 * top.
 */
void ClientModel::get_visible_in_layer_order(std::vector<Window> &return_clients)
{
    get_visible_clients(return_clients);

    UniqueMultimapSorter<Layer, Window> layer_sorter(m_layers);
    std::sort(return_clients.begin(), return_clients.end(),
            layer_sorter);
}

/**
 * Adds a new client with some basic initial state.
 */
void ClientModel::add_client(Window client, InitialState state,
    Dimension2D location, Dimension2D size, bool autofocus)
{
    if (DIM2D_WIDTH(size) <= 0 || DIM2D_HEIGHT(size) <= 0)
        return;

    // Special care is given to honor the initial state, since it is
    // mandated by the ICCCM
    switch (state)
    {
        case IS_VISIBLE:
            m_desktops.add_member(m_current_desktop, client);
            m_changes.push(new ChangeClientDesktop(client, 0,
                        m_current_desktop));
            break;
        case IS_HIDDEN:
            m_desktops.add_member(ICON_DESKTOP, client);
            m_changes.push(new ChangeClientDesktop(client, 0, ICON_DESKTOP));
            break;
    }

    m_layers.add_member(DEF_LAYER, client);
    m_changes.push(new ChangeLayer(client, DEF_LAYER));

    // Since the size and locations are already current, don't put out
    // an event now that they're set
    m_location[client] = location;
    m_size[client] = size;
    m_cps_mode[client] = CPS_FLOATING;

    Crt *current_screen = m_crt_manager.screen_of_coord(DIM2D_X(location), DIM2D_Y(location));
    if (!current_screen)
        // An invalid screen - no monitor ever contains a negative screen
        m_screen[client] = Box(-1, -1, 0, 0);
    else
    {
        Box &screen_box = m_crt_manager.box_of_screen(current_screen);
        m_screen[client] = screen_box;
    }

    if (autofocus)
    {
        set_autofocus(client, true);
        focus(client);
    }
    else
        set_autofocus(client, false);
}

/**
 * Removes a client.
 *
 * Note that this method will put out a ChangeFocus event, but that event will
 * have a nonexistent 'prev_focus' field (pointing to the client that is
 * destroyed). Other than that event, however, no other notification will be
 * delivered that this window was removed.
 */
void ClientModel::remove_client(Window client)
{
    if (!is_client(client))
        return;

    // A destroyed window cannot be focused.
    unfocus_if_focused(client);

    // Unregister the client from any categories it may be a member of, but
    // keep a copy of each of the categories so we can pass it on to notify
    // that the window was destroyed (don't copy the size/location though,
    // since they will most likely be invalid, and of no use anyway)
    const Desktop *desktop = find_desktop(client);
    Layer layer = find_layer(client);

    m_desktops.remove_member(client);
    m_layers.remove_member(client);
    m_location.erase(client);
    m_size.erase(client);
    m_cps_mode.erase(client);
    m_screen.erase(client);
    m_autofocus.erase(client);

    m_changes.push(new DestroyChange(client, desktop, layer));
}

/**
 * This is just a way of directly sending an event to the ClientModel - we
 * don't do anything with it.
 */
void ClientModel::unmap_client(Window client)
{
    if (!is_client(client))
        return;

    m_changes.push(new UnmapChange(client));
}

/**
 * Gets  the position/scale mode of a client.
 */
ClientPosScale ClientModel::get_mode(Window client)
{
    return m_cps_mode[client];
}

/**
 * Changes the position/scale mode of a client.
 *
 * Note that this change doesn't actually update the location or the size, since
 * the ClientModel is isolated from the information necessary to make that 
 * change.
 */
void ClientModel::change_mode(Window client, ClientPosScale cps)
{
    if (m_cps_mode[client] != cps)
    {
        m_cps_mode[client] = cps;
        m_changes.push(new ChangeCPSMode(client, cps));
    }
}

/**
 * Changes the location of a client.
 */
void ClientModel::change_location(Window client, Dimension x, Dimension y)
{
    // See whether the client should end up on a new desktop with its new
    // location
    Box &old_desktop = m_screen[client];
    Crt *new_screen = m_crt_manager.screen_of_coord(x, y);
    Box &new_desktop = m_crt_manager.box_of_screen(new_screen);

    m_location[client] = Dimension2D(x, y);
    m_changes.push(new ChangeLocation(client, x, y));

    if (old_desktop != new_desktop)
        to_screen_box(client, new_desktop);
}

/**
 * Changes the size of a client.
 */
void ClientModel::change_size(Window client, Dimension width, Dimension height)
{
    if (width > 0 && height > 0)
    {
        m_size[client] = Dimension2D(width, height);
        m_changes.push(new ChangeSize(client, width, height));
    }
}

/**
 * Gets the next window in the current desktop's focus history, that is a visible.
 */
Window ClientModel::get_next_in_focus_history()
{
    UniqueStack<Window> &focus_history = m_current_desktop->focus_history;
    while (!focus_history.empty())
    {
        Window candidate = focus_history.top();
        focus_history.pop();

        if (m_desktops.is_member(candidate) &&is_visible(candidate))
            return candidate;
    }

    return None;
}

/**
 * Removes an element from the current desktop's focus history.
 *
 * This should be used when, for example, focusing a window fails.
 */
bool ClientModel::remove_from_focus_history(Window client)
{
    return m_current_desktop->focus_history.remove(client);
}

/**
 * Gets the currently focused window.
 */
Window ClientModel::get_focused()
{
    return m_focused;
}

/**
 * Returns true if a window can be automatically focused, or false otherwise.
 */
bool ClientModel::is_autofocusable(Window client)
{
    if (!is_client(client))
        return false;

    return m_autofocus[client];
}

/**
 * Either allows, or prevents, a client from being autofocused.
 */
void ClientModel::set_autofocus(Window client, bool can_autofocus)
{
    if (!is_client(client))
        return;

    m_autofocus[client] = can_autofocus;
}

/**
 * Changes the focus to another window. Note that this fails if the client
 * is not currently visible.
 *
 * This is used for automated focusing, and is subject to the rules of
 * autofocusing - in particular, if the user enables nofocus for this type of
 * window, then this will do nothing.
 */
void ClientModel::focus(Window client)
{
    if (!is_visible(client))
        return;

    if (!m_autofocus[client])
        return;

    Window old_focus = m_focused;
    m_focused = client;

    m_current_desktop->focus_history.push(client);
    m_changes.push(new ChangeFocus(old_focus, client));
}

/**
 * Forces a window to be focused, ignoring the nofocus policy.
 */
void ClientModel::force_focus(Window client)
{
    if (!is_visible(client))
        return;

    Window old_focus = m_focused;
    m_focused = client;

    // Since the focus history is used for automatic focusing, avoid
    // polluting it with windows that cannot be focused
    if (m_autofocus[client])
        m_current_desktop->focus_history.push(client);

    m_changes.push(new ChangeFocus(old_focus, client));
}

/**
 * Unfocuses a window if it is currently focused.
 */
void ClientModel::unfocus()
{
    if (m_focused != None)
    {
        Window old_focus = m_focused;
        m_focused = None;
        m_changes.push(new ChangeFocus(old_focus, None));
    }
}

/**
 * Unfocuses a window if it is currently focused, otherwise the focus is
 * not changed.
 */
void ClientModel::unfocus_if_focused(Window client)
{
    if (m_focused == client)
        unfocus();
}

/**
 * Gets the current desktop which the client inhabits.
 *
 * You should _NEVER_ free the value returned by this function.
 */
ClientModel::desktop_ptr ClientModel::find_desktop(Window client)
{
    if (m_desktops.is_member(client))
        return m_desktops.get_category_of(client);
    else
        return static_cast<ClientModel::desktop_ptr>(0);
}

/**
 * Gets the current layer which the client inhabits.
 */
Layer ClientModel::find_layer(Window client)
{
    if (m_desktops.is_member(client))
        return m_layers.get_category_of(client);
    else
        return INVALID_LAYER;
}

/**
 * Moves a client up in the layer stack.
 */
void ClientModel::up_layer(Window client)
{
    Layer old_layer = m_layers.get_category_of(client);
    if (old_layer < MAX_LAYER)
    {
        m_layers.move_member(client, old_layer + 1);
        m_changes.push(new ChangeLayer(client, old_layer + 1));
    }
}

/**
 * Moves a client up in the layer stack.
 */
void ClientModel::down_layer(Window client)
{
    Layer old_layer = m_layers.get_category_of(client);
    if (old_layer > MIN_LAYER)
    {
        m_layers.move_member(client, old_layer - 1);
        m_changes.push(new ChangeLayer(client, old_layer - 1));
    }
}

/**
 * Changes the layer of a client.
 */
void ClientModel::set_layer(Window client, Layer layer)
{
    if (!m_layers.is_category(layer))
        return;

    Layer old_layer = m_layers.get_category_of(client);
    if (old_layer != layer)
    {
        m_layers.move_member(client, layer);
        m_changes.push(new ChangeLayer(client, layer));
    }
}

/**
 * Toggles the stickiness of a client.
 */
void ClientModel::toggle_stick(Window client)
{
    if (!is_visible(client))
        return;

    desktop_ptr old_desktop = m_desktops.get_category_of(client);
    if (old_desktop->is_user_desktop())
        move_to_desktop(client, ALL_DESKTOPS, false);
    else
        move_to_desktop(client, m_current_desktop, true);
}

/**
 * Moves a client onto the current desktop.
 *
 * Note that this will not work if the client is on:
 *  - The "all" desktop
 *  - The "moving/resizing" desktops
 *  - The "icon" desktop
 */
void ClientModel::client_reset_desktop(Window client)
{
    desktop_ptr old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_user_desktop())
        return;

    move_to_desktop(client, m_current_desktop, false);
}

/**
 * Moves a client onto the next desktop.
 */
void ClientModel::client_next_desktop(Window client)
{
    desktop_ptr old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_user_desktop())
        return;

    user_desktop_ptr user_desktop = 
        dynamic_cast<user_desktop_ptr>(old_desktop);
    unsigned long long desktop_index = user_desktop->desktop;
    desktop_index  = (desktop_index + 1) % m_max_desktops;
    move_to_desktop(client, USER_DESKTOPS[desktop_index], true);
}

/**
 * Moves a client onto the previous desktop.
 */
void ClientModel::client_prev_desktop(Window client)
{
    desktop_ptr old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_user_desktop())
        return;

    user_desktop_ptr user_desktop = 
        dynamic_cast<user_desktop_ptr>(old_desktop);
    unsigned long long desktop_index = user_desktop->desktop;
    desktop_index = (desktop_index - 1 + m_max_desktops) 
        % m_max_desktops;
    move_to_desktop(client, USER_DESKTOPS[desktop_index], true);
}

/**
 * Changes the current desktop to the desktop after the current.
 */
void ClientModel::next_desktop()
{
    unsigned long long desktop_index = 
        (m_current_desktop->desktop + 1) % m_max_desktops;

    // We can't change while a window is being moved or resized
    if (m_desktops.count_members_of(MOVING_DESKTOP) > 0 ||
            m_desktops.count_members_of(RESIZING_DESKTOP) > 0)
        return;

    user_desktop_ptr old_desktop = m_current_desktop;
    m_current_desktop = USER_DESKTOPS[desktop_index];

    if (m_focused != None && !is_visible(m_focused))
        unfocus();

    m_changes.push(new ChangeCurrentDesktop(old_desktop, m_current_desktop));
}

/**
 * Changes the current desktop to the desktop before the current.
 */
void ClientModel::prev_desktop()
{
    // We have to add the maximum desktops back in, since C++ doesn't
    // guarantee what will happen with a negative modulus
    unsigned long long desktop_index = 
        (m_current_desktop->desktop - 1 + m_max_desktops) 
        % m_max_desktops;

    // We can't change while a window is being moved or resized
    if (m_desktops.count_members_of(MOVING_DESKTOP) > 0 ||
            m_desktops.count_members_of(RESIZING_DESKTOP) > 0)
        return;

    user_desktop_ptr old_desktop = m_current_desktop;
    m_current_desktop = USER_DESKTOPS[desktop_index];

    if (m_focused != None && !is_visible(m_focused))
        unfocus();

    m_changes.push(new ChangeCurrentDesktop(old_desktop, m_current_desktop));
}

/**
 * Hides the client and moves it onto the icon desktop.
 */
void ClientModel::iconify(Window client)
{
    desktop_ptr old_desktop = m_desktops.get_category_of(client);

    if (old_desktop->is_icon_desktop())
        return;
    else if (!is_visible(client))
        return;

    m_was_stuck[client] = old_desktop->is_all_desktop();

    move_to_desktop(client, ICON_DESKTOP, true);
}

/**
 * Hides the client and moves it onto the icon desktop.
 */
void ClientModel::deiconify(Window client)
{
    desktop_ptr old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_icon_desktop())
        return;

    // If the client was stuck before it was iconified, then respect that
    // when deiconifying it
    if (m_was_stuck[client])
        move_to_desktop(client, ALL_DESKTOPS, false);
    else
        move_to_desktop(client, m_current_desktop, false);

    focus(client);
}

/**
 * Starts moving a window.
 */
void ClientModel::start_moving(Window client)
{
    if (!is_visible(client))
        return;

    desktop_ptr old_desktop = m_desktops.get_category_of(client);

    // Only one window, at max, can be either moved or resized
    if (m_desktops.count_members_of(MOVING_DESKTOP) > 0 ||
            m_desktops.count_members_of(RESIZING_DESKTOP) > 0)
        return;

    change_mode(client, CPS_FLOATING);
    m_was_stuck[client] = old_desktop->is_all_desktop();
    move_to_desktop(client, MOVING_DESKTOP, true);
}

/**
 * Stops moving a window, and fixes its position.
 */
void ClientModel::stop_moving(Window client, Dimension2D location)
{
    desktop_ptr old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_moving_desktop())
        return;

    if (m_was_stuck[client])
        move_to_desktop(client, ALL_DESKTOPS, false);
    else
        move_to_desktop(client, m_current_desktop, false);

    change_location(client, DIM2D_X(location), DIM2D_Y(location));

    focus(client);
}

/**
 * Starts moving a window.
 */
void ClientModel::start_resizing(Window client)
{
    if (!is_visible(client))
        return;

    desktop_ptr old_desktop = m_desktops.get_category_of(client);

    // Only one window, at max, can be either moved or resized
    if (m_desktops.count_members_of(MOVING_DESKTOP) > 0 ||
            m_desktops.count_members_of(RESIZING_DESKTOP) > 0)
        return;

    change_mode(client, CPS_FLOATING);
    m_was_stuck[client] = old_desktop->is_all_desktop();
    move_to_desktop(client, RESIZING_DESKTOP, true);
}

/**
 * Stops resizing a window, and fixes its position.
 */
void ClientModel::stop_resizing(Window client, Dimension2D size)
{
    desktop_ptr old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_resizing_desktop())
        return;

    if (m_was_stuck[client])
        move_to_desktop(client, ALL_DESKTOPS, false);
    else
        move_to_desktop(client, m_current_desktop, false);

    change_size(client, DIM2D_WIDTH(size), DIM2D_HEIGHT(size));

    focus(client);
}

/**
 * Gets the root screen.
 */
Box &ClientModel::get_root_screen()
{
    return m_crt_manager.box_of_screen(m_crt_manager.root());
}

/**
 * Gets the bounding box of the screen that the client currently inhabits.
 */
Box &ClientModel::get_screen(Window client)
{
    return m_screen[client];
}

/**
 * Change the relative screen of a window to a neighboring screen.
 * Does nothing if no such neighboring screen exists.
 */
void ClientModel::to_relative_screen(Window client, Direction dir)
{
    Crt *current_screen = m_crt_manager.screen_of_box(m_screen[client]);
    if (!current_screen)
        return;

    Crt *target = NULL;

    switch (dir)
    {
        case DIR_TOP:
            target = current_screen->top;
            break;
        case DIR_BOTTOM:
            target = current_screen->bottom;
            break;
        case DIR_LEFT:
            target = current_screen->left;
            break;
        case DIR_RIGHT:
            target = current_screen->right;
            break;
    }

    if (!target)
        return;

    to_screen_crt(client, target);
}

/**
 * Change the screen of a window to the screen given by a particular box.
 */
void ClientModel::to_screen_box(Window client, Box box)
{
    Crt *box_screen = m_crt_manager.screen_of_box(box);
    if (!box_screen)
        return;

    to_screen_crt(client, box_screen);
}

/**
 * Changes the screen of a window to the given screen directly.
 */
void ClientModel::to_screen_crt(Window client, Crt* screen)
{
    Box &current_box = m_screen[client];
    Box &target_box = m_crt_manager.box_of_screen(screen);

    if (current_box != target_box)
    {
        m_screen[client] = target_box;
        m_changes.push(new ChangeScreen(client, target_box));
    }
}

/**
 * Updates the screen configuration, as well as the screen property of every
 * client window.
 */
void ClientModel::update_screens(std::vector<Box> &bounds)
{
    m_crt_manager.rebuild_graph(bounds);

    // Now, translate the location of every client back into its updated screen
    for (std::map<Window, Dimension2D>::iterator client_location = m_location.begin();
         client_location != m_location.end();
         client_location++)
    {
        Window client = client_location->first;
        Dimension2D &location = client_location->second;

        // Keep the old screen - if the new screen is the same, we don't want
        // to send out a change notification
        Box &old_box = m_screen[client];
        Box new_box(-1, -1, 0, 0);

        Crt *new_screen = m_crt_manager.screen_of_coord(
            DIM2D_X(location), DIM2D_Y(location));

        if (new_screen)
            new_box = m_crt_manager.box_of_screen(new_screen);

        if (new_box != old_box)
        {
            m_screen[client] = new_box;

            // Why do the ref like this? Well, if it is done as a reference
            // to new_box directly, then new_box will go out of scope and
            // our data will be thoroughly shat over. Thankfully the unit
            // tests caught this one.
            m_changes.push(new ChangeScreen(client, m_screen[client]));
        }
    }
}

/**
 * Moves a client between two desktops and fires the resulting event.
 */
void ClientModel::move_to_desktop(Window client, desktop_ptr new_desktop, 
        bool unfocus)
{
    desktop_ptr old_desktop = m_desktops.get_category_of(client);
    if (*old_desktop == *new_desktop)
        return;

    m_desktops.move_member(client, new_desktop);

    if (unfocus && !is_visible(client))
        unfocus_if_focused(client);

    m_changes.push(new ChangeClientDesktop(client, old_desktop, new_desktop));
}
