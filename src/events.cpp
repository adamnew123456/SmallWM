#include "events.h"

/**
 * Runs a single iteration of the event loop, by capturing an X event and
 * acting upon it.
 *
 * @return true if more events can be processed, false otherwise.
 */
bool XEvents::step()
{
    // Avoid processing any more events if we've been cancelled
    if (m_done)
        return false;

    // Grab the next event from X, and then dispatch upon its type
    m_xdata.next_event(m_event);

    if (m_event.type == m_randroffset + RRNotify)
        handle_rrnotify();
    
    if (m_event.type == KeyPress)
        handle_keypress();

    if (m_event.type == ButtonPress)
        handle_buttonpress();

    if (m_event.type == ButtonRelease)
        handle_buttonrelease();

    if (m_event.type == MotionNotify)
        handle_motionnotify();

    if (m_event.type == MapNotify)
        handle_mapnotify();

    if (m_event.type == Expose)
        handle_expose();

    if (m_event.type == DestroyNotify)
        handle_destroynotify();
}

/**
 * Handles keyboard shortcuts.
 */
void XEvents::handle_keypress()
{
    KeySym key = m_xdata.get_keysym(m_event.xkey.keycode);

    Window client = m_event.xkey.subwindow;
    if (client == None)
        client = m_event.xkey.window;

    bool is_client = m_clients.is_client(client);

    // Only some of the actions below need this, but it shouldn't be
    // prohibitively expensive to go ahead and do it here
    Dimension scr_width, scr_height;
    m_xdata.get_screen_size(scr_width, scr_height);

    // We need to subtract the height of the icon row when dealing with
    // the size of the screen
    Dimension workspace_height = scr_height - m_config.icon_height;

    KeyboardAction action = m_config.key_commands.reverse_bindings[key];
    switch (action)
    {
    case CLIENT_NEXT_DESKTOP:
        is_clietn && m_clients.client_next_desktop(client);
        break;
    case CLIENT_PREV_DESKTOP:
        is_client && m_clients.client_prev_desktop(client);
        break;
    case NEXT_DESKTOP:
        m_clients.next_desktop();
        break;
    case PREV_DESKTOP:
        m_clients.prev_desktop();
        break;
    case TOGGLE_STICK:
        is_client && m_clients.toggle_stick(client);
        break;
    case ICONIFY:
        is_client && m_clients.iconify(client);
        break;
    case DEICONIFY:
        is_client && m_clients.deiconify(client);
        break;
    case MAXIMIZE:
        if (is_client)
        {
            m_clients.change_location(client, 0, m_config.icon_height);
            m_clients.change_size(client, scr_width, scr_height);
        }; 
        break;
    case REQUEST_CLOSE:
        is_client && m_xdata.request_close(client);
        break;
    case FORCE_CLOSE:
        is_client && m_xdata.destroy_win(client);
        break;
    case K_SNAP_TOP:
        if (is_client)
        {
            m_clients.change_location(client, 0, m_config.icon_height);
            m_clients.change_size(client, scr_width, workspace_height / 2);
        }
        break;
    case K_SNAP_BOTTOM:
        if (is_client)
        {
            m_clients.change_location(client, 0, 
                m_config.icon_height + (workspace_height / 2));
            m_clients.change_size(client, scr_width, workspace_height / 2);
        }
        break;
    case K_SNAP_LEFT:
        if (is_client)
        {
            m_clients.change_location(client, 0, m_config.icon_height);
            m_clients.change_size(client, scr_width / 2, workspace_height);
        }
        break;
    case K_SNAP_RIGHT:
        if (is_client)
        {
            m_clients.change_location(client, scr_width / 2, 
                m_config.icon_height);
            m_clients.change_size(client, scr_width / 2, workspace_height);
        }
        break;

    case LAYER_ABOVE:
        is_client && m_clients.up_layer(client);
        break;
    case LAYER_BELOW:
        is_client && m_clients.down_layer(client);
        break;
    case LAYER_TOP:
        is_client && m_clients.set_layer(client, MAX_LAYER);
        break;
    case LAYER_BOTTOM:
        is_client && m_clients.set_layer(client, MIN_LAYER);
        break;

#define LAYER_SET(l) case LAYER_##l: \
        is_client && m_clients.set_layer(client, l); \
        break;

    LAYER_SET(1);
    LAYER_SET(2);
    LAYER_SET(3);
    LAYER_SET(4);
    LAYER_SET(5);
    LAYER_SET(6);
    LAYER_SET(7);
    LAYER_SET(8);
    LAYER_SET(9);

#undef LAYER_SET

    case EXIT_WM:
        m_done = true;
        break;
}
