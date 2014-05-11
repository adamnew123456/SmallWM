/** @file */
#include "clientmanager.h"

/**
 * Makes a sticky client unsticky, and an unsticky client sticky.
 * @param window The client window to stick/unstick.
 */
void DesktopManager::flip_sticky_flag(Window window)
{
    if (!m_clients->is_client(window))
        return;

    m_sticky[window] = !m_sticky[window];
}

/**
 * Puts a client onto the current desktop.
 * @param window The client to move.
 */
void DesktopManager::reset_desktop(Window window)
{
    m_desktops[window] = m_current_desktop;
}

/**
 * Sets the desktop of a client to the current desktop.
 * @param window The client to initialize.
 */
void DesktopManager::add_desktop(Window window)
{
    reset_desktop(window);
    m_sticky[window] = false;
}

/**
 * Checks to see if a client should be visible on the current desktop.
 * @param window The window to check the visibility of.
 * @return Whether or not the client should be visible.
 */
bool DesktopManager::should_be_visible(Window window)
{
    Desktop client_desktop = m_desktops[window];
    bool sticky = m_sticky[window];

    return (sticky ||
            client_desktop == m_current_desktop);
}

/**
 * Sets the current desktop of a client.
 * @param window The client.
 * @param desktop The desktop to put the client on.
 */
void DesktopManager::set_desktop(Window window, Desktop desktop)
{
    m_desktops[window] = desktop;
}

/**
 * Moves a client to the desktop after its current one.
 * @param window The client window to relocate.
 */
void DesktopManager::to_next_desktop(Window window)
{
    if (!m_clients->is_client(window))
        return;

    if (m_desktops[window] == m_shared.max_desktops)
        m_desktops[window] = 1;
    else
        m_desktops[window]++;

    m_clients->redesktop();
}

/**
 * Moves a client to the desktop before its current one.
 * @param window The client window to relocate.
 */
void DesktopManager::to_prev_desktop(Window window)
{
    if (!m_clients->is_client(window))
        return;

    if (m_desktops[window] == 1)
        m_desktops[window] = m_shared.max_desktops;
    else
        m_desktops[window]--;

    m_clients->redesktop();
}

/**
 * Removes a client from the desktop/sticky list.
 * @param window The client to remove.
 */
void DesktopManager::delete_desktop(Window window)
{
    m_desktops.erase(window);
    m_sticky.erase(window);
}

/**
 * Shows/hides the clients which should/should not be visible on the current desktop.
 */
void DesktopManager::update_desktop()
{
    for (DesktopManager::iterator client_iter = m_desktops.begin();
            client_iter != m_desktops.end();
            client_iter++)
    {
        ClientState state = m_clients->get_state(client_iter->first);
        if (should_be_visible(client_iter->first))
        {
            // Make sure that the focused window stays focused
            if (state != CS_ACTIVE)
                m_clients->state_transition(client_iter->first, CS_VISIBLE);
        }
        else
            m_clients->state_transition(client_iter->first, CS_INVISIBLE);
    }
}

/**
 * Switches the user's viewport to the next desktop.
 */
void DesktopManager::next_desktop()
{
    if (m_current_desktop == m_shared.max_desktops)
        m_current_desktop = 1;
    else
        m_current_desktop++;

    m_clients->redesktop();
}

/**
 * Switches a user's viewport to the previous desktop.
 */
void DesktopManager::prev_desktop()
{
    if (m_current_desktop == 1)
        m_current_desktop = m_shared.max_desktops;
    else
        m_current_desktop--;

    m_clients->redesktop();
}
