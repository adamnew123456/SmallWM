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
    Desktop* desktop_of = m_desktops.get_category_of(client);
    if (desktop_of->is_all_desktop())
        return true;

    if (desktop_of->is_user_desktop())
        return *desktop_of == *m_current_desktop;

    return false;
}

/**
 * Returns whether a particular desktop as a whole is visible.
 */
bool ClientModel::is_visible_desktop(Desktop* desktop)
{
    if (desktop->is_all_desktop())
        return true;

    if (desktop->is_user_desktop())
        return *desktop == *m_current_desktop;

    return false;
}

/**
 * Returns whether a particular window is a child of a client.
 */
bool ClientModel::is_child(Window child)
{
    return m_parents.count(child) > 0;
}

/**
 * Gets a list of all of the clients on a desktop.
 */
void ClientModel::get_clients_of(Desktop *desktop, std::vector<Window> &return_clients)
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
    for (client_iter iter = m_desktops.get_members_of_begin(m_current_desktop);
            iter != m_desktops.get_members_of_end(m_current_desktop);
            iter++)
        return_clients.push_back(*iter);

    for (client_iter iter = m_desktops.get_members_of_begin(ALL_DESKTOPS);
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
 * Gets the parent of a child window, or None if there is no such child.
 */
Window ClientModel::get_parent_of(Window child)
{
    if (!is_child(child))
        return None;

    return m_parents[child];
}

/**
 * Gets the children of a client.
 */
void ClientModel::get_children_of(Window client,
                                  std::vector<Window> &return_children)
{
    if (!is_client(client))
        return;

    for (std::set<Window>::iterator child = m_children[client]->begin();
         child != m_children[client]->end();
         child++)
    {
        return_children.push_back(*child);
    }
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
    {
        // No monitor ever contains a negative screen
        const Box invalid_box(-1, -1, 0, 0);
        m_screen.insert(std::pair<Window, const Box>(client, invalid_box));
    }
    else
    {
        const Box &screen_box = m_crt_manager.box_of_screen(current_screen);
        m_screen.insert(std::pair<Window, const Box>(client, screen_box));
    }

    if (autofocus)
    {
        m_current_desktop->focus_cycle.add(client);

        set_autofocus(client, true);
        focus(client);
    }
    else
        set_autofocus(client, false);

    m_children[client] = new std::set<Window>();
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

    // Unregister the client from any categories it may be a member of, but
    // keep a copy of each of the categories so we can pass it on to notify
    // that the window was destroyed (don't copy the size/location though,
    // since they will most likely be invalid, and of no use anyway)
    Desktop *desktop = find_desktop(client);
    Layer layer = find_layer(client);

    if (desktop->is_user_desktop())
    {
        UserDesktop *user_desktop = dynamic_cast<UserDesktop*>(desktop);
        user_desktop->focus_cycle.remove(client, true);
        sync_focus_to_cycle();
    }
    else if (desktop->is_all_desktop())
    {
        dynamic_cast<AllDesktops*>(ALL_DESKTOPS)->focus_cycle.remove(client, true);
        sync_focus_to_cycle();
    }

    m_desktops.remove_member(client);
    m_layers.remove_member(client);
    m_location.erase(client);
    m_size.erase(client);
    m_cps_mode.erase(client);
    m_screen.erase(client);
    m_autofocus.erase(client);
    m_pack_corners.erase(client);
    m_pack_priority.erase(client);

    std::set<Window> children(*m_children[client]);
    for (std::set<Window>::iterator child = children.begin();
         child != children.end();
         child++)
    {
        remove_child(*child, false);
    }

    delete m_children[client];
    m_children.erase(client);

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

    Desktop *desktop = find_desktop(client);
    if (desktop->is_user_desktop())
    {
        UserDesktop *user_desktop = dynamic_cast<UserDesktop*>(desktop);
        user_desktop->focus_cycle.remove(client, true);
        sync_focus_to_cycle();
    }
    else if (desktop->is_all_desktop())
    {
        dynamic_cast<AllDesktops*>(ALL_DESKTOPS)->focus_cycle.remove(client, true);
        sync_focus_to_cycle();
    }

    m_changes.push(new UnmapChange(client));
}

/**
 * Adds a new child window to the given client.
 */
void ClientModel::add_child(Window client, Window child)
{
    if (!is_client(client))
        return;

    if (is_child(child))
        return;

    m_children[client]->insert(child);
    m_parents[child] = client;

    m_changes.push(new ChildAddChange(client, child));

    if (is_autofocusable(client))
    {
        m_current_desktop->focus_cycle.add_after(child, client);
        focus(child);
    }
}

/**
 * Removes a child from the given client.
 */
void ClientModel::remove_child(Window child, bool focus_parent)
{
    if (!is_child(child))
        return;

    Window parent = m_parents[child];
    m_children[parent]->erase(child);
    m_parents.erase(child);

    if (m_focused == child)
    {
        if (focus_parent)
            focus(parent);
        else
            unfocus();
    }

    Desktop *desktop = find_desktop(parent);
    if (desktop->is_user_desktop())
    {
        UserDesktop *user_desktop = dynamic_cast<UserDesktop*>(desktop);
        user_desktop->focus_cycle.remove(child, false);
    }
    else if (desktop->is_all_desktop())
        dynamic_cast<AllDesktops*>(ALL_DESKTOPS)->focus_cycle.remove(child, false);

    m_changes.push(new ChildRemoveChange(parent, child));
}

/**
 * Configures the client for packing.
 */
void ClientModel::pack_client(Window client, PackCorner corner, unsigned long priority)
{
    if (is_packed_client(client))
        return;

    m_pack_corners[client] = corner;
    m_pack_priority[client] = priority;
}

/**
 * Returns true if the client is packed, or false otherwise.
 */
bool ClientModel::is_packed_client(Window client)
{
    return m_pack_corners.count(client) == 1;
}

/**
 * Returns the packing corner this client is assigned to. Undefined if
 * is_packed_client(the_client) is false.
 */
PackCorner ClientModel::get_pack_corner(Window client)
{
    return m_pack_corners[client];
}

/**
 * Repacks the clients at the given corner.
 */
void ClientModel::repack_corner(PackCorner corner)
{
    // We only need X because SmallWM doesn't currently support the vertical
    // packing case
    int x_incr_sign;
    int x_coord, y_coord;

    // For east/south side packings, we have to subtract the width/height of
    // the window from the current coordinate
    bool subtract_width_first, subtract_height_first;

    const Box &screen = get_root_screen();

    switch (corner)
    {
    case PACK_NORTHWEST:
        x_incr_sign = 1;
        subtract_width_first = false;
        subtract_height_first = false;
        x_coord = screen.x;
        y_coord = screen.y;
        break;
    case PACK_NORTHEAST:
        x_incr_sign = -1;
        subtract_width_first = true;
        subtract_height_first = false;
        x_coord = screen.width;
        y_coord = screen.y;
        break;
    case PACK_SOUTHWEST:
        x_incr_sign = 1;
        subtract_width_first = false;
        subtract_height_first = true;
        x_coord = 0;
        y_coord = screen.height;
        break;
    case PACK_SOUTHEAST:
        x_incr_sign = -1;
        subtract_width_first = true;
        subtract_height_first = true;
        x_coord = screen.width;
        y_coord = screen.height;
        break;
    }

    // We need to collect all the windows so that we can sort them by layout
    // order (low priority closest to the corner, higher priority farther
    // away)
    std::vector<Window> windows_on_this_corner;
    for (std::map<Window, PackCorner>::iterator iter = m_pack_corners.begin();
         iter != m_pack_corners.end();
         iter++)
    {
        if (iter->second == corner)
            windows_on_this_corner.push_back(iter->first);
    }

    MapSorter<Window, unsigned long> sorter(m_pack_priority);
    std::sort(windows_on_this_corner.begin(),
              windows_on_this_corner.end(),
              sorter);

    for (std::vector<Window>::iterator iter = windows_on_this_corner.begin();
         iter != windows_on_this_corner.end();
         iter++)
    {
        Dimension2D &size = m_size[*iter];

        int real_x, real_y;
        if (subtract_width_first)
            real_x = x_coord - DIM2D_WIDTH(size);
        else
            real_x = x_coord;

        if (subtract_height_first)
            real_y = y_coord - DIM2D_HEIGHT(size);
        else
            real_y = y_coord;

        change_location(*iter, real_x, real_y);
        x_coord += x_incr_sign * DIM2D_WIDTH(size);
    }
}

/**
 * Moves the focus forward, and focuses whatever window becomes current.
 */
void ClientModel::cycle_focus_forward()
{
    m_current_desktop->focus_cycle.forward();
    sync_focus_to_cycle();
}

/**
 * Moves the focus backward, and focuses whatever window becomes current.
 */
void ClientModel::cycle_focus_backward()
{
    m_current_desktop->focus_cycle.backward();
    sync_focus_to_cycle();
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
    // Packed clients are at a bit of a weird state, since they aren't movable
    // nor resizble by the user at all
    if (is_packed_client(client))
        return;

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
    const Box &old_desktop = m_screen[client];
    Crt *new_screen = m_crt_manager.screen_of_coord(x, y);
    const Box &new_desktop = m_crt_manager.box_of_screen(new_screen);

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
        update_size(client, width, height);
        m_changes.push(new ChangeSize(client, width, height));
    }
}

/**
 * Updates the size of a client without causing a change.
 *
 * This is meant to inform the model of changes that happened because of the
 * client doing things on its own, and not because of us.
 */
void ClientModel::update_size(Window client, Dimension width, Dimension height)
{
    if (width > 0 && height > 0)
        m_size[client] = Dimension2D(width, height);
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
    Window parent = client;
    if (is_child(client))
        parent = get_parent_of(client);

    if (!is_client(parent))
        return;

    if (!is_visible(parent))
        return;

    if (!m_autofocus[parent])
        return;

    Window old_focus = m_focused;
    m_focused = client;

    m_current_desktop->focus_cycle.set(client);
    m_changes.push(new ChangeFocus(old_focus, client));
}

/**
 * Forces a window to be focused, ignoring the nofocus policy.
 */
void ClientModel::force_focus(Window client)
{
    Window parent = client;
    if (is_child(client))
        parent = get_parent_of(client);

    if (!is_visible(parent))
        return;

    Window old_focus = m_focused;
    m_focused = client;

    m_current_desktop->focus_cycle.set(client);
    m_changes.push(new ChangeFocus(old_focus, client));
}

/**
 * Unfocuses a window if it is currently focused.
 */
void ClientModel::unfocus()
{
    unfocus(true);
}

/**
 * Unfocuses a window if it is currently focused, possibly invalidating the
 * focus cycle or not.
 * 
 * This should only be used when changing desktops, since it allows the state
 * of the focus cycle to be kept when shifting desktops. Everything else 
 * should use the 0-arg overload since it always invalidates the focus cycle.
 */
void ClientModel::unfocus(bool invalidate_cycle)
{
    if (m_focused != None)
    {
        Window old_focus = m_focused;
        m_focused = None;

        if (invalidate_cycle)
            m_current_desktop->focus_cycle.unset();

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
 * Synchronizes the currently focused window to what the focus cycle
 * says should be focused.
 */
void ClientModel::sync_focus_to_cycle()
{
    FocusCycle &cycle = m_current_desktop->focus_cycle;
    if (!cycle.valid())
        unfocus();
    else if (cycle.get() != m_focused)
        focus(cycle.get());
}

/**
 * Gets the current desktop which the client inhabits.
 *
 * You should _NEVER_ free the value returned by this function.
 */
Desktop* ClientModel::find_desktop(Window client)
{
    if (m_desktops.is_member(client))
        return m_desktops.get_category_of(client);
    else
        return static_cast<Desktop*>(0);
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

    Desktop* old_desktop = m_desktops.get_category_of(client);
    if (old_desktop->is_user_desktop())
        move_to_desktop(client, ALL_DESKTOPS, false);
    else
        move_to_desktop(client, m_current_desktop, false);
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
    Desktop* old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_user_desktop())
        return;

    move_to_desktop(client, m_current_desktop, false);
}

/**
 * Moves a client onto the next desktop.
 */
void ClientModel::client_next_desktop(Window client)
{
    Desktop* old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_user_desktop())
        return;

    UserDesktop* user_desktop = dynamic_cast<UserDesktop*>(old_desktop);
    unsigned long long desktop_index = user_desktop->desktop;
    desktop_index  = (desktop_index + 1) % m_max_desktops;
    move_to_desktop(client, USER_DESKTOPS[desktop_index], true);
}

/**
 * Moves a client onto the previous desktop.
 */
void ClientModel::client_prev_desktop(Window client)
{
    Desktop* old_desktop = m_desktops.get_category_of(client);
    if (!old_desktop->is_user_desktop())
        return;

    UserDesktop* user_desktop = dynamic_cast<UserDesktop*>(old_desktop);
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

    UserDesktop* old_desktop = m_current_desktop;
    m_current_desktop = USER_DESKTOPS[desktop_index];

    Window old_focus = m_focused;
    if (m_focused != None)
    {
        if (is_child(m_focused) && !is_visible(get_parent_of(m_focused)))
            unfocus(false);

        if (is_client(m_focused) && !is_visible(m_focused))
            unfocus(false);
    }

    m_changes.push(new ChangeCurrentDesktop(old_desktop, m_current_desktop));

    // If we can still focus the window we were focused on before, then do so
    // Otherwise, figure out the next logical window in the focus cycle
    if (m_focused != None && m_focused == old_focus)
        m_current_desktop->focus_cycle.set(m_focused);
    else
        sync_focus_to_cycle();
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

    UserDesktop* old_desktop = m_current_desktop;
    m_current_desktop = USER_DESKTOPS[desktop_index];

    Window old_focus = m_focused;
    if (m_focused != None)
    {
        if (is_child(m_focused) && !is_visible(get_parent_of(m_focused)))
            unfocus(false);

        if (is_client(m_focused) && !is_visible(m_focused))
            unfocus(false);
    }

    m_changes.push(new ChangeCurrentDesktop(old_desktop, m_current_desktop));

    // If we can still focus the window we were focused on before, then do so
    // Otherwise, figure out the next logical window in the focus cycle
    if (m_focused != None && m_focused == old_focus)
        m_current_desktop->focus_cycle.set(m_focused);
    else if (m_current_desktop->focus_cycle.valid())
        sync_focus_to_cycle();
}

/**
 * Hides the client and moves it onto the icon desktop.
 */
void ClientModel::iconify(Window client)
{
    Desktop* old_desktop = m_desktops.get_category_of(client);

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
    Desktop* old_desktop = m_desktops.get_category_of(client);
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

    Desktop* old_desktop = m_desktops.get_category_of(client);

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
    Desktop* old_desktop = m_desktops.get_category_of(client);
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

    Desktop* old_desktop = m_desktops.get_category_of(client);

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
    Desktop* old_desktop = m_desktops.get_category_of(client);
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
const Box &ClientModel::get_root_screen() const
{
    return m_crt_manager.box_of_screen(m_crt_manager.root());
}

/**
 * Gets the bounding box of the screen that the client currently inhabits.
 */
const Box &ClientModel::get_screen(Window client) const
{
    return m_screen.find(client)->second;
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

    // Since packing is only for the main screen for now, it doesn't make
    // sense to try to do this
    if (is_packed_client(client))
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
    const Box &current_box = m_screen[client];
    const Box &target_box = m_crt_manager.box_of_screen(screen);

    if (current_box != target_box)
    {
        m_screen.erase(client);
        m_screen.insert(std::pair<Window, const Box>(client, target_box));
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

        // Although this technically *should* occur, the way that this is handled would
        // cause the client to be moved outside of our control, and we don't want that
        if (is_packed_client(client))
            continue;

        // Keep the old screen - if the new screen is the same, we don't want
        // to send out a change notification
        const Box &old_box = m_screen[client];
        Box new_box(-1, -1, 0, 0);

        Crt *new_screen = m_crt_manager.screen_of_coord(
            DIM2D_X(location), DIM2D_Y(location));

        if (new_screen)
            new_box = m_crt_manager.box_of_screen(new_screen);

        if (new_box != old_box)
        {
            m_screen.erase(client);
            m_screen.insert(std::pair<Window, const Box>(client, new_box));

            // Why do the ref like this? Well, if it is done as a reference
            // to new_box directly, then new_box will go out of scope and
            // our data will be thoroughly shat over. Thankfully the unit
            // tests caught this one.
            m_changes.push(new ChangeScreen(client, m_screen[client]));
        }
    }

    // Since the location of the primary screen's corners may have changed, we
    // have to repack everything
    repack_corner(PACK_NORTHEAST);
    repack_corner(PACK_NORTHWEST);
    repack_corner(PACK_SOUTHEAST);
    repack_corner(PACK_SOUTHWEST);
}

/**
 * Moves a client between two desktops and fires the resulting event.
 */
void ClientModel::move_to_desktop(Window client, Desktop* new_desktop, bool should_unfocus)
{
    Desktop* old_desktop = m_desktops.get_category_of(client);
    if (*old_desktop == *new_desktop)
        return;

    bool can_focus = m_autofocus[client];
    m_desktops.move_member(client, new_desktop);

    if (can_focus && old_desktop->is_user_desktop())
    {
        UserDesktop *user_desktop = dynamic_cast<UserDesktop*>(old_desktop);
        user_desktop->focus_cycle.remove(client, false);

        for (std::set<Window>::iterator child = m_children[client]->begin();
                child != m_children[client]->end();
                child++)
            user_desktop->focus_cycle.remove(*child, false);
    }
    else if (can_focus && old_desktop->is_all_desktop())
    {
        dynamic_cast<AllDesktops*>(ALL_DESKTOPS)->focus_cycle.remove(client, false);

        for (std::set<Window>::iterator child = m_children[client]->begin();
                child != m_children[client]->end();
                child++)
            dynamic_cast<AllDesktops*>(ALL_DESKTOPS)->focus_cycle.remove(*child, false);
    }

    if (can_focus && new_desktop->is_user_desktop())
    {
        UserDesktop *user_desktop = dynamic_cast<UserDesktop*>(new_desktop);
        user_desktop->focus_cycle.add(client);

        for (std::set<Window>::iterator child = m_children[client]->begin();
                child != m_children[client]->end();
                child++)
            user_desktop->focus_cycle.add_after(*child, client);
    }
    else if (can_focus && new_desktop->is_all_desktop())
    {
        dynamic_cast<AllDesktops*>(ALL_DESKTOPS)->focus_cycle.add(client);

        for (std::set<Window>::iterator child = m_children[client]->begin();
                child != m_children[client]->end();
                child++)
            dynamic_cast<AllDesktops*>(ALL_DESKTOPS)->focus_cycle.add_after(*child, client);
    }

    if (should_unfocus)
    {
        if (m_focused == client)
        {
            unfocus();
        }
        else if (is_child(m_focused) && get_parent_of(m_focused) == client)
        {
            unfocus();
        }
    }
    // Make sure that the focus is transferred properly into the new cycle
    else if (can_focus && new_desktop->is_user_desktop())
    {
        UserDesktop *user_desktop = dynamic_cast<UserDesktop*>(new_desktop);
        user_desktop->focus_cycle.set(client);
    }
    else if (can_focus && new_desktop->is_all_desktop())
        dynamic_cast<AllDesktops*>(ALL_DESKTOPS)->focus_cycle.set(client);

    m_changes.push(new ChangeClientDesktop(client, old_desktop, new_desktop));
}
