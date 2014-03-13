/** @file */
#ifndef __SMALLWM_CONFIG__
#define __SMALLWM_CONFIG__

#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "ini.h"
#include "actions.h"
#include "common.h"
#include "utils.h"

/**
 * All the keyboard controls, which can loaded from the config file.
 */
struct KeyboardConfig
{
    // Initialize the default keyboard bindings.
    KeyboardConfig() :
        client_next_desktop(XK_bracketright),
        client_prev_desktop(XK_bracketleft),
        next_desktop(XK_period),
        prev_desktop(XK_comma),
        toggle_stick(XK_backslash),
        iconify(XK_h),
        maximize(XK_m),
        request_close(XK_c),
        force_close(XK_x),
        snap_top(XK_Up),
        snap_bottom(XK_Down),
        snap_left(XK_Left),
        snap_right(XK_Right),
        layer_above(XK_Page_Up),
        layer_below(XK_Page_Down),
        layer_top(XK_Home),
        layer_bottom(XK_End),
        exit_wm(XK_Escape),
        layer1(XK_1), layer2(XK_2), layer3(XK_3), layer4(XK_4), layer5(XK_5),
        layer6(XK_6), layer7(XK_7), layer8(XK_8), layer9(XK_9)
    {};

    /// Moves the client to the desktop after its curreont one.
    KeySym client_next_desktop,
        /// Moves the client to the desktop before its current one.
        client_prev_desktop,
        /// Shows the next desktop.
        next_desktop,
        /// Shows the previous desktop.
        prev_desktop,
        /// (Un)sticks a client.
        toggle_stick,
        /// Iconifies a client.
        iconify,
        /// Maximizes a client.
        maximize,
        /// Request a client to close.
        request_close,
        /// Force a client to close.
        force_close,
        /// Snaps a window to the top.
        snap_top,
        /// Snaps a window to the bottom.
        snap_bottom,
        /// Snaps a window to the left.
        snap_left,
        /// Snaps a window to the right.
        snap_right,
        /// Puts a client onto the layer above.
        layer_above,
        /// Puts a client onto the layer below.
        layer_below,
        /// Puts a client onto the very top layer.
        layer_top,
        /// Puts a client onto the very bottom layer.
        layer_bottom,
        /// Exits the window manager.
        exit_wm,
        
        // Set the layers directly
        layer1,
        layer2,
        layer3,
        layer4,
        layer5,
        layer6,
        layer7,
        layer8,
        layer9;
};

/**
 * Reads and manages configuration options in the SmallWM configure option.
 */
class WMConfig
{
public:
    /// Loads a couple of defaults for the WMConfig.
    WMConfig() : shell("/usr/bin/xterm"), num_desktops(5), 
        icon_width(75), icon_height(20), border_width(2) {};
    
    void load();
    
    /// The shell to run.
    std::string shell;

    /// The window manager's keyboard bindings.
    KeyboardConfig key_commands;

    /// The number of available desktops.
    Desktop num_desktops;

    /// The width of hidden icons.
    Dimension icon_width, 
    /// The height of the hidden icons.
              icon_height;

    /// The width of the window border.
    Dimension border_width;

    /// Handles all the configured class actions.
    std::map<std::string, ClassActions> classactions;

protected:
    virtual std::string get_config_path() const;

private:
    static int config_parser(void *user, const char *c_section, const char *c_name, const char *c_value);
};



#endif
