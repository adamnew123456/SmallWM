/** @file */
#include "client.h"

/**
 * Constructs a new client given the X11 window this client will own.
 */
Client::Client(const WMShared &shared, ClientManager &manager, 
        const ClassActions &actions, Window win) :
    m_shared(shared), m_manager(manager), m_actions(actions), m_window(win)
{
    // Add the border "decoration" to the client window (a simple white border)
    XSetWindowBorder(m_shared.display, m_window,
            WhitePixel(m_shared.display, m_shared.screen));
    XSetWindowBorderWidth(m_shared.display, m_window, shared.border_width);
}

/**
 * Removes this client from the X11 server. Better to have a client destroyed, 
 * so that the problem can be debugged rather than have orphaned windows sitting
 * around.
 */
Client::~Client()
{
    destroy();
}

/**
 * Gets the window that belongs to this client.
 */
Window Client::window() const
{
    return m_window;
}

/**
 * Snaps this client to a particular half of the screen.
 * @param side The side of the screen to snap this client toward.
 */
void Client::snap(SnapDir side)
{   
    Dimension x, y, w, h;
    Dimension icon_height = std::get<1>(m_shared.icon_size);
    Dimension screen_width = std::get<0>(m_shared.screen_size),
              screen_height = std::get<1>(m_shared.screen_size) - icon_height;
    switch (side)
    {
        case SNAP_TOP:
            x = 0;
            y = icon_height;
            w = screen_width;
            h = screen_height / 2;
            break;
        case SNAP_BOTTOM:
            x = 0;
            y = (screen_height / 2) + icon_height;
            w = screen_width;
            h = screen_height / 2;
            break;
        case SNAP_LEFT:
            x = 0;
            y = icon_height;
            w = screen_width / 2;
            h = screen_height;
            break;
        case SNAP_RIGHT:
            x = screen_width / 2;
            y = icon_height;
            w = screen_width / 2;
            h = screen_height;
            break;
    }

    XMoveResizeWindow(m_shared.display, m_window, x, y, w, h);
}

/**
 * Maximizes this client to use all of the screen, except the icon row.
 */
void Client::maximize()
{
    Dimension icon_height = std::get<1>(m_shared.icon_size);
    Dimension screen_width = std::get<0>(m_shared.screen_size),
              screen_height = std::get<1>(m_shared.screen_size) - icon_height;
    
    XMoveResizeWindow(m_shared.display, m_window, 0, icon_height, screen_width, screen_height);
}

/**
 * Does all the actions specified by this client's ClassActions.
 * @param self The representation of this object that the ClientManager holds.
 */
void Client::run_actions(ClientRef self)
{
    if (m_actions.actions & static_cast<unsigned int>(ACT_STICK))
        m_manager.desktops().flip_sticky_flag(self);

    if (m_actions.actions & static_cast<unsigned int>(ACT_MAXIMIZE))
        maximize();

    if (m_actions.actions & static_cast<unsigned int>(ACT_SETLAYER))
        m_manager.layers().set_layer(self, m_actions.layer);

    if (m_actions.actions & static_cast<unsigned int>(ACT_SNAP))
        snap(m_actions.snap);
}

/**
 * Requests that this client close, in the polite ICCCM fashion.
 */
void Client::close()
{
    // Being nice like this allows programs to show their typical closure dialogs
    XEvent close_event;
    XClientMessageEvent client_close = {
        .type = ClientMessage,
        .window = m_window,
        .message_type = static_cast<unsigned long>(
                XInternAtom(m_shared.display, "WM_PROTOCOLS", false)),
        .format = 32,
        .data = {
            .l = { static_cast<long>(
                    XInternAtom(m_shared.display, "WM_DELETE_WINDOW", false)),
                   CurrentTime }
        }
    };

    close_event.xclient = client_close;
    XSendEvent(m_shared.display, m_window, false, NoEventMask, &close_event);
}

/**
 * Forces this client to close - this really only happens at the user's request,
 * and is not generally used by the WM on its own since this can lead crash the
 * program, lead to possible data loss ("Do you want to save?"), etc.
 */
void Client::destroy()
{
    MoveResizeManager &mvr = m_manager.moveresize();
    FocusManager &focus = m_manager.focus();

    XDestroyWindow(m_shared.display, m_window);
}

/**
 * Sets the layer of a client, as long as it is not already a dialog.
 * @param client The client to set the layer of
 * @param layer The layer to put the client on
 */
void LayerManager::set_layer(ClientRef client, Layer layer)
{
    if (m_layers[client] != DIALOG_LAYER)
        m_layers[client] = layer;
}

/**
 * Moves a client up to the next layer.
 * @param client The client to move up.
 */
void LayerManager::move_up(ClientRef client)
{
    if (m_layers[client] != DIALOG_LAYER &&
            m_layers[client] != MAX_LAYER)
        m_layers[client]++;
}

/**
 * Moves a client down to the previous layer.
 * @param client The client to move down.
 */
void LayerManager::move_down(ClientRef client)
{
    if (m_layers[client] != DIALOG_LAYER &&
            m_layers[client] != MIN_LAYER)
        m_layers[client]--;
}

/**
 * Configures a client as a dialog.
 * @param client The client to set as a dialog.
 */
void LayerManager::set_as_dialog(ClientRef client)
{
    m_layers[client] = DIALOG_LAYER;
}

/**
 * A wrapper around the layer sorter, which is required because std::find doesn't
 * take user data. In this case, the map between client and layer is needed to
 * sort clients by their layer.
 */
class ClientLayerSorter {
public:
    /// Initialize the client-to-layers mapping.
    ClientLayerSorter(const std::map<ClientRef, Layer> &layers) :
        m_layers(layers)
    {};

    /**
     * Sorts two clients by layer. Note that Xlib requires that windows be in
     * stacking order - from top to bottom - so this comparison function is actually
     * backwards.
     * @param a The first client.
     * @param b The second client.
     * @return Whether or not the first client is above the second.
     */
    bool operator()(ClientRef a, ClientRef b)
    {
        Layer a_layer = m_layers.at(a);
        Layer b_layer = m_layers.at(b);
        return a_layer > b_layer;
    };

private:
    const std::map<ClientRef, Layer> &m_layers;
};

/**
 * Gets all the clients and stacks them properly.
 */
void LayerManager::relayer_clients()
{
    std::vector<ClientRef> client_stacking;
    for (std::map<ClientRef,Layer>::iterator client_iter = m_layers.begin();
            client_iter != m_layers.end();
            client_iter++)
    {
        client_stacking.push_back(client_iter->first);
    }

    ClientLayerSorter sorter(m_layers);
    std::sort(client_stacking.begin(), client_stacking.end(), sorter);

    std::vector<Window> window_stacking;
    for (std::vector<ClientRef>::iterator client_iter = client_stacking.begin();
            client_iter != client_stacking.end();
            client_iter++)
    {
        window_stacking.push_back(client_iter->get()->window());
    }

    XRestackWindows(m_shared.display, &window_stacking[0], window_stacking.size());

    // Ensure that, if a window is being moved/resized, its placeholder is not
    // obscured by other windows
    m_manager.moveresize().raise_placeholder();
}

/**
 * Removes a client from this manager.
 * @param client The client to remove.
 */
void LayerManager::remove(ClientRef client)
{
    m_layers.erase(client);
}

/**
 * Sets the desktop that a client is shown on.
 * @param client The client to move.
 * @param desktop The desktop to move the client to.
 */
void DesktopManager::set_desktop(ClientRef client, Desktop desktop)
{
    if (desktop == THIS_DESKTOP)
        m_desktops[client] = m_current_desktop;
    else
        m_desktops[client] = desktop;
    redraw_clients();
}

/**
 * Moves a client to the desktop after its current one.
 * @param client The client to move.
 */
void DesktopManager::next_desktop(ClientRef client)
{
    if (m_desktops[client] == m_shared.max_desktops)
        m_desktops[client] = 1;
    else
        m_desktops[client]++;

    redraw_clients();
}

/**
 * Moves a client to the desktop before its current one.
 * @param client The client to move.
 */
void DesktopManager::prev_desktop(ClientRef client)
{
    if (m_desktops[client] == 1)
        m_desktops[client] = m_shared.max_desktops;
    else
        m_desktops[client]--;

    redraw_clients();
}

/**
 * Unsticks a sticky client, or sticks an unsticky client.
 * @param client The client to (un)stick.
 */
void DesktopManager::flip_sticky_flag(ClientRef client)
{
    m_stickies[client] = !m_stickies[client];
}

/**
 * Changes the current desktop to another desktop.
 * @param desktop The desktop to change to.
 */
void DesktopManager::change_desktop(Desktop desktop)
{
    if (m_current_desktop != desktop)
    {
        m_current_desktop = desktop;
        redraw_clients();
    }
}

/**
 * Takes all the clients and relayers them in the correct order.
 */
void DesktopManager::redraw_clients()
{
    XWindowAttributes attr;
    for (std::map<ClientRef, Desktop>::iterator client_iter = m_desktops.begin();
            client_iter != m_desktops.end();
            client_iter++)
    {
        ClientRef client_ref = client_iter->first;
        Window client_win = client_ref->window();
        XGetWindowAttributes(m_shared.display, client_win, &attr);

        bool is_visible = 
                (m_desktops[client_ref] == m_current_desktop) ||
                m_stickies[client_ref];

        if (is_visible && attr.map_state != IsViewable)
            XMapWindow(m_shared.display, client_win);

        if (is_visible && attr.map_state == IsViewable)
            XUnmapWindow(m_shared.display, client_win);
    }
}

/**
 * Removes a client from this manager.
 * @param client The client to remove.
 */
void DesktopManager::remove(ClientRef client)
{
    m_desktops.erase(client);
    m_stickies.erase(client);
}

/**
 * Begins moving a particular client's window.
 * @param client The client whose window should be moved.
 * @param ptr_x The absolute x position of the mouse pointer.
 * @param ptr_y The absolute y position of the mouse pointer.
 */
void MoveResizeManager::begin_move(ClientRef client, Dimension ptr_x, Dimension ptr_y)
{
    if (m_state != MVR_NONE)
        return;

    m_state = MVR_MOVE;
    m_ptr_loc = Dimension2D(ptr_x, ptr_y);
    m_client = client;

    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, client->window(), &attr);
    m_old_params = Dimension2D(attr.x, attr.y);

    create_placeholder(client->window(), attr);
}

/**
 * Begins resizing a particular client's window.
 * @param client The client whose window should be resized.
 * @param ptr_x The absolute x position of the mouse pointer.
 * @param ptr_y The absolute y position of the mouse pointer.
 */
void MoveResizeManager::begin_resize(ClientRef client, Dimension ptr_x, Dimension ptr_y)
{
    if (m_state != MVR_NONE)
        return;

    m_state = MVR_RESIZE;
    m_ptr_loc = Dimension2D(ptr_x, ptr_y);
    m_client = client;

    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, client->window(), &attr);

    create_placeholder(client->window(), attr);
}

/**
 * Handles an X11 pointer motion event.
 * @param event The X11 MotionNotify event.
 */
void MoveResizeManager::handle_motion_event(const XEvent &event)
{
    if (m_state == MVR_NONE)
        return;
     
    // Get the most recent event, instead of handling them all individually
    XEvent latest = event;
    while (XCheckTypedEvent(m_shared.display, MotionNotify, &latest));

    // Figure out where the pointer is now, compared to where it originally was
    Dimension xdiff = latest.xmotion.x_root - std::get<0>(m_ptr_loc),
              ydiff = latest.xmotion.y_root - std::get<1>(m_ptr_loc);

    Dimension new_x = xdiff + std::get<0>(m_old_params),
            new_y = ydiff + std::get<1>(m_old_params);

    switch (m_state)
    {
    case MVR_NONE: /* This is only to appease clang, since it's not smart enough
                      to understand that this case was handled by the above guard
                      clause. */
        break;
    case MVR_MOVE:
        XMoveWindow(m_shared.display, m_placeholder, new_x, new_y);
        break;

    case MVR_RESIZE:
        new_x = std::max<Dimension>(new_x, 1);
        new_y = std::max<Dimension>(new_y, 1);
        XResizeWindow(m_shared.display, m_placeholder, new_x, new_y);
    }
}

/**
 * Finishes the current move/resize operation.
 */
void MoveResizeManager::end_move_resize()
{
    if (m_state == MVR_NONE)
        return;

    // Find out where the placeholder ended up, so that information can be
    // applied to the client.
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, m_placeholder, &attr);
    XDestroyWindow(m_shared.display, m_placeholder);
    m_placeholder = None;

    XMapWindow(m_shared.display, m_client->window());
    XMoveResizeWindow(m_shared.display, m_client->window(), 
            attr.x, attr.y, attr.width, attr.height);
}

/**
 * Check and see if a particular window is being moved or resized.
 * @param window The window to check.
 * @return Whether or not the given window is being moved or resized.
 */
bool MoveResizeManager::is_being_moved(Window window) const
{
    return window == m_client->window();
}

/**
 * Raises the placeholder window to the top of the stack.
 *
 * This is necessary, in case a window is relayered or a new window is created,
 * and the LayerManager relayers the windows. It would be bad to let another window
 * obscure the placeholder, so keep it on top at all times.
 */
void MoveResizeManager::raise_placeholder() const
{
    if (m_placeholder != None)
        XRaiseWindow(m_shared.display, m_placeholder);
}

/**
 * Creates a placeholder window which is used to resize and move windows 
 * instead of moving the window itself.
 * @param window The window to give the placeholder for.
 * @param attr The parameters of the original window.
 */
void MoveResizeManager::create_placeholder(Window window, XWindowAttributes attr)
{   
    XUnmapWindow(m_shared.display, window);

    m_placeholder = XCreateSimpleWindow(m_shared.display, m_shared.root,
            attr.x, attr.y, attr.width, attr.height, 1,
            BlackPixel(m_shared.display, m_shared.screen),
            WhitePixel(m_shared.display, m_shared.screen));

    // Ignore this window by marking it as 'override_redirect'
    XSetWindowAttributes set_attr;
    set_attr.override_redirect = true;
    XChangeWindowAttributes(m_shared.display, m_placeholder, 
            CWOverrideRedirect, &set_attr);

    XMapWindow(m_shared.display, m_placeholder);
    XGrabPointer(m_shared.display, m_placeholder, true, 
            PointerMotionMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
            None, None, CurrentTime);
    raise_placeholder();
}

/**
 * Sets the input focus to a particular window.
 * @param window The window to focus.
 */
void FocusManager::change_focus(Window window)
{
    /*
     * A brief overview - this was something I borrowed from 9wm, which is used
     * to do a very simple focus-on-click style of window managment.
     *
     * A "haze" is defined as a pointer grab that exists on a window, which
     * prevents it from recieving clicks, but instead routes those clicks to
     * the window manager. "Breaking the haze" refers to the act of cliking on
     * a window with a "haze", while "hazing a window" refers to adding a pointer
     * grab onto a window.
     */
   
    // Add a haze to whatever window was previously focused, so that if this
    // operation fails then all windows will be hazed.
    unfocus();

    // This is done to make sure that the window actually accepts the input focus.
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, window, &attr);

    if (attr.c_class != InputOnly && attr.map_state == IsViewable)
    {
        // Remove the haze from the window which we are focusing.
        XUngrabButton(m_shared.display, AnyButton, AnyModifier, window);

        // Then, transfer the input and verify that the input was transferred.
        XSetInputFocus(m_shared.display, AnyButton, AnyModifier, window);

        Window new_focus;
        int _unused;

        XGetInputFocus(m_shared.display, &new_focus, &_unused);
        if (new_focus != window)
        {
            // The focus operation failed, and the haze must be restored.
            XGrabButton(m_shared.display, AnyButton, AnyModifier, window, false,
                    ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
                    None, None);
            m_focused = None;
        }
        else
            m_focused = window;
    }
}

/**
 * Removes the focus from the currently focused window, if possible.
 */
void FocusManager::unfocus()
{
    if (m_focused != None)
        XGrabButton(m_shared.display, AnyButton, AnyModifier, m_focused, false,
                ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync,
                None, None);
}

/**
 * Gets the currently focused window.
 */
Window FocusManager::get_focused() const
{
    return m_focused;
}

/**
 * Associates a ClassActions from the config file to a named window class.
 * @param cls The X11 class to apply the actions to.
 * @param actions The actions themselves.
 */
void ClientManager::register_classaction(std::string cls, ClassActions actions)
{
    m_actions[cls] = actions;
}

/**
 * Creates a new client and registers it.
 * @param window The X11 window to create the client from.
 */
void ClientManager::create_client(Window window)
{
    /*
     * There are a number of things to assert if this window is actually a
     * valid Client or not.
     *
     * 1. Does it _want_ to be managed?
     * 2. Is it a dialog or not?
     * 3. Is this manager _already_ managing it?
     */
    
    // Step 1.
    XWindowAttributes attr;
    XGetWindowAttributes(m_shared.display, window, &attr);

    if (attr.override_redirect)
    {
        // Do not manage these windows - they are things like combo boxes, etc.
        // which are private to applications.
        return;
    }

    // Step 2.
    bool is_transient = false;
    Window transient_for;
    if (XGetTransientForHint(m_shared.display, window, &transient_for) && transient_for != None)
    {
        // Dialogs are a special kind of client - give them a Client, but let the
        // Cliet know that this is a special kind of window
        is_transient = true;
    }

    // Step 3.
    if (m_clients.count(window) > 0)
        return;

    // If all these conditions are satisfied, then create the Client
    std::string client_class;

    XClassHint *classhint = XAllocClassHint();
    {   // The braces are here just to make the allocation more explicit
        XGetClassHint(m_shared.display, window, classhint);
        if (!classhint->res_class)
            client_class = std::string("");
        else
            client_class = std::string(classhint->res_class);
    }
    XFree(classhint);

    Client *client = new Client(m_shared, *this, m_actions[client_class], window);
    ClientRef cliref = ClientRef(client);
    m_clients[window] = cliref;

    // Register the new client with all of the submanagers
    m_layers.set_layer(cliref, is_transient ? DIALOG_LAYER : DEF_LAYER);
    m_desktops.set_desktop(cliref, THIS_DESKTOP);

    if (is_transient)
        m_layers.set_as_dialog(cliref);

    // Do all the things that this client needs to do when it is created, except
    // if it is a dialog (since not all actions apply to dialogs).
    if (is_transient)
        client->run_actions(cliref);

    // Now, relayer the clients and focus this window.
    m_layers.relayer_clients();
    m_focus.change_focus(window);
}

/**
 * Gets a client based on the X11 window.
 * @param window The window that a client manages.
 * @return A ClientRef which references either a Client or NULL.
 */
ClientRef ClientManager::get_client(Window window)
{
    return m_clients[window];
}

/**
 * Removes a client, given its X11 window.
 * @param window The window that a client manages.
 */
void ClientManager::remove_client(Window window)
{
    if (m_focus.get_focused() == window)
        m_focus.unfocus();

    if (m_moveresize.is_being_moved(window))
        m_moveresize.end_move_resize();

    ClientRef client = get_client(window);
    m_desktops.remove(client);
    m_layers.remove(client);

    m_clients.erase(window);
}
