/** @file */
#ifndef __SMALLWM_EVENTS__
#define __SMALLWM_EVENTS__

#include "clientmanager.h"
#include "configparse.h"
#include "common.h"
#include "shared.h"

/**
 * A dispatcher for handling the different type of X events.
 */
class XEvents
{
public:
    XEvents(WMShared&, ClientManager&, KeyboardConfig&, int);
    void run();

    void handle_keypress();
    void handle_buttonpress();
    void handle_buttonrelease();
    void handle_motionnotify();
    void handle_mapnotify();
    void handle_expose();
    void handle_destroynotify();

    void handle_rrnotify();

private:
    /// The currently active event
    XEvent m_event;

    /// Whether or not the user has terminated SmallWM
    bool m_done;

    /// Shared window-manager data
    WMShared &m_shared;

    /// Interface to the window manager's clients
    ClientManager &m_clients;

    /// The master list of all keyboard shortcuts
    KeyboardConfig &m_keyboard;

    /// The offset for all RandR generated events
    int m_randroffset;
};

#endif
