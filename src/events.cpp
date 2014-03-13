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

    KeySym keys[] = {m_keyboard.client_next_desktop, m_keyboard.client_prev_desktop,
        m_keyboard.next_desktop, m_keyboard.prev_desktop, m_keyboard.toggle_stick,
        m_keyboard.iconify, m_keyboard.maximize, m_keyboard.request_close, 
        m_keyboard.force_close, m_keyboard.snap_top, m_keyboard.snap_bottom,
        m_keyboard.snap_left, m_keyboard.snap_right, m_keyboard.layer_above,
        m_keyboard.layer_below, m_keyboard.layer_top, m_keyboard.layer_bottom,
        m_keyboard.exit_wm, m_keyboard.layer1, m_keyboard.layer2, m_keyboard.layer3,
        m_keyboard.layer4, m_keyboard.layer5, m_keyboard.layer6, m_keyboard.layer7,
        m_keyboard.layer8, m_keyboard.layer9, NoSymbol};

    for (KeySym *key = &keys[0]; *key != NoSymbol; key++)
    {
        int keycode = XKeysymToKeycode(m_shared.display, *key);
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
    if (*keysym == m_keyboard.client_next_desktop)
        m_clients.to_next_desktop(client);

    if (*keysym == m_keyboard.client_prev_desktop)
        m_clients.to_prev_desktop(client);

    if (*keysym == m_keyboard.next_desktop)
        m_clients.next_desktop();

    if (*keysym == m_keyboard.prev_desktop)
        m_clients.prev_desktop();

    if (*keysym == m_keyboard.toggle_stick)
        m_clients.flip_sticky_flag(client);

    if (*keysym == m_keyboard.iconify)
        m_clients.state_transition(client, CS_ICON);

    if (*keysym == m_keyboard.maximize)
        m_clients.maximize(client);

    if (*keysym == m_keyboard.request_close)
        m_clients.close(client);

    // Note that m_clients.destroy() is not usable here, since it must be called
    // _after_ the window has been removed
    if (*keysym == m_keyboard.force_close)
        XDestroyWindow(m_shared.display, client);

    if (*keysym == m_keyboard.snap_top)
        m_clients.snap(client, SNAP_TOP);

    if (*keysym == m_keyboard.snap_bottom)
        m_clients.snap(client, SNAP_BOTTOM);

    if (*keysym == m_keyboard.snap_left)
        m_clients.snap(client, SNAP_LEFT);

    if (*keysym == m_keyboard.snap_right)
        m_clients.snap(client, SNAP_RIGHT);

    if (*keysym == m_keyboard.layer_above)
        m_clients.raise_layer(client);

    if (*keysym == m_keyboard.layer_below)
        m_clients.lower_layer(client);

    if (*keysym == m_keyboard.layer_top)
        m_clients.set_layer(client, MAX_LAYER);

    if (*keysym == m_keyboard.layer_bottom)
        m_clients.set_layer(client, MIN_LAYER);

    if (*keysym == m_keyboard.exit_wm)
        m_done = true;

    if (*keysym == m_keyboard.layer1)
        m_clients.set_layer(client, 1);

    if (*keysym == m_keyboard.layer2)
        m_clients.set_layer(client, 2);

    if (*keysym == m_keyboard.layer3)
        m_clients.set_layer(client, 3);

    if (*keysym == m_keyboard.layer4)
        m_clients.set_layer(client, 4);

    if (*keysym == m_keyboard.layer5)
        m_clients.set_layer(client, 5);

    if (*keysym == m_keyboard.layer6)
        m_clients.set_layer(client, 6);

    if (*keysym == m_keyboard.layer7)
        m_clients.set_layer(client, 7);

    if (*keysym == m_keyboard.layer8)
        m_clients.set_layer(client, 8);

    if (*keysym == m_keyboard.layer9)
        m_clients.set_layer(client, 9);
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

    Icon *icon = m_clients.get_icon(m_event.xbutton.subwindow);

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
