/** @file */
#include "events.h"

/**
 * Sets up some member variables, and binds the mouse and keyboard shortcuts.
 * @param shared The shared window manager data
 * @param clients The collection of clients
 * @param keyboard The list of keyboard shortcuts
 * @param randr_offset The X event offset for RandR events
*/
XEvents::XEvents(WMShared &shared, ClientManager &clients, KeyboardConfig &keyboard,
        int randr_offset) :
    m_shared(shared), m_clients(clients), m_randroffset(randr_offset), m_keyboard(keyboard)
{
    int buttons[] = {MOVE_BUTTON, RESIZE_BUTTON, LAUNCH_BUTTON , -1};

    for (int *button = &buttons[0]; *button != -1; button++)
    {
        XGrabButton(m_shared.display, *button, BUTTON_MASK, m_shared.root, true, 
                ButtonPressMask,GrabModeAsync, GrabModeAsync, None, None);
    }

    KeyboardAction actions[] = {
        CLIENT_NEXT_DESKTOP, CLIENT_PREV_DESKTOP,
        NEXT_DESKTOP, PREV_DESKTOP,
        TOGGLE_STICK,
        ICONIFY,
        MAXIMIZE,
        REQUEST_CLOSE, FORCE_CLOSE,
        K_SNAP_TOP, K_SNAP_BOTTOM, K_SNAP_LEFT, K_SNAP_RIGHT,
        LAYER_ABOVE, LAYER_BELOW, LAYER_TOP, LAYER_BOTTOM,
        LAYER_1, LAYER_2, LAYER_3, LAYER_4, LAYER_5, LAYER_6, LAYER_7, LAYER_8, LAYER_9,
        EXIT_WM,
        INVALID_ACTION
    };

    for (KeyboardAction *action = &actions[0]; *action != INVALID_ACTION; action++)
    {
        KeySym key = m_keyboard.bindings[*action];
        int keycode = XKeysymToKeycode(m_shared.display, key);
        XGrabKey(m_shared.display, keycode, BUTTON_MASK, m_shared.root, true, 
                GrabModeAsync, GrabModeAsync);
    }
};

/**
 * Processes all events until the user terminates SmallWM.
 */
void XEvents::run()
{
    m_done = false;
    while (!m_done)
    {
        XNextEvent(m_shared.display, &m_event);

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

        if (m_event.type == ConfigureNotify)
            handle_configurenotify();

        if (m_event.type == DestroyNotify)
            handle_destroynotify();
    }
}

/**
 * Handles keyboard presses, which trigger keyboard shortcuts.
 */
void XEvents::handle_keypress()
{
    int nkeys;
    KeySym *keysym = NULL;
    keysym = XGetKeyboardMapping(m_shared.display, m_event.xkey.keycode, 1, &nkeys);

    Window client = m_event.xkey.subwindow;
    if (client == None)
        client = m_event.xkey.window;

    KeyboardAction action = m_keyboard.reverse_bindings[*keysym];
    switch (action)
    {
    case CLIENT_NEXT_DESKTOP:
        m_clients.to_next_desktop(client);
        break;
    case CLIENT_PREV_DESKTOP:
        m_clients.to_prev_desktop(client);
        break;
    case NEXT_DESKTOP:
        m_clients.next_desktop();
        break;
    case PREV_DESKTOP:
        m_clients.prev_desktop();
        break;
    case TOGGLE_STICK:
        m_clients.flip_sticky_flag(client);
        break;
    case ICONIFY:
        m_clients.state_transition(client, CS_ICON);
        break;
    case MAXIMIZE:
        m_clients.maximize(client);
        break;
    case REQUEST_CLOSE:
        m_clients.close(client);
        break;
    case FORCE_CLOSE:
        // Make sure that the window isn't an icon before we nuke it
        if (!m_clients.get_icon_of_icon(client))
            XDestroyWindow(m_shared.display, client);
        break;
    case K_SNAP_TOP:
        m_clients.snap(client, SNAP_TOP);
        break;
    case K_SNAP_BOTTOM:
        m_clients.snap(client, SNAP_BOTTOM);
        break;
    case K_SNAP_LEFT:
        m_clients.snap(client, SNAP_LEFT);
        break;
    case K_SNAP_RIGHT:
        m_clients.snap(client, SNAP_RIGHT);
        break;
    case LAYER_ABOVE:
        m_clients.raise_layer(client);
        break;
    case LAYER_BELOW:
        m_clients.lower_layer(client);
        break;
    case LAYER_TOP:
        m_clients.set_layer(client, MAX_LAYER);
        break;
    case LAYER_BOTTOM:
        m_clients.set_layer(client, MIN_LAYER);
        break;
    case LAYER_1:
        m_clients.set_layer(client, 10);
        break;
    case LAYER_2:
        m_clients.set_layer(client, 20);
        break;
    case LAYER_3:
        m_clients.set_layer(client, 30);
        break;
    case LAYER_4:
        m_clients.set_layer(client, 40);
        break;
    case LAYER_5:
        m_clients.set_layer(client, 50);
        break;
    case LAYER_6:
        m_clients.set_layer(client, 60);
        break;
    case LAYER_7:
        m_clients.set_layer(client, 70);
        break;
    case LAYER_8:
        m_clients.set_layer(client, 80);
        break;
    case LAYER_9:
        m_clients.set_layer(client, 90);
        break;
    case EXIT_WM:
        m_done = true;
        break;
    }

}

/**
 * Handles a number of things related to clicking:
 *
 *  - Begins moving/resizing when BUTTON_MASK is applied to a client.
 *  - Changes the focus when clicking on an unfocused client (no BUTTON_MASK).
 *  - Shows a previously hidden window (when applied to an icon)
 *  - Launches a shell (when BUTTON_MASK is applied, and on the root window)
 */
void XEvents::handle_buttonpress()
{
    bool is_client = false;
    if (m_clients.is_client(m_event.xbutton.window))
        is_client = true;
    if (m_clients.is_client(m_event.xbutton.subwindow))
        is_client = true;

    Icon *icon = m_clients.get_icon_of_icon(m_event.xbutton.window);

    //  Click on the root window - launch a terminal
    if (!(is_client|| icon) && m_event.xbutton.button == LAUNCH_BUTTON 
            && m_event.xbutton.state == BUTTON_MASK)
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
        m_clients.state_transition(icon->client, CS_ACTIVE);
    }
    else if (is_client && m_event.xbutton.state == BUTTON_MASK)
    {
        if (m_event.xbutton.button == MOVE_BUTTON)
            m_clients.state_transition(m_event.xbutton.subwindow, CS_MOVING);
        if (m_event.xbutton.button == RESIZE_BUTTON)
            m_clients.state_transition(m_event.xbutton.subwindow, CS_RESIZING);
    }
    else
    {
        m_clients.state_transition(m_event.xbutton.window, CS_ACTIVE);
    }
}

/**
 * Stop moving/resizing anything if anything is actually being moved/resized.
 */
void XEvents::handle_buttonrelease()
{
    Window client = m_clients.get_from_placeholder(m_event.xbutton.window);
    
    if (client != None)
        m_clients.state_transition(client, CS_ACTIVE);
}

/**
 * Handle pointer motion by dispatching to the client manager.
 */
void XEvents::handle_motionnotify()
{
    m_clients.handle_motion(m_event);
}

/**
 * Handle the appearance of a new client on the screen.
 */
void XEvents::handle_mapnotify()
{
    m_clients.create(m_event.xmap.window);
}

/**
 * Redraw an icon.
 */
void XEvents::handle_expose()
{
    m_clients.redraw_icon(m_event.xexpose.window);
}

/**
 * ConfigureNotify has _a lot_ of uses - the one we care about is that a window
 * has tried to raise itself. We don't want that to happen, since we need to
 * keep the layers of everything consistent.
 */
void XEvents::handle_configurenotify()
{
    // If we're not managing this window, then don't trigger any updates in
    // response to it
    if (!m_event.xconfigure.override_redirect)
        m_clients.relayer();
}

/**
 * Handle the destruction of a window.
 */
void XEvents::handle_destroynotify()
{
    m_clients.state_transition(m_event.xdestroywindow.window, CS_DESTROY);
}

/**
 * Handles screen resizes and other RandR events.
 */
void XEvents::handle_rrnotify()
{
    int nsizes;
    XRRScreenConfiguration *screen_config = XRRGetScreenInfo(m_shared.display, m_shared.root);
    XRRScreenSize *screen_size = XRRConfigSizes(screen_config, &nsizes);

    m_shared.screen_size = Dimension2D(screen_size->width, screen_size->height);
}
