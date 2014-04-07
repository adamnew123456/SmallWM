#include "clients.h"

/**
 * Check to see if a window is a registered client window.
 * @param window The window to check.
 * @return Whether or not the window represents a client.
 */
bool ClientContainer::is_client(Window window)
{
    // Fixes a strange bug, where the state of a window would be set to 0 for
    // no apparent reason, even though the window was destroyed.
    return m_clients[window] != 0;
}

/**
 * Make sure that a client is mapped onto the display.
 * @param window The client window to check.
 * @return Whether the client window is visible or not.
 */
bool ClientContainer::is_visible(Window window)
{
    ClientState state = m_clients[window];
    return (state == CS_ACTIVE) || (state == CS_VISIBLE);
}

/**
 * Gets the iterator at the beginning of the client list.
 * @return An iterator.
 */
std::map<Window,ClientState>::iterator ClientContainer::clients_begin()
{
    return m_clients.begin();
}

/**
 * Gets an iterator at the end of the client list.
 * @return An iterator.
 */
std::map<Window,ClientState>::iterator ClientContainer::clients_end()
{
    return m_clients.end();
}

/**
 * Gets the state of a client.
 * @param window The window to get the state of.
 * @return The state of the given client.
 */
ClientState ClientContainer::get_state(Window window)
{
    return m_clients[window];
}

/**
 * Sets the state of a client.
 * @param window The window to set the state of.
 * @param state The state to set the client to.
 */
void ClientContainer::set_state(Window window, ClientState state)
{
    if (state < CS_ACTIVE || state > CS_DESTROY)
        delete_state(window);
    else
    {
        m_clients[window] = state;

        // As mandated by ICCCM, we should set the WM_STATE property depending
        // on how the client container has been changed.

        // Although ICCCM mandates that we give the client the icon window, we
        // don't want them to muck with it.
        int property[] = {0, None};

        switch (state)
        {
            case CS_ACTIVE:
            case CS_VISIBLE:
                property[0] = NormalState;
                break;
            case CS_WITHDRAWN:
            case CS_INVISIBLE:
            case CS_MOVING:
            case CS_RESIZING:
                property[0] = WithdrawnState;
                break;
            case CS_ICON:
                property[0] = IconicState;
                break;
        }

        XChangeProperty(m_shared.display, window,
                m_shared.atoms["WM_STATE"],
                XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char*)property, 2);
    }
}

/**
 * Removes a client from the client state list.
 * @param window The window to remove.
 */
void ClientContainer::delete_state(Window window)
{
    m_clients.erase(window);
}
