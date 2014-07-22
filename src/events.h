/** @file */
#ifndef __SMALLWM_EVENTS__
#define __SMALLWM_EVENTS__

#include "model/client-model.h"
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
            int randr_offset)
        m_config(config), m_xdata(xdata), _clients(clients), 
        m_randroffset(randr_offset), m_done(false),
    {};

    bool step();

    void handle_keypress();
    void handle_buttonpress();
    void handle_buttonrelease();
    void handle_motionnotify();
    void handle_mapnotify();
    void handle_expose();
    void handle_configurenotify();
    void handle_destroynotify();

    void handle_rrnotify();

private:
    /// The currently active event
    XEvent m_event;

    /// Whether or not the user has terminated SmallWM
    bool m_done;

    /// The configuration options that were given in the configuration file
    WMConfig &m_config;

    /// The data required to interface with Xlib
    XData &m_xdata;

    /// The data model which stores the clients and data about them
    ClientModel m_clients;

    /// The offset for all RandR generated events
    int m_randroffset;
};

#endif
