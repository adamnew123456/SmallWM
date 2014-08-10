/** @file */
#ifndef __SMALLWM_CLIENTMODEL_EVENTS__
#define __SMALLWM_CLIENTMODEL_EVENTS__

#include <algorithm>
#include <vector>

#include "model/client-model.h"
#include "model/x-model.h"
#include "configparse.h"
#include "common.h"
#include "xdata.h"

/**
 * A dispatcher for handling the different change events raised by the
 * ClientModel.
 *
 * This serves as the linkage between changes in the client model, and changes
 * to the UI on the screen.
 */
class ClientModelEvents
{
public:
    ClientModelEvents(WMConfig &config, XData &xdata, ClientModel &clients, 
        XModel &xmodel) :
    m_config(config), m_xdata(xdata), m_clients(clients), m_xmodel(xmodel),
    m_change(0), m_should_relayer(false)
    {};

    void handle_queued_changes();

private:
    void do_relayer();

    void handle_layer_change();
    void handle_focus_change();
    void handle_client_desktop_change();
    void handle_current_desktop_change();
    void handle_location_change();
    void handle_size_change();

    /// The change that is currently being processed
    Change const *m_change;

    /// The configuration options that were given in the configuration file
    WMConfig &m_config;

    /// The data required to interface with Xlib
    XData &m_xdata;

    /// The data model which stores the clients and data about them
    ClientModel &m_clients;

    /** The data model which stores information related to clients, but not
     * about them. */
    XModel &m_xmodel;

    /** Whether or not to relayer the visible windows - this allows this class
     * to avoid restacking windows on every `ChangeLayer`, and instead only do 
     * it once at the end of `handle_queued_changes`. */
    bool m_should_relayer;
};
#endif
