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

    unsigned int pix_width, pix_height;
    XGetGeometry(m_display, pixmap, &_u1, &_u2, &_u2,
            &pix_width, &pix_height, &_u3, &_u3);

    XCopyArea(m_display, pixmap, m_window, m_gc, 0, 0, pix_width, pix_height,
        x, y);

    // Return the size of the copied pixmap, since there isn't another way in
    // the XGC definition to get this data
    return Dimension2D(pix_width, pix_height);
}

/**
 * Initializes XRandR on the current display.
 *
 * Note that SmallWM *depends* upon XRandR support, so it will die if it is not
 * present.
 */
void XData::init_xrandr()
{
    int _;
    bool randr_state = XRRQueryExtension(m_display, &randr_event_offset, &_);

    if (randr_state == false)
    {
        m_logger.set_priority(LOG_ERR) <<
            "Unable to initialize XRandR extension - terminating" << SysLog::endl;
        m_logger.stop();

        std::exit(1);
    }

    // Version 1.4 is about 2 years, so even though it probably has more
    // than we require, it seems like a good starting point
    int major_version = 1, minor_version = 4;
    XRRQueryVersion(m_display, &major_version, &minor_version);
}

/**
 * Creates a new graphics context for a given window.
 * @return A new XGC for the given window.
 */
XGC *XData::create_gc(Window window)
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

    return win;
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
void XData::next_event(XEvent &data)
{
    XNextEvent(m_display, &data);
}

/**
 * Gets the latest event of a given type.
 * @param[in] event The place to store the event.
 * @param type The type of the event to iterate through.
 */
void XData::get_latest_event(XEvent &data, int type)
{
    while (XCheckTypedEvent(m_display, type, &data));
}

/**
 * Adds a new hotkey - this means that the given key (plus the default
 * modifier) registers an event no matter where it is pressed.
 * @param key The key to bind.
 */
void XData::add_hotkey(KeySym key, bool use_secondary_action)
{
    // X grabs on keycodes, not on KeySyms, so we have to do the conversion
    int keycode = XKeysymToKeycode(m_display, key);

    int mask = ACTION_MASK;
    if (use_secondary_action)
        mask |= SECONDARY_MASK;

    XGrabKey(m_display, keycode, mask, m_root, true, 
        GrabModeAsync, GrabModeAsync);
}

/**
 * Binds a mouse button to raise an event globally.
 * @param button The button to bind (1 is left, 3 is right, etc.)
 */
void XData::add_hotkey_mouse(unsigned int button)
{
    XGrabButton(m_display, button, ACTION_MASK, m_root, true,
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
    if (m_confined == None)
    {
        XGrabPointer(m_display, window, false,
            PointerMotionMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync,
            None, None, CurrentTime);
        m_confined = window;
    }
}

/**
 * Stops confining the pointer to the window.
 * @param winodw The window to release.
 */
void XData::stop_confining_pointer()
{
    if (m_confined != None)
    {
        XUngrabButton(m_display, AnyButton, AnyModifier, m_confined);
        m_confined = None;
    }
}

/**
 * Captures all the mouse clicks going to a window, rather than sending it off
 * to the application itself. 
 * @param window The window to intercept clicks from.
 */
void XData::grab_mouse(Window window)
{
    XGrabButton(m_display, AnyButton, AnyModifier, window, true,
            ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync, GrabModeAsync, None, None);
}

/**
 * Stops grabbing the clicks going to a window and lets the application handle
 * the clicks itself.
 * @param window The window to stop intercepting clicks from.
 */
void XData::ungrab_mouse(Window window)
{
    XUngrabButton(m_display, AnyButton, AnyModifier, window);
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
            windows.push_back(children[idx]);
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
    XQueryPointer(m_display, m_root, &_u1, &_u1, 
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

    XGetInputFocus(m_display, &new_focus, &_unused);
    return new_focus;
}

/**
 * Sets the input focus,  
 * @param window The window to set the focus of.
 * @return true if the change succeeded or false otherwise.
 */
bool XData::set_input_focus(Window window)
{
    // If we're unfocusing, then move the focus to the root so that keyboard
    // shortcuts work
    if (window == None)
        window = m_root;

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
void XData::set_attributes(Window window, XSetWindowAttributes &attr,
        unsigned long mask)
{
    XChangeWindowAttributes(m_display, window, mask, &attr);
}

/**
 * Checks to see if a window is visible or not.
 */
bool XData::is_mapped(Window window)
{
    XWindowAttributes attrs;
    get_attributes(window, attrs);
    return attrs.map_state != IsUnmapped;
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
    XSetWindowBorderWidth(m_display, window, size);
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
    // We have to do some juggling to get a non-const pointer from a const 
    // iteartor
    Window *win_ptr = const_cast<Window*>(&(*windows.begin()));
    XRestackWindows(m_display, win_ptr, windows.size());
}

/**
 * Gets the XWMHints structure corresponding to the given window.
 * @param window The window to get the hints for.
 * @param[out] hints The storage for the hints.
 * @return True if the window has hints, False otherwise.
 */
bool XData::get_wm_hints(Window window, XWMHints &hints)
{
    XWMHints *returned_hints = XGetWMHints(m_display, window);

    // Since we have to get rid of this later, and it is an unnecessary
    // complication to return it, we'll just copy it and get rid of the
    // pointer that was returned to us
    if (returned_hints)
    {
        std::memcpy(&hints, returned_hints, sizeof(XWMHints));

        XFree(returned_hints);
        return true;
    }
    else
        return false;
}

/***
 * Gets the XSizeHints structure corresponding to the given window.
 * @param window The window to get the hints for.
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
void XData::get_icon_name(Window window, std::string &name)
{
    char *icon_name;
    XGetIconName(m_display, window, &icon_name);
    if (icon_name)
    {
        name.assign(icon_name);
        XFree(icon_name);
        return;
    }
   
    XFetchName(m_display, window, &icon_name);
    
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
    XClassHint *hint = XAllocClassHint();
    XGetClassHint(m_display, win, hint);
    
    if (hint->res_name)
        XFree(hint->res_name);

    
    if (hint->res_class)
    {
        xclass.assign(hint->res_class);
        XFree(hint->res_class);
    }
    else
        xclass.clear();

    XFree(hint);
}

/**
 * Gets the size of the primary screen.
 * @param[out] width The width of the screen.
 * @param[out] height The height of the screen.
 */
void XData::get_screen_bounds(Box &box)
{
    get_screen_bounds_at_location(0, 0, box);
}

/**
 * Gets the size of the screen which a particular window occupies.
 */
void XData::get_screen_bounds_for_window(Window window, Box &box) 
{
    XWindowAttributes attr;
    get_attributes(window, attr);

    get_screen_bounds_at_location(attr.x, attr.y, box);
}

/**
 * Figures out what screen is covering a point, and then gets the bounds of that
 * screen.
 *
 * This is the result of my crawling through Xrandr.h rather than any attempt 
 * at processing formal documentation. There aren't any good docs, from what 
 * I can find.
 *
 * The AwesomeWM codebase was helpful in finding out a few things, though.
 */
void XData::get_screen_bounds_at_location(int x, int y, Box &box)
{
    XRRScreenResources *resources = XRRGetScreenResourcesCurrent(m_display, m_root);

    if (resources == NULL)
    {
        box.x = -1;
        box.y = -1;
        box.width = -1;
        box.height = -1;
        return;
    }

    // XRandR stores things called 'CRTCs', which is apparently a funny way of
    // spelling 'outputs' (like LVDS1 or VGA2). We have to find out what location
    // the top-left of the window is in, and then test all the CRTCs to figure
    // out which contains our position.
    //
    // It *seems* like there should be a better way, but this is exactly what
    // awesome does.
    //
    // I may decide to do caching on this later, but I'll have to see how slow
    // it is.
    for (int crtc_idx = 0; crtc_idx < resources->ncrtc; crtc_idx++)
    {
        RRCrtc crtc_id = resources->crtcs[crtc_idx];

        XRRCrtcInfo *crtc = XRRGetCrtcInfo(m_display, resources, crtc_id);
        if (!crtc)
            continue;

        int left = crtc->x;
        int right = left + crtc->width;
        int top = crtc->y;
        int bottom = top + crtc->height;

        XRRFreeCrtcInfo(crtc);

        if (IN_BOUNDS(x, left, right) && IN_BOUNDS(y, top, bottom))
        {
            box.x = left;
            box.y = top;
            box.width = right - left;
            box.height = bottom - top;

            goto cleanup_screen;
        }
    }

    // Somehow, the position isn't on any screen
    box.x = -1;
    box.y = -1;
    box.width = -1;
    box.height = -1;

cleanup_screen:
    XRRFreeScreenResources(resources);
    return;
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
 * Converts a KeySym into a string.
 * @param keysym The KeySym to convert.
 * @param[out] as_string The string version of the KeySym.
 */
void XData::keysym_to_string(KeySym keysym, std::string &as_string)
{
    // Interestingly, the pointer here references some kind of table in static
    // memory, so we can't free it
    char *keysym_str = XKeysymToString(keysym);

    if (!keysym_str)
        as_string.clear();
    else
        as_string.assign(keysym_str);
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
unsigned long XData::decode_monocolor(MonoColor color)
{
    switch (color)
    {
        case X_BLACK:
            return BlackPixel(m_display, m_screen);
        case X_WHITE:
            return WhitePixel(m_display, m_screen);
    }
}
