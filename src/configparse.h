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
 * The different default keyboard shortcuts.
 */
enum KeyboardAction
{
    INVALID_ACTION = 0,
    CLIENT_NEXT_DESKTOP, CLIENT_PREV_DESKTOP,
    NEXT_DESKTOP, PREV_DESKTOP,
    TOGGLE_STICK,
    ICONIFY,
    MAXIMIZE,
    REQUEST_CLOSE, FORCE_CLOSE,
    K_SNAP_TOP, K_SNAP_BOTTOM, K_SNAP_LEFT, K_SNAP_RIGHT,
    LAYER_ABOVE, LAYER_BELOW, LAYER_TOP, LAYER_BOTTOM,
    LAYER_1, LAYER_2, LAYER_3, LAYER_4, LAYER_5, LAYER_6, LAYER_7, LAYER_8, LAYER_9,
    CYCLE_FOCUS,
    EXIT_WM
};

/**
 * Describes a relationship between:
 *  - A keyboard shortcut
 *  - The name in the configuration file
 *  - The default keyboard shortcut
 */
struct DefaultShortcut
{
    KeyboardAction action;
    const char *config_name;
    KeySym keysym;
};

/**
 * All the keyboard controls, which can loaded from the config file.
 */
struct KeyboardConfig
{
    // Initialize the default keyboard bindings.
    KeyboardConfig()
    {
        reset();
    };

    void reset()
    {
        action_to_keysym.clear();
        keysym_to_action.clear();

        DefaultShortcut shortcuts[] = {
            { CLIENT_NEXT_DESKTOP, "client-next-desktop", XK_bracketright },
            { CLIENT_PREV_DESKTOP, "client-prev-desktop", XK_bracketleft },
            { NEXT_DESKTOP, "next-desktop", XK_period },
            { PREV_DESKTOP, "prev-desktop", XK_comma },
            { TOGGLE_STICK, "toggle-stick", XK_backslash },
            { ICONIFY, "iconify", XK_h },
            { MAXIMIZE, "maximize", XK_m },
            { REQUEST_CLOSE, "request-close", XK_c },
            { FORCE_CLOSE, "force-close", XK_x },
            { K_SNAP_TOP, "snap-top", XK_Up },
            { K_SNAP_BOTTOM, "snap-bottom", XK_Down },
            { K_SNAP_LEFT, "snap-left", XK_Left },
            { K_SNAP_RIGHT, "snap-right", XK_Right },
            { LAYER_ABOVE, "layer-above", XK_Page_Up },
            { LAYER_BELOW, "layer-below", XK_Page_Down },
            { LAYER_TOP, "layer-top", XK_Home },
            { LAYER_BOTTOM, "layer-bottom", XK_End },
            { LAYER_1, "layer-1", XK_1 },
            { LAYER_2, "layer-2", XK_2 },
            { LAYER_3, "layer-3", XK_3 },
            { LAYER_4, "layer-4", XK_4 },
            { LAYER_5, "layer-5", XK_5 },
            { LAYER_6, "layer-6", XK_6 },
            { LAYER_7, "layer-7", XK_7 },
            { LAYER_8, "layer-8", XK_8 },
            { LAYER_9, "layer-9", XK_9 },
            { CYCLE_FOCUS, "cycle-focus", XK_Tab },
            { EXIT_WM, "exit", XK_Escape },
        };

        int num_shortcuts = sizeof(shortcuts) / sizeof(shortcuts[0]);
        for (int i = 0; i < num_shortcuts; i++)
        {
            DefaultShortcut &current_shortcut = shortcuts[i];
            if (current_shortcut.config_name != NULL)
            {
                std::string config_name = std::string(current_shortcut.config_name);
                action_names[config_name] = current_shortcut.action;
            }

            action_to_keysym[current_shortcut.action] = 
                current_shortcut.keysym;
            keysym_to_action[current_shortcut.keysym] =
                current_shortcut.action;
        }
    }

    /// The keyboard shortcuts which are bound to specific keys
    std::map<KeyboardAction, KeySym> action_to_keysym;

    /// The configuration values which are bound to specific shortcuts
    std::map<std::string, KeyboardAction> action_names;

    /// A reverse mapping between KeySyms and KeyboardActions
    std::map<KeySym, KeyboardAction> keysym_to_action;
};

/**
 * Reads and manages configuration options in the SmallWM configure option.
 */
class WMConfig
{
public:
    /// Loads a couple of defaults for the WMConfig.
    WMConfig()
    { reset(); };
    
    void load();
    void reset();

    /// The minimum message level to send to syslog
    int log_mask;

    /// The shell to run.
    std::string shell;

    /// The window manager's keyboard bindings.
    KeyboardConfig key_commands;

    /// The number of available desktops.
    unsigned long long num_desktops;

    /// The width of hidden icons.
    Dimension icon_width, 
    /// The height of the hidden icons.
              icon_height;

    /// The width of the window border.
    Dimension border_width;

    /// Handles all the configured class actions.
    std::map<std::string, ClassActions> classactions;

    /// Whether or not to show images inside icons for hidden windows
    bool show_icons;

protected:
    virtual std::string get_config_path() const;

private:
    static int config_parser(void *user, const char *c_section, const char *c_name, const char *c_value);
};

#endif
