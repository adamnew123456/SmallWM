#include "clientmanager.h"

/**
 * Makes a sticky client unsticky, and an unsticky client sticky.
 * @param window The client window to stick/unstick.
 */
void ClientManager::flip_sticky_flag(Window window)
{
    if (!is_client(window))
        return;

    m_sticky[window] = !m_sticky[window];
}

/**
 * Moves a client to the desktop after its current one.
 * @param window The client window to relocate.
 */
void ClientManager::to_next_desktop(Window window)
{
    if (m_desktops[window] == m_shared.max_desktops)
        m_desktops[window] = 1;
    else
        m_desktops[window]++;

    update_desktop();
}

/**
 * Moves a client to the desktop before its current one.
 * @param window The client window to relocate.
 */
void ClientManager::to_prev_desktop(Window window)
{
    if (m_desktops[window] == 1)
        m_desktops[window] = m_shared.max_desktops;
    else
        m_desktops[window]--;

    update_desktop();
}

/**
 * Shows/hides the clients which should/should not be visible on the current desktop.
 */
void ClientManager::update_desktop()
{
    for (std::map<Window,Desktop>::iterator client_iter = m_desktops.begin();
            client_iter != m_desktops.end();
            client_iter++)
    {
        bool sticky = m_sticky[client_iter->first];
        if (sticky || client_iter->second == m_current_desktop)
            state_transition(client_iter->first, CS_VISIBLE);
        else
            state_transition(client_iter->first, CS_INVISIBLE);
    }
}

/**
 * Switches the user's viewport to the next desktop.
 */
void ClientManager::next_desktop()
{
    if (m_current_desktop == m_shared.max_desktops)
        m_current_desktop = 1;
    else
        m_current_desktop++;

    update_desktop();
}

/**
 * Switches a user's viewport to the previous desktop.
 */
void ClientManager::prev_desktop()
{
    if (m_current_desktop == 1)
        m_current_desktop = m_shared.max_desktops;
    else
        m_current_desktop--;

    update_desktop();
}
