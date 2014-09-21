#include "x-events.h"

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

    KeyboardAction action = m_config.key_commands.keysym_to_action[key];
    switch (action)
    {
    case CLIENT_NEXT_DESKTOP:
        if (is_client)
             m_clients.client_next_desktop(client);
        break;
    case CLIENT_PREV_DESKTOP:
        if (is_client)
             m_clients.client_prev_desktop(client);
        break;
    case NEXT_DESKTOP:
        m_clients.next_desktop();
        break;
    case PREV_DESKTOP:
        m_clients.prev_desktop();
        break;
    case TOGGLE_STICK:
        if (is_client)
             m_clients.toggle_stick(client);
        break;
    case ICONIFY:
        if (is_client)
             m_clients.iconify(client);
        break;
    case MAXIMIZE:
        if (is_client)
            maximize_client(client);
        break;
    case REQUEST_CLOSE:
        if (is_client)
            m_xdata.request_close(client);
        break;
    case FORCE_CLOSE:
        if (is_client)
            m_xdata.destroy_win(client);
        break;
    case K_SNAP_TOP:
        if (is_client)
            snap_client(client, SNAP_TOP);
        break;
    case K_SNAP_BOTTOM:
        if (is_client)
            snap_client(client, SNAP_BOTTOM);
        break;
    case K_SNAP_LEFT:
        if (is_client)
            snap_client(client, SNAP_LEFT);
        break;
    case K_SNAP_RIGHT:
        if (is_client)
            snap_client(client, SNAP_RIGHT);
        break;
    case LAYER_ABOVE:
        if (is_client)
             m_clients.up_layer(client);
        break;
    case LAYER_BELOW:
        if (is_client)
             m_clients.down_layer(client);
        break;
    case LAYER_TOP:
        if (is_client)
             m_clients.set_layer(client, MAX_LAYER);
        break;
    case LAYER_BOTTOM:
        if (is_client)
             m_clients.set_layer(client, MIN_LAYER);
        break;

#define LAYER_SET(l) case LAYER_##l: \
        if (is_client) m_clients.set_layer(client, l); \
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
            std::string shell = std::string("exec ") + m_config.shell;
            execl("/bin/sh", "/bin/sh", "-c", shell.c_str(), NULL);
            exit(1);
        }
    }
    else if (icon)
    {
        // Any click on an icon, whether or not the action modifier is
        // enabled or not, should deiconify a client
        m_clients.deiconify(icon->client);
    }
    else if (is_client && m_event.xbutton.state == ACTION_MASK)
    {
        if (m_event.xbutton.button != MOVE_BUTTON &&
                m_event.xbutton.button != RESIZE_BUTTON)
            return;

        // A left-click, with the action modifier, start resizing
        if (m_event.xbutton.button == MOVE_BUTTON)
            m_clients.start_moving(m_event.xbutton.subwindow);

        // A right-click, with the action modifier, start resizing
        if (m_event.xbutton.button == RESIZE_BUTTON)
            m_clients.start_resizing(m_event.xbutton.subwindow);
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

    MoveResizeState state = m_xmodel.get_move_resize_state();
    Window client = m_xmodel.get_move_resize_client();

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

    m_xmodel.exit_move_resize();
}

/**
 * Handles windows which have just shown themselves.
 *
 * Note that this can happen for any number of reasons. This method handles
 * the following scenarios:
 *
 *  - A genuinely new client which we want to manage
 *  - A genuinely new client, which happens to be a dialog window
 *  - A window which we aren't interested in managing
 *  - A client which is remapping itself, possibly from another desktop
 */
void XEvents::handle_mapnotify()
{
    Window being_mapped = m_event.xmap.window;

    add_window(being_mapped);
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

    switch (m_xmodel.get_move_resize_state())
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
            Dimension2D pixmap_size = the_icon->gc->copy_pixmap(
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
    the_icon->gc->draw_string(text_x_offset, m_config.icon_height,
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
        delete as_icon;
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

/**
 * Maximizes a client, taking up the whole screen, with the exception of one
 * row of the icon bar.
 * @param window The window to maximize.
 */
void XEvents::maximize_client(Window window)
{

    Dimension scr_width, scr_height;
    m_xdata.get_screen_size(scr_width, scr_height);

    m_clients.change_location(window, 0, m_config.icon_height);
    m_clients.change_size(window, scr_width, scr_height - m_config.icon_height);
}

/**
 * Snaps the client to a particular half of the screen, respecting the icon row.
 * @param window The window to snap.
 * @param side The side of the screen to snap to.
 */
void XEvents::snap_client(Window window, SnapDir side)
{
    Dimension scr_width, scr_height;
    m_xdata.get_screen_size(scr_width, scr_height);

    Dimension workspace_height = scr_height - m_config.icon_height;

    switch (side)
    {
    case K_SNAP_TOP:
        m_clients.change_location(window, 0, m_config.icon_height);
        m_clients.change_size(window, scr_width, workspace_height);
        break;
    case K_SNAP_BOTTOM:
        m_clients.change_location(window, 0, 
            m_config.icon_height + (workspace_height / 2));
        m_clients.change_size(window, scr_width, workspace_height / 2);
        break;
    case K_SNAP_LEFT:
        m_clients.change_location(window, 0, m_config.icon_height);
        m_clients.change_size(window, scr_width / 2, workspace_height);
        break;
    case K_SNAP_RIGHT:
        m_clients.change_location(window, scr_width / 2, m_config.icon_height);
        m_clients.change_size(window, scr_width / 2, workspace_height);
        break;
    }
}

/**
 * Adds a window - this is exposed specifically so that smallwm.cpp can
 * access this method when it imports existing windows.
 *
 * @param window The window to add.
 */
void XEvents::add_window(Window window)
{
    // First, test if this client is already known to us - if it is, then
    // move it onto the current desktop
    if (m_clients.is_client(window))
    {
        Desktop const *mapped_desktop = m_clients.find_desktop(window);

        // Icons must be uniconified
        if (mapped_desktop->is_icon_desktop())
        {
            Icon *icon = m_xmodel.find_icon_from_client(window);
            m_xmodel.unregister_icon(icon);

            m_clients.deiconify(window);
        }

        // Moving/resizing clients must stop being moved/resized
        if (mapped_desktop->is_moving_desktop() || mapped_desktop->is_resizing_desktop())
        {
            Window placeholder = m_xmodel.get_move_resize_placeholder();
            m_xmodel.exit_move_resize();

            XWindowAttributes placeholder_attr;
            m_xdata.get_attributes(placeholder, placeholder_attr);

            if (mapped_desktop->is_moving_desktop())
                m_clients.stop_moving(window, 
                    Dimension2D(placeholder_attr.x, placeholder_attr.y));
            else if (mapped_desktop->is_resizing_desktop())
                m_clients.stop_resizing(window, 
                    Dimension2D(placeholder_attr.width, placeholder_attr.height));
        }

        // Clients which are currently stuck on all desktops don't need to have 
        // anything done to them. Everybody else has to be moved onto the 
        // current desktop.
        if (!mapped_desktop->is_all_desktop())
            m_clients.client_reset_desktop(window);
    }

    // So, this isn't an existing client. We have to figure out now if this is
    // even a client *at all* - override_redirect indicates if this client does
    // (false) or does not (true) want to be managed
    XWindowAttributes win_attr;
    m_xdata.get_attributes(window, win_attr);

    if (win_attr.override_redirect)
        return;

    // This is a new, manageable client - register it with the client database.
    // This requires we know 3 things:
    //  - What the client wants, with regards to its initial state - either
    //    visible or iconified
    //  - The client's position (we know this one)
    //  - The client's size (we know this one too)
    //
    //  The information about the initial state is given by XWMHints
    XWMHints hints;
    bool has_hints = m_xdata.get_wm_hints(window, hints);

    InitialState init_state = IS_VISIBLE;
    if (has_hints && hints.flags & StateHint && 
                     hints.initial_state == IconicState)
        init_state = IS_HIDDEN;

    m_clients.add_client(window, init_state,
            Dimension2D(win_attr.x, win_attr.y), 
            Dimension2D(win_attr.width, win_attr.height));

    // If the client is a dialog, this will be represented in the transient 
    // hint (which is None if the client is not a dialog, or not-None if it is)
    if (m_xdata.get_transient_hint(window) != None)
        m_clients.set_layer(window, DIALOG_LAYER);

    // Finally, execute the actions tied to the window's class
    std::string win_class;
    m_xdata.get_class(window, win_class);

    if (m_config.classactions.count(win_class) > 0 && init_state != IS_HIDDEN)
    {
        ClassActions &action = m_config.classactions[win_class];

        if (action.actions & ACT_STICK)
            m_clients.toggle_stick(window);

        if (action.actions & ACT_MAXIMIZE)
            maximize_client(window);

        if (action.actions & ACT_SETLAYER)
            m_clients.set_layer(window, action.layer);

        if (action.actions & ACT_SNAP)
            snap_client(window, action.snap);
    }
}
