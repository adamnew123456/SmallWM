/** @file */
#include "clientmanager.h"

/**
 * Raises a client to the layer above it.
 * @param window The client to raise
 */
void ClientManager::raise_layer(Window window)
{
    if (!is_client(window))
        return;

    Layer current = m_layers[current];
    if (current < MAX_LAYER)
    {
        m_layers[current]++;
        relayer();
    }
}

/**
 * Lowers a client to the layer below it.
 * @param window The client to lower
 */
void ClientManager::lower_layer(Window window)
{
    if (!is_client(window))
        return;

    Layer current = m_layers[current];
    if (current > MIN_LAYER)
    {
        m_layers[current]--;
        relayer();
    }
}

/**
 * Sets the layer of a client.
 * @param window The window of a client
 * @param layer The layer to set the client to
 */
void ClientManager::set_layer(Window window, Layer layer)
{
    if (!is_client(window))
        return;

    m_layers[window] = layer;
    relayer();
}

/**
 * Relayers all clients which are currently CS_VISIBLE or CS_ACTIVE. Puts all
 * CS_ICON clients, or the placeholder windows for any CS_MOVING or CS_RESIZING
 * clients on the very top.
 */
void ClientManager::relayer()
{
    // First, only layer the 'normal' windows which are not icons or placeholders.
    std::vector<Window> clients;
    for (std::map<Window,ClientState>::iterator client_iter = m_clients.begin();
            client_iter != m_clients.end();
            client_iter++)
    {
        if (client_iter->second == CS_VISIBLE || client_iter->second == CS_ACTIVE)
            clients.push_back(client_iter->first);
    }

    MappedVectorSorter<Window,Layer> sorter(m_layers, true);
    std::sort(clients.begin(), clients.end(), sorter);
    XRestackWindows(m_shared.display, &clients[0], clients.size());

    // Now, go back and put all of the icons on the top
    for (std::map<Window,Icon*>::iterator icon_iter = m_icons.begin();
            icon_iter != m_icons.end();
            icon_iter++)
    {
        XRaiseWindow(m_shared.display, icon_iter->first);
    }

    // Finally, put the placeholder on the top, if there is one
    if (m_mvr.window != None)
        XRaiseWindow(m_shared.display, m_mvr.window);
}
