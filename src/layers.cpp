/** @file */
#include "clientmanager.h"

/**
 * Gets the current 'offset', which is 0 when the window is unfocused and 5 when it is.
 */
int get_layer_offset(Layer layer)
{
    return layer % 10;
}

/**
 * Raises a client to the layer above it.
 * @param window The client to raise
 */
void LayerManager::raise_layer(Window window)
{
    if (!m_clients->is_client(window))
        return;

    Layer current = m_layers[window];
    if (current < MAX_LAYER)
    {
        m_layers[window] += 10;
        m_clients->relayer();
    }
}

/**
 * Lowers a client to the layer below it.
 * @param window The client to lower
 */
void LayerManager::lower_layer(Window window)
{
    if (!m_clients->is_client(window))
        return;

    Layer current = m_layers[window];
    if (current > MIN_LAYER)
    {
        m_layers[window] -= 10;
        m_clients->relayer();
    }
}

/**
 * Shifts the layer of a window up/down by a certain value.
 * @param window The window of a client
 * @param offset The z-layer to shift by.
 */
void LayerManager::adjust_layer(Window window, LayerDiff layer)
{
    Layer new_layer = m_layers[window] + layer;
    if (new_layer >= MIN_LAYER && new_layer <= MAX_LAYER)
    {
        m_layers[window] = new_layer;
        m_clients->relayer();
    }
}

/**
 * Sets the layer of a client.
 * @param window The window of a client
 * @param layer The layer to set the client to
 */
void LayerManager::set_layer(Window window, Layer layer)
{
    if (!m_clients->is_client(window))
        return;
    
    // If the window is focused, then be sure to keep the focus offset when
    // changing to a new layer
    Layer old_layer = m_layers[window];
    Layer offset = get_layer_offset(old_layer);

    m_layers[window] = layer + offset;
    m_clients->relayer();
}

/**
 * Removes a client from the layer list.
 * @param window The client window to remove.
 */
void LayerManager::delete_layer(Window window)
{
    m_layers.erase(window);
}

/**
 * Relayers all clients which are currently CS_VISIBLE or CS_ACTIVE. Puts all
 * CS_ICON clients, or the placeholder windows for any CS_MOVING or CS_RESIZING
 * clients on the very top.
 */
void LayerManager::relayer_clients()
{
    // First, only layer the 'normal' windows which are not icons or placeholders.
    std::vector<Window> clients;
    for (LayerManager::iterator client_iter = m_layers.begin();
            client_iter != m_layers.end();
            client_iter++)
    {
        ClientState state = m_clients->get_state(client_iter->first);
        if (state == CS_VISIBLE || state == CS_ACTIVE)
            clients.push_back(client_iter->first);
    }

    MappedVectorSorter<Window,Layer> sorter(m_layers, true);
    std::sort(clients.begin(), clients.end(), sorter);
    XRestackWindows(m_shared.display, &clients[0], clients.size());
}
