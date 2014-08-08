#include "clientmodel-events.h"

/**
 * Handles all the currently queued change events, returning when the
 * ClientModel change list is exhausted.
 */
void ClientModelEvents::handle_queued_changes()
{
    m_should_relayer = false;

    for (ClientModel::change_iter change_iter = m_clients.changes_begin();
            change_iter != m_clients.changes_end();
            change_iter++)
    {
        m_change = *change_iter;

        if (m_change->is_layer_change())
            handle_layer_change();
        else if (m_change->is_focus_change())
            handle_focus_change();
        else if (m_change->is_client_desktop_change())
            handle_client_desktop_change();
        else if (m_change->is_current_desktop_change())
            handle_current_desktop_change();
        else if (m_change->is_location_change())
            handle_location_change();
        else if (m_change->is_size_change())
            handle_size_change();
    }

    if (m_should_relayer)
        do_relayer();
}

/**
 * Sets a flag so that relayering occurs later - this avoid relayering on
 * every ChangeLayer event.
 */
void ClientModelEvents::handle_layer_change()
{
    m_should_relayer = true;
}
