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
        m_clients[window] = state;
}

/**
 * Removes a client from the client state list.
 * @param window The window to remove.
 */
void ClientContainer::delete_state(Window window)
{
    m_clients.erase(window);
}
