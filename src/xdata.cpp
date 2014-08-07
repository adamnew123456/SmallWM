/** @file */
#include "xdata.h"

/**
 * Clears the window of the graphics context.
 *
 * (Although this doesn't *require* the graphics context, this function is
 * typically used when drawing, so it fits in well with the rest of the
 * class).
 */
void XGC::clear()
{
    XClearWindow(m_display, m_window);
}

/**
 * Draws a string into the current graphics context.
 * @param x The X coordinate of the left of the text.
 * @param y The Y coordinate of the bottom of the text.
 * @param text The text to draw.
 */
void XGC::draw_string(Dimension x, Dimension y, const std::string &text)
{
    // Although Xlib will handle this for us (passing it a 0 length string
    // will work), don't bother with it if we know it will do nothing.
    if (text.size() == 0)
        return;

    XDrawString(m_display, m_window, m_gc, x, y, text.c_str(), text.size());
}

/**
 * Copies the contents of a pixmap onto this graphics context.
 * @param pixmap The pixmap to copy.
 * @param x The X coordinate of the target area.
 * @param y The Y coordinate of the target area.
 */
Dimension2D XGC::copy_pixmap(Drawable pixmap, Dimension x, Dimension y)
{
    // First, get the size of the pixmap that we're interested in. We need 
    // several other parameters since XGetGeometry is pretty general.
    Window _u1;
    int _u2;
    unsigned int _u3;

    Dimension pix_width, pix_height;
    XGetGeometry(m_display, pixmap, &_u1, &_u2, &_u2,
            &pix_width, &pix_height, &_u3, &_u3);

    XCopyArea(m_display, pixmap, m_window, m_gc, 0, 0, pix_width, pix_height,
        x, y);

    // Return the size of the copied pixmap, since there isn't another way in
    // the XGC definition to get this data
    return Dimension2D(pix_width, pix_height);
}

/**
 * Creates a new graphics context for a given window.
 * @return A new XGC for the given window.
 */
XGC *create_gc(Window window)
{
    return new XGC(m_display, window);
}

/**
 * Creates a new window. Note that it has the following default properties:
 *
 *  - Location at -1, -1.
 *  - Size of 1, 1.
 *  - Border width of 1.
 *  - Black border, with a white background.
 *
 * @param ignore Whether (true) or not (false) SmallWM should ignore the new
 *               window and not treat it as a client.
 * @return The ID of the new window.
 */
Window XData::create_window(bool ignore)
{
    Window win = XCreateSimpleWindow(
        m_display, m_root, 
        -1, -1, // Location
        1, 1, // Size
        1, // Border thickness
        decode_monocolor(X_BLACK),
        decode_monocolor(X_WHITE));

    // Setting the `override_redirect` flag is what SmallWM uses to check for
    // windows it should ignore
    if (ignore)
    {
        XSetWindowAttributes attr;
        attr.override_redirect = true;
        set_attributes(win, attr, CWOverrideRedirect);
    }

    return window;
}

/**
 * Changes the property on a window.
 * @param window The window to change the property of.
 * @param prop The name of the property to change.
 * @parm type The type of the property to change.
 * @param value The raw value of the property.
 * @param elems The length of the value of the property.
 */
void XData::change_property(Window window, const std::string &prop,
        Atom type, const unsigned char *value, size_t elems)
{
    XChangeProperty(m_display, window, intern_if_needed(prop),
            type, 32, PropModeReplace, value, elems);
}

/**
 * Gets the next event from the X server.
 * @param[in] event The place to store the event.
 */
void XData::next_event(XData &data)
{
    XNextEvent(m_dispplay, &data);
}

/**
 * Gets the latest event of a given type.
 * @param[in] event The place to store the event.
 * @param type The type of the event to iterate through.
 */
void XData::get_latest_event(XData &data, int type)
{
    while (XCheckTypedEvent(m_display, type, &data));
}

/**
 * Adds a new hotkey - this means that the given key (plus the default
 * modifier) registers an event no matter where it is pressed.
 * @param key The key to bind.
 */
void XData::add_hotkey(KeySym key)
{
    // X grabs on keycodes, not on KeySyms, so we have to do the conversion
    int keycode = XKeysymToKeycode(m_display, key);

    XGrabKey(m_display, keycode, WM_MODIFIER, m_root, true, 
        GrabModeAsync, GrabModeAsync);
}

/**
 * Binds a mouse button to raise an event globally.
 * @param button The button to bind (1 is left, 3 is right, etc.)
 */
void XData::add_hotkey_mouse(unsigned int button)
{
    XGrabButton(m_display, button, WM_MODIFIER, m_root, true,
            ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None);
}

/**
 * Confines a pointer to a window, allowing ButtonPress and ButtonRelease
 * events from the window.
 * @param window The window to confine the pointer to.
 */
void XData::confine_pointer(Window window)
{
    XGrabButton(m_display, AnyButton, AnyModifier, window, true,
            ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None);
}

/**
 * Stops confining the pointer to the window.
 * @param winodw The window to release.
 */
void XData::stop_confining_pointer(Window window)
{
    XUngrabButton(m_display, AnyButton, AnyModifie, window);
}

/**
 * Selects the input mask on a given window.
 * @param window The window to set the mask of.
 * @param mask The input mask.
 */
void XData::select_input(Window window, long mask)
{
    XSelectInput(m_display, window, mask);
}

/**
 * Gets a list of top-level windows on the display.
 * @param[out] windows The vector to put the windows into.
 */
void XData::get_windows(std::vector<Window> &windows)
{
    Window _unused1;
    Window *children;
    unsigned int nchildren;

    XQueryTree(m_display, m_root, &_unused1, &_unused1,
        &children, &nchildren);
    for (int idx = 0; idx < nchildren; idx++)
    {
        if (children[idx] != m_root)
            windows.push(children[idx]);
    }

    XFree(children);
}

/**
 * Gets the absolute location of the pointer.
 * @param[out] x The X location of the pointer.
 * @param[out] y The Y location of the pointer.
 */
void XData::get_pointer_location(Dimension &x, Dimension &y)
{
    Window _u1;
    int _u2;
    unsigned int _u3;
    XQueryPointer(m_shared.display, m_shared.root, &_u1, &_u1, 
            &x, &y, &_u2, &_u2, &_u3);
}

/**
 * Gets the current input focus.
 * @return The currently focused window.
 */
Window XData::get_input_focus()
{
    Window new_focus;
    int _unused;

    XGetInputFocus(m_shared.display, &new_focus, &_unused);
    return new_focus;
}

/**
 * Sets the input focus,  
 * @param window The window to set the focus of.
 * @return true if the change succeeded or false otherwise.
 */
bool XData::set_input_focus(Window window)
{
    XSetInputFocus(m_display, window, RevertToNone, CurrentTime);
    return get_input_focus() == window;
}

/**
 * Maps a window onto the screen, causing it to be displayed.
 * @param window The window to map.
 */
void XData::map_win(Window window)
{
    XMapWindow(m_display, window);
}

/**
 * Unmaps a window, causing it to no longer be displayed.
 * @param window The window to unmap.
 */
void XData::unmap_win(Window window)
{
    XUnmapWindow(m_display, window);
}

/**
 * Requests a window to close using the WM_DELETE_WINDOW message, as specified
 * by the ICCCM.
 * @param window The window to close.
 */
void XData::request_close(Window window)
{
    XEvent close_event;
    XClientMessageEvent client_close;
    client_close.type = ClientMessage;
    client_close.window = window;
    client_close.message_type = intern_if_needed("WM_PROTOCOLS");
    client_close.format = 32;
    client_close.data.l[0] = intern_if_needed("WM_DELETE_WINDOW");
    client_close.data.l[1] = CurrentTime;

    close_event.xclient = client_close;
    XSendEvent(m_display, window, False, NoEventMask, &close_event);
}

/**
 * Destroys a window.
 * @param window The window to destroy.
 */
void XData::destroy_win(Window window)
{
    XDestroyWindow(m_display, window);
}

/**
 * Gets the attributes of a window.
 * @param window The window to get the attributes of.
 * @param[out] attr The storage for the attributes.
 */
void XData::get_attributes(Window window, XWindowAttributes &attr)
{
    XGetWindowAttributes(m_display, window, &attr);
}

/**
 * Sets the attributes of a window.
 * @param window The window to set the attributes of.
 * @param attr The values to set as the attributes.
 * @param flag Which attributes are being changed.
 */
void XData::set_attributes(Window window, const XSetWindowAttributes &attr,
        unsigned long mask)
{
    XChangeWindowAttributes(m_display, window, mask, &attr);
}

/**
 * Sets the color of the border of a window.
 * @param window The window whose border to set.
 * @param color The border color.
 */
void XData::set_border_color(Window window, MonoColor color)
{
    XSetWindowBorder(m_display, window, decode_monocolor(color));
}

/**
 * Sets the width of the border of a window.
 * @param window The window whose border to change.
 * @param size The size of the window's border.
 */
void XData::set_border_width(Window window, Dimension size)
{
    XSetWindowBorderWidth(window, size);
}

/**
 * Moves a window from its current location to the given location.
 * @param window The window to move.
 * @param x The X coordinate of the window's new position.
 * @param y The Y coordinate of the window's new position.
 */
void XData::move_window(Window window, int x, int y)
{
    XMoveWindow(m_display, window, x, y);
}

/**
 * Resizes a window from its current size to the given size.
 * @param window The window to resize.
 * @param width The width of the window's new size.
 * @param height The height of the window's new size.
 */
void XData::resize_window(Window window, Dimension width, Dimension height)
{
    XResizeWindow(m_display, window, width, height);
}

/**
 * Raises a window to the top of the stack.
 * @param window The window to raise.
 */
void XData::raise(Window window)
{
    XRaiseWindow(m_display, window);
}

/**
 * Stacks a series of windows.
 * @param windows The windows to stack, in top-to-bottom order.
 */
void XData::restack(const std::vector<Window> &windows)
{
    XRestackWindow(m_display, windows.begin(), windows.size());
}

/**
 * Gets the XWMHints structure corresponding to the given window.
 * @param window The window to get the hints for.
 * @param[out] hints The storage for the hints.
 */
void XData::get_wm_hints(Window window, XWMHints &hints)
{
    XWMHints *returned_hints = XGetWMHints(m_display, window);

    // Since we have to get rid of this later, and it is an unnecessary
    // complication to return it, we'll just copy it and get rid of the
    // pointer that was returned to us
    std::memcpy(&hints, returned_hints, sizeof(XWMHints));

    XFree(returned_hints);
}

/***
 * Gets the XSizeHints structure corresponding to the given window.
 * @param window THe window to get the hints for.
 * @param[out] hints The storage for the hints.
 */
void XData::get_size_hints(Window window, XSizeHints &hints)
{
    long _u1;
    XGetWMNormalHints(m_display, window, &hints, &_u1);
}

/**
 * Gets the transient hint for a window - a window which is transient for
 * another is assumed to be some form of dialog window.
 * @param window The window to get the hints for.
 * @return The window that the given window is transient for.
 */
Window XData::get_transient_hint(Window window)
{
    Window transient = None;
    XGetTransientForHint(m_display, window, &transient);
    return transient;
}

/**
 * Gets the name of a window. Note that a window can have multiple names,
 * and thus this function tries to pick the most appropriate one for use as
 * an icon.
 * @param window The window to get the name of.
 * @param[out] name The name of the window.
 */
void XData::get_icon_name(Window window, std::string name)
{
    char *icon_name;
    XGetIconName(m_display, window, &icon_name)
    if (icon_name)
    {
        name.assign(icon_name);
        XFree(icon_name);
        return;
    }
   
    XFetchName(m_shared.display, icon->client, &icon_name)
    
    if (icon_name)
    {
        name.assign(icon_name);
        XFree(icon_name);
        return;
    }

    name.clear();
}

/**
 * Gets the window's "class" (an X term, not mine), a text string which is mean
 * to uniquely identify what application a window is being created by.
 * @param windwo The window to get the class of.
 * @param[out] xclass The X class of the window.
 */
void XData::get_class(Window win, std::string &xclass)
{
    XClassHint hint;
    XGetClassHint(m_display, win, &hint);
    
    if (hint.res_name)
        XFree(hint.res_name);

    
    if (hint.res_class)
    {
        name.assign(hint.res_class)
        XFree(hint.res_class);
    }
    else
        name.clear();
}

/**
 * Gets the current screen size.
 * @param[out] width The width of the screen.
 * @param[out] height The height of the screen.
 */
void XData::get_screen_size(Dimension &width, Dimension &height)
{
    width = DIM2D_WIDTH(m_screen_size);
    height = DIM2D_HEIGHT(m_screen_size);
}

/**
 * Updates the current screen size by querying X RandR.
 */
void XData::update_screen_size()
{
    XRRScreenConfiguration *screen_config = XRRGetScreenInfo(m_display,
        m_root);
   
    // First, we have to retrieve the list of all *possible* sizes, and then
    // pick out the current size from that
    int _;
    XRRScreenSize *sizes = XRRConfigSizes(screen_config, &_);

    Rotation __;
    SizeID current_size_idx = XRRCurrentConfiguration(screen_config, &__);

    DIM2D_WIDTH(m_screen_size) = sizes[current_size_idx].width;
    DIM2D_HEIGHT(m_screen_size) = size[current_size_idx].height;
}

/**
 * Converts from a raw keycode into a KeySym.
 * @param keycode The raw keycode given by X.
 * @return The KeySym represented by that keycode.
 */
KeySym XData::get_keysym(int keycode)
{
    KeySym *possible_keysyms;
    int keysyms_per_keycode;

    possible_keysyms = XGetKeyboardMapping(m_display, keycode, 1,
        &keysyms_per_keycode);

    // The man pages don't explicitly say if this is a possibility, so
    // protect against it just in case
    if (!keysyms_per_keycode)
        return NoSymbol;

    KeySym result = possible_keysyms[0];
    XFree(possible_keysyms);

    return result;
}

/**
 * Interns an string, converting it into an atom and caching it. On
 * subsequent calls, the cache is used instead of going through Xlib.
 * @param atom The name of the atom to convert.
 * @return The converted atom.
 */
Atom XData::intern_if_needed(const std::string &atom_name)
{
    if (m_atoms.count(atom_name) > 0)
        return m_atoms[atom_name];

    Atom the_atom = XInternAtom(m_display, atom_name.c_str(), false);
    m_atoms[atom_name] = the_atom;
    return the_atom;
}

/**
 * Converts a MonoColor into an Xlib color.
 * @param color The MonoColor to convert from.
 * @return The equivalent Xlib color.
 */
unsigned long XData::decode_multicolor(MonoColor color)
{
    switch (color)
    {
        case X_BLACK:
            return BlackPixel(m_display, m_screen);
        case X_WHITE:
            return WhitePixel(m_display, m_screen);
    }
}
