/** @file */
#include "x-model.h"

/**
 * Registers a new icon - note that, at this point, XModel takes
 * responsibility for the given icon, and it should not be deleted unless
 * explicitly unregistered.
 *
 * @param icon The new icon to register.
 */
void XModel::register_icon(Icon *icon)
{
    m_clients_to_icons[icon->client] = icon;
    m_icon_windows_to_icons[icon->icon] = icon;
}

/**
 * Unregisters an icon. This also removes the responsibility of the pointer
 * from the XModel, and it should be deleted later to avoid a memory leak.
 *
 * @param icon The icon to unregister.
 */
void XModel::unregister_icon(Icon *icon)
{
    m_clients_to_icons.erase(icon->client);
    m_icon_windows_to_icons.erase(icon->icon);
}

/**
 * Gets the icon from the client window the icon is hiding.
 */
Icon* XModel::find_icon_from_client(Window client)
{
    return m_clients_to_icons[client];
}

/**
 * Gets the icon from the icon window which is being shown.
 */
Icon* XModel::find_icon_from_icon_window(Window icon_win)
{
    return m_icon_windows_to_icons[icon_win];
}

/**
 * Gets a list of all of the icons.
 */
void XModel::get_icons(std::vector<Icon*> &icons)
{
    for (std::map<Window,Icon*>::iterator iter = m_clients_to_icons.begin();
            iter != m_clients_to_icons.end();
            iter++)
        icons.push_back(iter->second);
}

/**
 * Registers that a client is being moved, recording the client and the
 * placeholder, and recording the current pointer location.
 *
 * @return true if the change is successful, false if the change cannot be
 *      done due to an invalid state.
 */
void XModel::enter_move(Window client, Window placeholder, 
    Dimension2D pointer)
{
    if (m_moveresize)
        return;

    m_moveresize = new MoveResize(client, placeholder, MR_MOVE);
    m_pointer = pointer;
}

/**
 * Registers that a client is being resized, recording the client and the
 * placeholder, and recording the current pointer location.
 *
 * @return true if the change is successful, false if the change cannot be
 *      done due to an invalid state.
 */
void XModel::enter_resize(Window client, Window placeholder,
    Dimension2D pointer)
{
    if (m_moveresize)
        return;

    m_moveresize = new MoveResize(client, placeholder, MR_RESIZE);
    m_pointer = pointer;
}

/**
 * Updates the pointer to a new location, returning the difference between
 * the old position and the current position.
 *
 * Note that, if no movement or resizing is currently going on, then the
 * return value will be (0, 0).
 */
Dimension2D XModel::update_pointer(Dimension x, Dimension y)
{
    if (!m_moveresize)
        return Dimension2D(0, 0);

    Dimension2D diff(x - DIM2D_X(m_pointer),
                     y - DIM2D_Y(m_pointer));

    DIM2D_X(m_pointer) = x;
    DIM2D_Y(m_pointer) = y;
    return diff;
}

/**
 * Gets the current placeholder which is being used to move/resize.
 *
 * @return The placeholder, or None if no window is being moved/resized.
 */
Window XModel::get_move_resize_placeholder()
{
    if (!m_moveresize)
        return None;

    return m_moveresize->placeholder;
}

/**
 * Gets the current client which is being moved/resized.
 *
 * @return The client, or None if no window is being moved/resized.
 */
Window XModel::get_move_resize_client()
{
    if (!m_moveresize)
        return None;

    return m_moveresize->client;
}

/**
 * Gets the current move/resize state.
 */
MoveResizeState XModel::get_move_resize_state()
{
    if (!m_moveresize)
        return MR_INVALID;

    return m_moveresize->state;
}

/**
 * Stops moving/resizing.
 */
void XModel::exit_move_resize()
{
    if (!m_moveresize)
        return;

    delete m_moveresize;
    m_moveresize = 0;
}
