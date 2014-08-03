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
}

/**
 * Handles a button click, which can do one of five things:
 *  - Launching a terminal
 *  - Deiconifying an icon
 *  - Start moving a window
 *  - Start resizing a window
 *  - Focusing a window
 */
void XEvents::handle_buttonpress()
{
    // We have to test both the window and the subwindow, because different
    // events use different windows
    bool is_client = false;
    if (m_clients.is_client(m_event.xbutton.window))
        is_client = true;

    if (m_clients.is_client(m_event.xbutton.subwindow))
        is_client = true;

    Icon *icon = m_xmodel.find_icon_from_icon_window(m_event.xbutton.window);

    // This is 
    if (!(is_client|| icon) && m_event.xbutton.button == LAUNCH_BUTTON 
            && m_event.xbutton.state == ACTION_MASK)
    {
        if (!fork())
        {
            /*
             * Here's why 'exec' is used in two different ways. First, it is
             * important to have /bin/sh process the shell command since it
             * supports argument parsing, which eases our burden dramatically.
             *
             * Now, consider the process sequence as depicted below (where 'xterm'
             * is the user's chosen shell).
             *
             * fork()
             * [creates process] ==> execl(/bin/sh, -c, /bin/sh, exec xterm)
             * # Somewhere in the /bin/sh source...
             * [creates process] ==> execl(/usr/bin/xterm, /usr/bin/xterm)
             *
             * If we used std::system instead, then the first process after fork()
             * would stick around to get the return code from the /bin/sh. If 'exec'
             * were not used in the /bin/sh command line, then /bin/sh would stick
             * around waiting for /usr/bin/xterm.
             *
             * So, to avoid an extra smallwm process sticking around, _or_ an 
             * unnecessary /bin/sh process sticking around, use 'exec' twice.
             */
            std::string shell = std::string("exec ") + m_shared.shell;
            execl("/bin/sh", "/bin/sh", "-c", shell.c_str(), NULL);
            exit(1);
        }
    }
    else if (icon)
    {
        // Any click on an icon, whether or not the action modifier is
        // enabled or not, should deiconify a client
        m_xmodel.unregister_icon(icon);
        m_clients.deiconify(icon->client);
        delete icon;
    }
    else if (is_client && m_event.xbutton.state == BUTTON_MASK)
    {
        if (m_event.xbutton.button != MOVE_BUTTON &&
                m_event.xbutton.button != RESIZE_BUTTON)
            return;

        // The placeholder is important, because it allows us to do as little
        // drawing as possible when it is resized. Create it here, since
        // both branches below use this.
        Window placeholder = m_xdata.create_window(true);

        // Align the placeholder to the window it's replacing
        XWindowAttributes attr;
        m_xdata.get_attributes(m_event.xbutton.subwindow, attr);

        m_xdata.move_window(placeholder, attr.x, attr.y);
        m_xdata.resize_window(placeholder, attr.width, attr.height);

        // Map this window onto the screen, bind the user's mouse to this window, and
        // then make it visible
        m_xdata.map_window(placeholder);
        m_xdata.confine_pointer(placeholder);
        m_xdata.raise(placeholder);

        // Figure out where the pointer is, since this is also important to
        // both of the branches
        Dimension ptr_x, ptr_y;
        m_xdata.get_pointer_location(ptr_x, ptr_y);
        Dimension2D ptr(ptr_x, ptr_y);
    
        // A left-click, with the action modifier, start resizing
        if (m_event.xbutton.button == MOVE_BUTTON)
        {
            m_clients.start_moving(m_event.xbutton.subwindow);
            m_xmodel.enter_move(m_event.xbutton.subwindow, placeholder, ptr);
        }

        // A right-click, with the action modifier, start resizing
        if (m_event.xbutton.button == RESIZE_BUTTON)
        {
            m_clients.start_resizing(m_event.xbutton.subwindow);
            m_xmodel.enter_resize(m_event.xbutton.subwindow, placeholder,
                ptr);
        }
    }
    else if (is_client) // Any other click on a client focuses that client
        m_clients.focus(m_event.xbutton.window);
}

/**
 * Handles the release of a mouse button. This event is only expected when
 * a placeholder is going to be released, so the only possible action is to
 * stop moving/resizing.
 */
void XEvents::handle_buttonrelease()
{
    Window expected_placeholder = m_xmodel.get_move_resize_placeholder();
    
    // If this is *not* the current placeholder, then bail
    if (expected_placeholder != m_event.xbutton.window)
        return;

    MoveResizeState state = x_model.get_move_resize_state()
    Window client = x_model.get_move_resize_client();

    // Figure out the attributes of the placeholder, so that way we can do
    // the movements/resizes
    XWindowAttributes attrs;
    m_xdata.get_attributes(expected_placeholder, attrs);

    switch (state)
    {
    case MR_MOVE:
        m_clients.stop_moving(client, Dimension2D(attrs.x, attrs.y));
        break;
    case MR_RESIZE:
        m_clients.stop_resizing(client, 
            Dimension2D(attrs.width, attrs.height));
        break;
    }

    x_model.exit_move_resize();
}

/**
 * Handles the motion of the pointer. The only time that this ever applies is
 * when the user has moved the placeholder window - at all other times, this
 * event is ignored.
 */
void XEvents::handle_motionnotify()
{
    // Get the placeholder's current geometry, since we need to modify the
    // placeholder relative to the way it is now
    Window placeholder = m_xmodel.get_move_resize_placeholder();
    if (placeholder == None)
        return;
    XWindowAttributes attr;
    m_xdata.get_attributes(placeholder, attr);

    // Avoid needless updates by getting the most recent version of this
    // event
    m_xdata.get_latest_event(m_event, MotionNotify);

    // Get the difference relative to the previous position
    Dimension ptr_x, ptr_y;
    m_xdata.get_pointer_location(ptr_x, ptr_y);

    Dimension2D relative_change = m_xmodel.update_pointer(ptr_x, ptr_y);

    switch (m_xmodel.get_move_resize_state)
    {
    case MR_MOVE:
        // Update the position of the placeholder
        m_xdata.move_window(placeholder, 
            attr.x + DIM2D_X(relative_change), 
            attr.y + DIM2D_Y(relative_change));
        break;
    case MR_RESIZE:
        // Update the location being careful to avoid making the placeholder
        // have a negative size
        if (attr.width + DIM2D_X(relative_change) <= 0)
            DIM2D_X(relative_change) = 0;
        if (attr.height + DIM2D_Y(relative_change) <= 0)
            DIM2D_Y(relative_change) = 0;

        m_xdata.resize_window(placeholder,
            attr.width + DIM2D_X(relative_change),
            attr.height + DIM2D_Y(relative_change));
        break;
    }
}

/**
 * This event is only ever called on icon windows, and causes the icon
 * window to be redrawn.
 */
void XEvents::handle_expose()
{
    Icon *the_icon = m_xmodel.find_icon_from_icon_window(
        m_event.xexpose.window);

    if (!the_icon)
        return;

    // Avoid drawing over the current contents of the icon
    the_icon->gc->clear();

    int text_x_offset;
    if (m_config.show_icons)
    {
        // Get the application's pixmap icon, and figure out where to place
        // the text (since the icon goes to the left)
        XWMHints hints;
        m_xdata.get_wm_hints(the_icon->client, hints);

        if (hints.flags & IconPixmapHint)
        {
            // Copy the pixmap into the left side of the icon, keeping
            // its size. The width of the pixmap is the same as the
            // X offset of the window name (no padding is done here).
            Dimension pixmap_size = the_icon->gc->copy_pixmap(
                hints.icon_pixmap, 0, 0);
            text_x_offset = DIM2D_WIDTH(pixmap_size);
        }
        else
            text_x_offset = 0;
    }
    else
        text_x_offset = 0;
        
    std::string preferred_icon_name;
    m_xdata.get_icon_name(the_icon->client, preferred_icon_name);

    // The one thing that is strange here is that the Y offset is the entire
    // icon's height. This is because Xlib draws the text, starting at the
    // Y offset, from *bottom* to *top*. I don't know why.
    the_icon->gc->draw_string(text_x_offset, m_config->icon_height,
        preferred_icon_name);
}

/**
 * Handles a window which has been destroyed, by unregistering it.
 */
void XEvents::handle_destroynotify()
{
    Window destroyed_window = m_event.xdestroywindow.window;

    Icon *as_icon = m_xmodel.find_icon_from_client(destroyed_window);
    if (as_icon)
    {
        // Deiconifying changes the focus, which we don't want, since the
        // recipient of the focus won't exist by the time the focus change is
        // processed.
        Window old_focus = m_clients.get_focused();

        // Start by ignoring all events completely, so that way we can do the
        // necessary changes to the client model without anybody else
        // knowing
        m_clients.begin_dropping_changes();
        {
            m_clients.deiconify(destroyed_window);

            // Since deiconifying messed up the focus, we have to restore it
            m_clients.focus(old_focus);
        }
        m_clients.end_dropping_changes();

        m_xmodel.unregister_icon(as_icon);
        delete icon;
    }

    if (m_xmodel.get_move_resize_client() == destroyed_window)
    {
        // First, we need to terminate the move/resize operation.
        // Since that involves changing the focus, we have to do what we
        // did above and play around with the focus manually.
        Window old_focus = m_clients.get_focused();

        m_clients.begin_dropping_changes();
        {
            switch (m_xmodel.get_move_resize_state())
            {
            case MR_MOVE:
                m_clients.stop_moving(destroyed_window, Dimension2D(0, 0));
                break;
            case MR_RESIZE:
                m_clients.stop_resizing(destroyed_window, Dimension2D(0, 0));
            }

            m_clients.focus(old_focus);
        }
        m_clients.end_dropping_changes();

        m_xmodel.exit_move_resize();
    }

    m_clients.remove_client(destroyed_window);
}

/**
 * Handles screen resizing events by updating the screen dimensions
 * when X RandR sends us events.
 */
void XEvents::handle_rrnotify()
{
    m_xdata.update_screen_size();
}
