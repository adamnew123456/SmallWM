/** @file */
#ifndef __SMALLWM_XDATA__
#define __SMALLWM_XDATA__

#include <cstring>
#include <vector>

#include "common.h"
#include "logging.h"

/**
 * An X graphics context which is used to draw on windows.
 */
class XGC
{
public:
    XGC(Display *dpy, Window window) :
        m_display(dpy), m_window(window)
    {
        m_gc = XCreateGC(dpy, window, 0, NULL);
    };

    ~XGC()
    {
        XFree(m_gc);
    };

    void clear();
    void draw_string(Dimension, Dimension, const std::string&);
    void copy_pixmap(Drawable, Dimension, Dimension);

private:
    /** The raw X display - this is necessary to have since XData doesn't
     * expose it. */
    Display *m_display;


    /// The window this graphics context belongs to
    Window m_window;
    
    /// The X graphics context this sits above
    GC m_gc;
};

/**
 * Identifies the colors which can be used for window borders and the like.
 */
enum MonoColor
{
    X_BLACK,
    X_WHITE,
};

/**
 * This forms a layer above raw Xlib, which stores the X display, root 
 * window, etc. and provides the most common operations which use these data.
 */
class XData
{
public:
    XData(SysLog &logger, Display *dpy, Window root, int screen) :
        m_display(dpy)
    {
        m_root = DefaultRootWindow(dpy);
        m_screen = DefaultScreen(dpy);

        // Use RandR to figure out what the initial screen size is
        XRRScreenConfiguration *config = XRRGetScreenInfo(dpy, root);

        int _u1;
        XRRScreenSize *size = XRRConfigSizes(config, &_u1);
        m_screen_size = Dimension2D(size->width, size->height);
        XFree(size);

        XRRFreeScreenConfigInfo(config);
    };

    XGC *create_gc(Window);
    Window create_window(bool);

    void change_property(Window, const std::string&, Atom, 
            const unsigned char*, size_t);

    void next_event(XEvent&);
    void get_latest_event(XEvent&, int);

    void add_hotkey(KeySym);
    void add_hotkey_mouse(unsigned int);

    void confine_pointer(Window);
    void stop_confining_pointer();

    void select_input(Window, long);

    void get_windows(std::vector<Window>&);
    void get_pointer_location(Dimension&, Dimension&);

    Window get_input_focus();
    bool set_input_focus(Window);

    void map_win(Window);
    void unmap_win(Window);
    void request_close(Window);
    void destroy_win(Window);

    void get_attributes(Window, XWindowAttributes&);
    void set_attributes(Window, const XSetWindowAttributes&,
        unsigned long);
    void set_border_color(Window, MonoColor);
    void set_border_width(Window, Dimension);

    void move_window(Window, int, int);
    void resize_window(Window, Dimension, Dimension);
    void raise(Window);
    void restack(const std::vector<Window>&);

    void get_wm_hints(Window, XWMHints);
    void get_size_hints(Window, XSizeHints&);
    Window get_transient_hint(Window);
    void get_icon_name(Window, std::string&);

    KeySym get_keysym(int);

    void get_screen_size(Dimension&, Dimension&);
    void set_screen_size(Dimension, Dimension);

private:
    Atom intern_if_needed(const std::string&);
    unsigned long decode_monocolor(MonoColor);

    /// The connection to the X server
    Display *m_display;

    /// The root window on the display
    Window m_root;

    /// The default X11 screen
    int m_screen;

    /// The pre-defined atoms, which are accessible via a string
    std::map<std::string, Atom> m_atoms;

    /// The size of the screen
    Dimension2D m_screen_size;
};

#endif
