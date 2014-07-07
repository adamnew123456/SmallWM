/** @file */
#ifndef __SMALLWM_CLIENT_MODEL__
#define __SMALLWM_CLIENT_MODEL__

#include "changes.h"
#include "desktop-type.h"
#include "maybe-type.h"
#include "unique-multimap.h"

#include <algorithm>
#include <utility>
#include <vector>

enum InitialState
{
    IS_VISIBLE,
    IS_HIDDEN,
};

bool is_valid_size(Dimension2D &size)
{
    return (DIM2D_WIDTH(size) > 0 &&
            DIM2D_HEIGHT(size) > 0);
}

/**
 * This defines the data model used for the client.
 *
 * This is intended to be totally divorced from Xlib, and is meant to do
 * transformations of data relating to clients and validations of that data.
 *
 * Note that the type of the client doesn't matter, so the user is allowed to
 * fill that in via a template.
 */
class ClientModel
{
public:
    typedef std::vector<Change>::iterator change_iter;
    typedef UniqueMultimap<Desktop,Window>::member_iter client_iter;

    /**
     * Initializes all of the categories in the maps
     */
    ClientModel(unsigned long long max_desktops) :
        m_max_desktops(max_desktops), m_current_desktop(1),
        m_focused(None)
    {
        m_desktops.add_category(AllDesktops());
        m_desktops.add_category(IconDesktop());
        m_desktops.add_category(MovingDesktop());
        m_desktops.add_category(ResizingDesktop());

        for (unsigned long long desktop = 1; desktop <= max_desktops;
                desktop++)
            m_desktops.add_category(UserDesktop(desktop));
    }

    /**
     * Removes all of the recorded changes.
     */
    void flush_changes()
    {
        m_changes.clear();
    }

    /**
     * Gets an iterator representing the start of the change list.
     */
    change_iter changes_begin()
    {
        return m_changes.begin();
    }

    /**
     * Gets an iterator representing the end of the change list.
     */
    change_iter changes_end()
    {
        return m_changes.end();
    }

    /**
     * Returns whether or not a client exists.
     */
    bool is_client(Window client)
    {
        return m_desktops.is_member(client);
    }

    /**
     * Returns whether or not a client is visible.
     */
    bool is_visible(Window client)
    {
        Desktop &desktop_of = m_desktops.get_category_of(client);
        if (desktop_of.is_all_desktop())
            return true;

        if (desktop_of.is_user_desktop())
            return desktop_of == m_current_desktop;

        return false;
    }

    /**
     * Gets a list of all of the clients on a desktop.
     */
    void get_clients_of(Desktop const &desktop,
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
    void get_visible_clients(std::vector<Window> &return_clients)
    {
        for (client_iter iter = 
                    m_desktops.get_members_of_begin(m_current_desktop);
                iter != m_desktops.get_members_of_end(m_current_desktop);
                iter++)
            return_clients.push_back(*iter);

        for (client_iter iter = 
                    m_desktops.get_members_of_begin(AllDesktops());
                iter != m_desktops.get_members_of_end(AllDesktops());
                iter++)
            return_clients.push_back(*iter);
    }

    /**
     * Gets all of the visible windows, but sorted by layer from bottom to
     * top.
     */
    void get_visible_in_layer_order(std::vector<Window> &return_clients)
    {
        get_visible_clients(return_clients);

        UniqueMultimapSorter<Layer, Window> layer_sorter(m_layers);
        std::sort(return_clients.begin(), return_clients.end(), layer_sorter);
    }
    
    /**
     * Adds a new client with some basic initial state.
     */
    void add_client(Window client, InitialState state,
        Dimension2D location, Dimension2D size)
    {
        if (!is_valid_size(size))
            return;

        // Special care is given to honor the initial state, since it is
        // mandated by the ICCCM
        switch (state)
        {
            case IS_VISIBLE:
                m_desktops.add_member(m_current_desktop, client);
                push_change(ChangeClientDesktop(client, m_current_desktop));
                break;
            case IS_HIDDEN:
                m_desktops.add_member(IconDesktop(), client);
                push_change(ChangeClientDesktop(client, IconDesktop()));
                break;
        }

        m_layers.add_member(DEF_LAYER, client);
        push_change(ChangeLayer(client, DEF_LAYER));

        // Since the size and locations are already current, don't put out
        // an event now that they're set
        m_location[client] = location;
        m_size[client] = size;

        focus(client);
    }

    /**
     * Removes a client.
     *
     * Note that this doesn't put out any events, since this method should
     * only be called once the client is destroyed
     */
    void remove_client(Window client)
    {
        // A destroyed window cannot be focused
        unfocus_if_focused(client);

        // Remove all the data associated with the client
        m_desktops.remove_member(client);
        m_layers.remove_member(client);
        m_location.erase(client);
        m_size.erase(client);
    }

    /**
     * Changes the focus to another window. Note that this fails if the client
     * is not currently visible.
     */
    void focus(Window client)
    {
        if (!is_visible(client))
            return;

        Window old_focus = m_focused;
        m_focused = client;
        push_change(ChangeFocus(old_focus, client));
    }

    /**
     * Unfocuses a window if it is currently focused.
     */
    void unfocus()
    {
        Window old_focus = m_focused;
        m_focused = None;
        push_change(ChangeFocus(old_focus, None));
    }

    /**
     * Unfocuses a window if it is currently focused, otherwise the focus is
     * not changed.
     */
    void unfocus_if_focused(Window client)
    {
        if (m_focused == client)
            unfocus();
    }

    /**
     * Gets the current desktop which the client inhabits.
     */
    Desktop find_desktop(Window client)
    {
        return m_desktops.get_category_of(client);
    }

    /**
     * Gets the current layer which the client inhabits.
     */
    Layer find_layer(Window client)
    {
        return m_layers.get_category_of(client);
    }

    /**
     * Toggles the stickiness of a client.
     */
    void toggle_stick(Window client)
    {
        if (!is_visible(client))
            return;

        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (old_desktop.is_user_desktop())
            move_to_desktop(client, AllDesktops(), false);
        else
            move_to_desktop(client, m_current_desktop, false);
    }

    /**
     * Moves a client up in the layer stack.
     */
    void up_layer(Window client)
    {
        Layer old_layer = m_layers.get_category_of(client);
        if (old_layer < MAX_LAYER)
        {
            m_layers.move_member(client, old_layer + 1);
            push_change(ChangeLayer(client, old_layer + 1));
        }
    }

    /**
     * Moves a client up in the layer stack.
     */
    void down_layer(Window client)
    {
        Layer old_layer = m_layers.get_category_of(client);
        if (old_layer > MIN_LAYER)
        {
            m_layers.move_member(client, old_layer - 1);
            push_change(ChangeLayer(client, old_layer - 1));
        }
    }

    /**
     * Changes the layer of a client.
     */
    void set_layer(Window client, Layer layer)
    {
        if (!m_layers.is_category(layer))
            return;

        Layer old_layer = m_layers.get_category_of(client);
        if (old_layer != layer)
        {
            m_layers.move_member(client, layer);
            push_change(ChangeLayer(client, layer));
        }
    }

    /**
     * Moves a client onto the next desktop.
     */
    void client_next_desktop(Window client)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (!old_desktop.is_user_desktop())
            return;

        UserDesktop *user_desktop = (UserDesktop*)&old_desktop;
        unsigned long long desktop_index = user_desktop->desktop;
        if (desktop_index == m_max_desktops)
            desktop_index = 1;
        else
            desktop_index++;

        move_to_desktop(client, UserDesktop(desktop_index), true);
    }

    /**
     * Moves a client onto the previous desktop.
     */
    void client_prev_desktop(Window client)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (!old_desktop.is_user_desktop())
            return;

        UserDesktop *user_desktop = (UserDesktop*)&old_desktop;
        unsigned long long desktop_index = user_desktop->desktop;
        if (desktop_index == 1)
            desktop_index = m_max_desktops;
        else
            desktop_index--;

        move_to_desktop(client, UserDesktop(desktop_index), true);
    }
    
    /**
     * Changes the current desktop to the desktop after the current.
     */
    void next_desktop()
    {
        if (m_current_desktop.desktop == m_max_desktops)
            m_current_desktop.desktop = 1;
        else
            m_current_desktop.desktop++;

        push_change(ChangeCurrentDesktop(m_current_desktop));
    }

    /**
     * Changes the current desktop to the desktop before the current.
     */
    void prev_desktop()
    {
        if (m_current_desktop.desktop == 1)
            m_current_desktop.desktop = m_max_desktops;
        else
            m_current_desktop.desktop--;

        push_change(ChangeCurrentDesktop(m_current_desktop));
    }

    /**
     * Hides the client and moves it onto the icon desktop.
     */
    void iconify(Window client)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (old_desktop.is_icon_desktop())
            return;
        else if (!is_visible(client))
            return;

        move_to_desktop(client, IconDesktop(), true);
    }

    /**
     * Hides the client and moves it onto the icon desktop.
     */
    void deiconify(Window client)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (!old_desktop.is_icon_desktop())
            return;

        // Focus after making the client visible, since a non-visible client
        // cannot be allowed to be focused
        move_to_desktop(client, m_current_desktop, false);
        focus(client);
    }

    /**
     * Starts moving a window.
     */
    void start_moving(Window client)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (!is_visible(client))
            return;

        // Only one window, at max, can be either moved or resized
        if (m_desktops.count_members_of(MovingDesktop()) > 0 ||
                m_desktops.count_members_of(ResizingDesktop()) > 0)
            return

        move_to_desktop(client, MovingDesktop(), true);
    }

    /**
     * Stops moving a window, and fixes its position.
     */
    void stop_moving(Window client, Dimension2D location)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (!old_desktop.is_moving_desktop())
            return;

        move_to_desktop(client, m_current_desktop, false);

        m_location[client] = location;
        push_change(ChangeLocation(client, DIM2D_X(location),
            DIM2D_Y(location)));

        focus(client);
    }

    /**
     * Starts moving a window.
     */
    void start_resizing(Window client)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (!is_visible(client))
            return;

        // Only one window, at max, can be either moved or resized
        if (m_desktops.count_members_of(MovingDesktop()) > 0 ||
                m_desktops.count_members_of(ResizingDesktop()) > 0)
            return

        move_to_desktop(client, ResizingDesktop(), true);
    }

    /**
     * Stops resizing a window, and fixes its position.
     */
    void stop_resizing(Window client, Dimension2D size)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (!old_desktop.is_resizing_desktop())
            return;

        move_to_desktop(client, m_current_desktop, false);

        if (!is_valid_size(size))
        {
            m_size[client] = size;
            push_change(ChangeSize(client, DIM2D_WIDTH(size),
                DIM2D_HEIGHT(size)));
        }

        focus(client);
    }

protected:
    /**
     * Pushes a change into the change buffer.
     */
    void push_change(Change change)
    {
        m_changes.push_back(change);
    }

    /**
     * Moves a client between two desktops and fires the resulting event.
     */
    void move_to_desktop(Window client, const Desktop &new_desktop, bool unfocus)
    {
        Desktop &old_desktop = m_desktops.get_category_of(client);
        if (old_desktop == new_desktop)
            return;

        if (unfocus)
            unfocus_if_focused(client);

        m_desktops.move_member(client, new_desktop);
        push_change(ChangeClientDesktop(client, new_desktop));
    }

private:
    /// A list of the changes made to the client data
    std::vector<Change> m_changes;
    /// The maximum number of user-visible desktops
    unsigned long long m_max_desktops;

    /// A mapping between clients and their desktops
    UniqueMultimap<Desktop, Window> m_desktops;
    /// A mapping between clients and the layers they inhabit
    UniqueMultimap<Layer, Window> m_layers;
    /// A mapping between clients and their locations
    std::map<Window, Dimension2D> m_location;
    /// A mapping between clients and their sizes
    std::map<Window, Dimension2D> m_size;

    /// The currently visible desktop
    UserDesktop m_current_desktop;
    /// The currently focused client
    Window m_focused;
};

#endif
