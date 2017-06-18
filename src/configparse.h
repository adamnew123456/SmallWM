/** @file */
#ifndef __SMALLWM_CONFIG__
#define __SMALLWM_CONFIG__

#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <utility>
#include <vector>

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
    SCREEN_TOP, SCREEN_BOTTOM, SCREEN_LEFT, SCREEN_RIGHT,
    LAYER_ABOVE, LAYER_BELOW, LAYER_TOP, LAYER_BOTTOM,
    LAYER_1, LAYER_2, LAYER_3, LAYER_4, LAYER_5, LAYER_6, LAYER_7, LAYER_8, LAYER_9,
    CYCLE_FOCUS, CYCLE_FOCUS_BACK,
    EXIT_WM
};

/**
 * Describes a relationship between:
 *  - A keyboard shortcut
 *  - The name in the configuration file
 *  - The default keyboard shortcut
 *  - Whether or not that shortcut uses the secondary action modifier
 */
struct DefaultShortcut
{
    KeyboardAction action;
    const char *config_name;
    KeySym keysym;
    bool uses_secondary;
};

/**
 * Key bindings consist of both a main key, a modifier, and (possibly)
 * a secondary modifier (the boolean).
 */
typedef std::pair<KeySym, bool> KeyBinding;

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
        action_to_binding.clear();
        binding_to_action.clear();

        DefaultShortcut shortcuts[] = {
            { CLIENT_NEXT_DESKTOP, "client-next-desktop", XK_bracketright, false },
            { CLIENT_PREV_DESKTOP, "client-prev-desktop", XK_bracketleft, false },
            { NEXT_DESKTOP, "next-desktop", XK_period, false },
            { PREV_DESKTOP, "prev-desktop", XK_comma, false },
            { TOGGLE_STICK, "toggle-stick", XK_backslash, false },
            { ICONIFY, "iconify", XK_h, false },
            { MAXIMIZE, "maximize", XK_m, false },
            { REQUEST_CLOSE, "request-close", XK_c, false },
            { FORCE_CLOSE, "force-close", XK_x, false },
            { K_SNAP_TOP, "snap-top", XK_Up, false },
            { K_SNAP_BOTTOM, "snap-bottom", XK_Down, false },
            { K_SNAP_LEFT, "snap-left", XK_Left, false },
            { K_SNAP_RIGHT, "snap-right", XK_Right, false },
            { SCREEN_TOP, "screen-top", XK_Up, true },
            { SCREEN_BOTTOM, "screen-bottom", XK_Down, true },
            { SCREEN_LEFT, "screen-left", XK_Left, true },
            { SCREEN_RIGHT, "screen-right", XK_Right, true },
            { LAYER_ABOVE, "layer-above", XK_Page_Up, false },
            { LAYER_BELOW, "layer-below", XK_Page_Down, false },
            { LAYER_TOP, "layer-top", XK_Home, false },
            { LAYER_BOTTOM, "layer-bottom", XK_End, false },
            { LAYER_1, "layer-1", XK_1, false },
            { LAYER_2, "layer-2", XK_2, false },
            { LAYER_3, "layer-3", XK_3, false },
            { LAYER_4, "layer-4", XK_4, false },
            { LAYER_5, "layer-5", XK_5, false },
            { LAYER_6, "layer-6", XK_6, false },
            { LAYER_7, "layer-7", XK_7, false },
            { LAYER_8, "layer-8", XK_8, false },
            { LAYER_9, "layer-9", XK_9, false },
            { CYCLE_FOCUS, "cycle-focus", XK_Tab, false },
            { CYCLE_FOCUS_BACK, "cycle-focus-back", XK_Tab, true },
            { EXIT_WM, "exit", XK_Escape, false },
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

            KeyBinding binding(current_shortcut.keysym, current_shortcut.uses_secondary);

            action_to_binding[current_shortcut.action] = binding;
            binding_to_action[binding] = current_shortcut.action;
        }
    }

    /// The keyboard shortcuts which are bound to specific keys
    std::map<KeyboardAction, KeyBinding> action_to_binding;

    /// The configuration values which are bound to specific shortcuts
    std::map<std::string, KeyboardAction> action_names;

    /// A reverse mapping between KeyBindings and KeyboardActions
    std::map<KeyBinding, KeyboardAction> binding_to_action;
};

/**
 * How to apply actions made with keyboard hotkeys.
 */
enum HotkeyType
{
    HK_FOCUS, //< Hotkeys apply to the currently focused window
    HK_MOUSE //< Hotkeys apply to the window the mouse cursor is on
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

    /// The current hotkey mode
    HotkeyType hotkey;

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

    /// All of the X11 classes which should not be focusable by default
    std::vector<std::string> no_autofocus;

    /// Whether or not to show images inside icons for hidden windows
    bool show_icons;

    /// The filename to dump the current state to when SIGUSR1 is received
    std::string dump_file;

protected:
    virtual std::string get_config_path() const;

private:
    static int config_parser(void *user, const char *c_section, const char *c_name, const char *c_value);
};

#endif
