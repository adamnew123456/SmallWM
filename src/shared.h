/** @file */
#ifndef __SMALLWM_SHARED__
#define __SMALLWM_SHARED__

#include <map>

#include "logging.h"

/**
 * This is data which needs to be shared between the window manager as a whole
 * and each individual client/icon in under the WM's control.
 */
struct WMShared {
    /// The X display - shared because each client/icon needs it do much of anything since X11 needs to know the display for each operation.
    Display *display;

    /// The root window - shared because each client needs it so it can create its move/resize placeholder window, and for each icon to create its icon window.
    Window root;

    /// The root screen - required, again, to create the move/resize placeholder and the icon window.
    int screen;

    /// The screen dimensions - required for each client to maximize itself and each icon to position itself.
    Dimension2D screen_size;

    ///The maximum number of desktops - required for each client to move itself from one desktop to another.
    Desktop max_desktops;

    /// The dimensions of icons - required for each icon to create its icon window.
    Dimension2D icon_size;

    /// The width of the window border - required for clients to decorate their windows.
    Dimension border_width;

    /// The shell to launch
    std::string shell;

    /// Whether or not to show application icons inside icon windows
    bool show_icons;

    /// All the atoms which are needed by SmallWM
    std::map<std::string, Atom> atoms;

    /// The logger
    SysLog logger;
};

#endif
