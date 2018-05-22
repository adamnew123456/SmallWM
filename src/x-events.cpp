#include "x-events.h"

/**
 * Runs a single iteration of the event loop, by capturing an X event and
 * acting upon it.
 *
 * @return true if more events can be processed, false otherwise.
 */
bool XEvents::step()
{
    // Grab the next event from X, and then dispatch upon its type
    m_xdata.next_event(m_event);

    if (m_event.type == m_xdata.randr_event_offset + RRNotify)
        handle_rrnotify();

    if (m_event.type == KeyPress)
        handle_keypress();

    if (m_event.type == ButtonPress)
        handle_buttonpress();

    if (m_event.type == ButtonRelease)
        handle_buttonrelease();

    if (m_event.type == MotionNotify)
        handle_motionnotify();

    if (m_event.type == ConfigureNotify)
        handle_configurenotify();

    if (m_event.type == MapNotify)
        handle_mapnotify();

    if (m_event.type == UnmapNotify)
        handle_unmapnotify();

    if (m_event.type == Expose)
        handle_expose();

    if (m_event.type == DestroyNotify)
        handle_destroynotify();

    if (m_event.type == ConfigureRequest)
        handle_configurerequest();

    if (m_event.type == MapRequest)
        handle_maprequest();

    if (m_event.type == CirculateRequest)
        handle_circulaterequest();

    return !m_done;
}

/**
 * Rebuilds the display graph whenever XRandR notifies us.
 */
void XEvents::handle_rrnotify()
{
    std::vector<Box> screens;
    m_xdata.get_screen_boxes(screens);
    m_clients.update_screens(screens);
}

/**
 * Handles keyboard shortcuts.
 */
void XEvents::handle_keypress()
{
    KeySym key = m_xdata.get_keysym(m_event.xkey.keycode);
    bool is_using_secondary_action = (m_event.xkey.state & SECONDARY_MASK);

    Window client = None;
    if (m_config.hotkey == HK_MOUSE)
    {
        client = m_event.xkey.subwindow;
        if (client == None)
            client = m_event.xkey.window;
    }
    else if (m_config.hotkey == HK_FOCUS)
        client = m_clients.get_focused();

    bool is_client = m_clients.is_client(client);

    KeyBinding binding(key, is_using_secondary_action);
    KeyboardAction action = m_config.key_commands.binding_to_action[binding];
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
            m_clients.change_mode(client, CPS_MAX);
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
            m_clients.change_mode(client, CPS_SPLIT_TOP);
        break;
    case K_SNAP_BOTTOM:
        if (is_client)
            m_clients.change_mode(client, CPS_SPLIT_BOTTOM);
        break;
    case K_SNAP_LEFT:
        if (is_client)
            m_clients.change_mode(client, CPS_SPLIT_LEFT);
        break;
    case K_SNAP_RIGHT:
        if (is_client)
            m_clients.change_mode(client, CPS_SPLIT_RIGHT);
        break;
    case SCREEN_TOP:
        if (is_client)
            m_clients.to_relative_screen(client, DIR_TOP);
        break;
    case SCREEN_BOTTOM:
        if (is_client)
            m_clients.to_relative_screen(client, DIR_BOTTOM);
        break;
    case SCREEN_LEFT:
        if (is_client)
            m_clients.to_relative_screen(client, DIR_LEFT);
        break;
    case SCREEN_RIGHT:
        if (is_client)
            m_clients.to_relative_screen(client, DIR_RIGHT);
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
    case CYCLE_FOCUS:
        m_clients.cycle_focus_forward();
        break;

    case CYCLE_FOCUS_BACK:
        m_clients.cycle_focus_backward();
        break;

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
    // We have to test both the window and the subwindow, because we might want
    // to route to the parent or the child, depending upon the event
    bool is_client = false;
    bool is_child = false;

    Window parent = None;
    Window child = None;

    if (m_clients.is_client(m_event.xbutton.window))
    {
        is_client = true;
        parent = m_event.xbutton.window;
    }

    if (m_clients.is_client(m_event.xbutton.subwindow))
    {
        is_client = true;
        parent = m_event.xbutton.subwindow;
    }

    if (m_clients.is_child(m_event.xbutton.window))
    {
        is_child = true;
        child = m_event.xbutton.window;
    }

    if (m_clients.is_child(m_event.xbutton.subwindow))
    {
        is_child = true;
        child = m_event.xbutton.subwindow;
    }

    Icon *icon = m_xmodel.find_icon_from_icon_window(m_event.xbutton.window);

    if (!(is_client || is_child || icon)
            && m_event.xbutton.button == LAUNCH_BUTTON
            && m_event.xbutton.state & ACTION_MASK)
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
    else if (is_client && m_event.xbutton.state & ACTION_MASK)
    {
        if (m_event.xbutton.button != MOVE_BUTTON &&
                m_event.xbutton.button != RESIZE_BUTTON)
            return;

        // If we're a packed client, then the user can't move/resize because it
        // would be undone by the packer later
        if (m_clients.is_packed_client(parent))
            return;

        // A left-click, with the action modifier, start resizing
        if (m_event.xbutton.button == MOVE_BUTTON)
            m_clients.start_moving(parent);

        // A right-click, with the action modifier, start resizing
        if (m_event.xbutton.button == RESIZE_BUTTON)
            m_clients.start_resizing(parent);
    }
    else if (is_child) // Any other click on a client focuses that child or client
        m_clients.force_focus(child);
    else if (is_client)
        m_clients.force_focus(parent);
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
}

/**
 * Handles ConfigureNotify events, which updates the size of the window, and
 * re-packs the corner it's on if it's packed.
 */
void XEvents::handle_configurenotify()
{
    Window client = m_event.xconfigure.window;
    if (!m_clients.is_client(client))
        return;

    m_clients.update_size(client,
                          m_event.xconfigure.width,
                          m_event.xconfigure.height);

    if (m_clients.is_packed_client(client))
    {
        PackCorner corner = m_clients.get_pack_corner(client);
        m_clients.repack_corner(corner);
    }
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

    // This has to bypass the expect check, since this needs to happen to every
    // mapped window, and we don't have another way to do this for windows that
    // are (for example) deiconified
    if (m_clients.is_packed_client(being_mapped))
    {
        PackCorner corner = m_clients.get_pack_corner(being_mapped);
        m_clients.repack_corner(corner);
    }

    if (m_xmodel.has_effect(being_mapped, EXPECT_MAP))
    {
        m_xmodel.clear_effect(being_mapped, EXPECT_MAP);
        return;
    }

    add_window(being_mapped);
}

/**
 * This fixes issues where a client that was unmapped but not destroyed
 * would keep the focus (and cause SmallWM's keybindings to break), corrupt
 * the focus cycle, and do other nasty things. In the end, this ensures
 * that the window is unfocused, removed from the focus list, etc.
 */
void XEvents::handle_unmapnotify()
{
    Window being_unmapped = m_event.xmap.window;

    if (m_xmodel.has_effect(being_unmapped, EXPECT_UNMAP))
    {
        m_xmodel.clear_effect(being_unmapped, EXPECT_UNMAP);
        return;
    }

    m_clients.unmap_client(being_unmapped);
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
        bool has_hints = m_xdata.get_wm_hints(the_icon->client, hints);

        if (has_hints && hints.flags & IconPixmapHint)
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
 *
 * Note that ClientModelEvents will do the work of unregistering the client
 * if it is an icon, moving, etc.
 */
void XEvents::handle_destroynotify()
{
    Window destroyed_window = m_event.xdestroywindow.window;

    m_xmodel.remove_all_effects(destroyed_window);

    if (m_clients.is_client(destroyed_window))
    {
        // Ensure to re-pack if we're removing something that should be
        // packed
        bool should_pack = false;
        PackCorner corner;
        if (m_clients.is_packed_client(destroyed_window))
        {
            should_pack = true;
            corner = m_clients.get_pack_corner(destroyed_window);
        }

        m_clients.remove_client(destroyed_window);

        if (should_pack)
            m_clients.repack_corner(corner);
    }

    if (m_clients.is_child(destroyed_window))
        m_clients.remove_child(destroyed_window, true);
}

/**
 * Handles requests from clients to configure themselves. These may or may not
 * be allowed, depending upon the window's current mode and the nature of the
 * request.
 */
void XEvents::handle_configurerequest()
{
    Window client = m_event.xconfigurerequest.window;

    // If we're not dealing with a window we manage, then allow it to do what
    // it wants. Note that we also give nearly free reign to children, since
    // the user can't change their size/position (but we still care about 
    // borders and stacking)
    if (!m_clients.is_client(client))
    {
        int allowed_flags = 
            m_clients.is_child(client) ?
            CWX | CWY | CWWidth | CWHeight :
            0;

        m_xdata.forward_configure_request(m_event, allowed_flags);
        return;
    }

    int modify_flags = m_event.xconfigurerequest.value_mask;
    int allowed_flags = 0;

    int change_pos = modify_flags & CWX || modify_flags & CWY;
    int change_size = modify_flags & CWWidth || modify_flags & CWHeight;
    ClientPosScale cps_mode = m_clients.get_mode(client);
    bool is_packed = m_clients.is_packed_client(client);

    if (change_pos && cps_mode == CPS_FLOATING && !is_packed)
        allowed_flags |= CWX | CWY;

    if (change_size && (cps_mode == CPS_FLOATING || is_packed))
        allowed_flags |= CWWidth | CWHeight;

    modify_flags &= allowed_flags;
    if (modify_flags != 0)
        m_xdata.forward_configure_request(m_event, modify_flags);
}

/**
 * Allows clients to map themselves. This is only necessary because
 * SubstructureRedirectMask includes it.
 */
void XEvents::handle_maprequest()
{
    m_xdata.map_win(m_event.xmaprequest.window);
}

/**
 * Allow these, but only because they should be affecting subwindows instead of
 * nested windows.
 */
void XEvents::handle_circulaterequest()
{
    m_xdata.forward_circulate_request(m_event);
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
            m_clients.deiconify(window);

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

        // Make sure that it can be accessed by the focus cycle again
        m_clients.remap_client(window);

        return;
    }

    // So, this isn't an existing client. We have to figure out now if this is
    // even a client *at all* - override_redirect indicates if this client does
    // (false) or does not (true) want to be managed. Similarly, InputOnly means
    // that the window should never be made visible and should never be focused,
    // so there's nothing we can usefully do to it
    XWindowAttributes win_attr;
    m_xdata.get_attributes(window, win_attr);

    if (win_attr.override_redirect || win_attr.c_class == InputOnly)
        return;

    // If this is a child window, then register it as such
    Window parent = m_xdata.get_transient_hint(window);

    if (parent != None)
    {
        // Make sure that the parent is something that we would also consider 
        // managing
        XWindowAttributes parent_attr;
        m_xdata.get_attributes(parent, parent_attr);

        if (parent_attr.override_redirect || parent_attr.c_class == InputOnly)
            return;

        if (m_clients.is_client(parent))
        {
            m_clients.add_child(parent, window);
            return;
        }
    }

    // At this point, anything we find will have a border
    m_xdata.set_border_width(window, m_config.border_width);

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

    std::string win_class;
    m_xdata.get_class(window, win_class);
    bool should_focus = !contains(m_config.no_autofocus.begin(),
                                  m_config.no_autofocus.end(),
                                  win_class);

    m_clients.add_client(window, init_state,
            Dimension2D(win_attr.x, win_attr.y),
            Dimension2D(win_attr.width, win_attr.height),
            should_focus);

    // Finally, execute the actions tied to the window's class

    if (m_config.classactions.count(win_class) > 0 && init_state != IS_HIDDEN)
    {
        ClassActions &action = m_config.classactions[win_class];

        if (action.actions & ACT_STICK)
            m_clients.toggle_stick(window);

        if (action.actions & ACT_MAXIMIZE)
            m_clients.change_mode(window, CPS_MAX);

        if (action.actions & ACT_SETLAYER)
            m_clients.set_layer(window, action.layer);

        if (action.actions & ACT_SNAP)
        {
            ClientPosScale mode;
            switch (action.snap)
            {
                case DIR_LEFT:
                    mode = CPS_SPLIT_LEFT;
                    break;
                case DIR_RIGHT:
                    mode = CPS_SPLIT_RIGHT;
                    break;
                case DIR_TOP:
                    mode = CPS_SPLIT_TOP;
                    break;
                case DIR_BOTTOM:
                    mode = CPS_SPLIT_BOTTOM;
            }
            m_clients.change_mode(window, mode);
        }

        if (action.actions & ACT_MOVE_X || action.actions & ACT_MOVE_Y)
        {
            // This is exempt from the typical use for screen sizes, which is
            // relative to the window (that is, the screen size is the size of
            // the screen *that the window occupies*). This is because we can't
            // know what screen the user intended the window to be on.
            Box screen = m_clients.get_screen(window);

            m_clients.change_mode(window, CPS_FLOATING);

            Dimension win_x_pos = win_attr.x;
            Dimension win_y_pos = win_attr.y;

            if (action.actions & ACT_MOVE_X)
                win_x_pos = screen.width * action.relative_x;

            if (action.actions & ACT_MOVE_Y)
                win_y_pos = screen.height * action.relative_y;

            if (win_attr.x != win_x_pos || win_attr.x != win_y_pos)
                m_clients.change_location(window, win_x_pos, win_y_pos);
        }

        if (action.actions & ACT_PACK)
            m_clients.pack_client(window, action.pack_corner, action.pack_priority);
    }
}
