/** @file */
#ifndef __SMALLWM_X_EVENTS__
#define __SMALLWM_X_EVENTS__

#include "model/client-model.h"
#include "model/focus-cycle.h"
#include "model/x-model.h"
#include "configparse.h"
#include "common.h"
#include "xdata.h"

/**
 * A dispatcher for handling the different type of X events.
 *
 * This serves as the linkage between raw Xlib events, and changes in the
 * client model.
 */
class XEvents
{
public:
    XEvents(WMConfig &config, XData &xdata, ClientModel &clients,
            XModel &xmodel, FocusCycle &focus_cycle) :
        m_config(config), m_xdata(xdata), m_clients(clients),
        m_xmodel(xmodel), m_focus_cycle(focus_cycle), m_done(false)
    {
        xdata.add_hotkey_mouse(MOVE_BUTTON);
        xdata.add_hotkey_mouse(RESIZE_BUTTON);
        xdata.add_hotkey_mouse(LAUNCH_BUTTON);

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
            CYCLE_FOCUS, EXIT_WM,
            INVALID_ACTION
        };

        for (KeyboardAction *action = &actions[0]; *action != INVALID_ACTION; action++)
        {
            KeyBinding &binding = config.key_commands.action_to_binding[*action];
            xdata.add_hotkey(binding.first, binding.second);
        }
    };

    bool step();

    // Note that this is exposed because smallwm.cpp has to import existing
    // windows when main() runs
    void add_window(Window);

private:
    void handle_rrnotify();
    void handle_keypress();
    void handle_buttonpress();
    void handle_buttonrelease();
    void handle_motionnotify();
    void handle_mapnotify();
    void handle_expose();
    void handle_destroynotify();

    /// The currently active event
    XEvent m_event;

    /// Whether or not the user has terminated SmallWM
    bool m_done;

    /// The configuration options that were given in the configuration file
    WMConfig &m_config;

    /// The data required to interface with Xlib
    XData &m_xdata;

    /// The data model which stores the clients and data about them
    ClientModel &m_clients;

    /** The data model which stores information related to clients, but not
     * about them. */
    XModel &m_xmodel;

    /// The focus cycler
    FocusCycle &m_focus_cycle;

    /// The offset for all RandR generated events
    int m_randroffset;
};

#endif
