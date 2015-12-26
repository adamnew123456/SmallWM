/** @file */
#ifndef __SMALLWM_CLIENTMODEL_EVENTS__
#define __SMALLWM_CLIENTMODEL_EVENTS__

#include <algorithm>
#include <vector>

#include "model/client-model.h"
#include "model/focus-cycle.h"
#include "model/x-model.h"
#include "configparse.h"
#include "common.h"
#include "logging/logging.h"
#include "utils.h"
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
    ClientModelEvents(WMConfig &config, Log &logger, ChangeStream &changes,
        XData &xdata, ClientModel &clients, XModel &xmodel,
        FocusCycle &focus_cycle) :
        m_config(config), m_xdata(xdata), m_clients(clients), m_xmodel(xmodel),
        m_changes(changes), m_logger(logger), m_focus_cycle(focus_cycle), 
        m_change(0), m_should_relayer(false), m_should_reposition_icons(false)
    {};

    void handle_queued_changes();

private:
    void register_new_icon(Window, bool);
    Window create_placeholder(Window);
    void start_moving(Window);
    void start_resizing(Window);
    void do_relayer();
    void reposition_icons();
    void update_focus_cycle();
    void update_location_size_for_cps(Window, ClientPosScale);

    void handle_layer_change();
    void handle_focus_change();
    void handle_client_desktop_change();
    void handle_current_desktop_change();
    void handle_screen_change();
    void handle_mode_change();
    void handle_location_change();
    void handle_size_change();
    void handle_destroy_change();
    void handle_unmap_change();

    void handle_new_client_desktop_change(const Desktop*, Window);
    void handle_client_change_from_user_desktop(const Desktop*, const Desktop*, 
                                                Window);
    void handle_client_change_from_all_desktop(const Desktop*, const Desktop*, 
                                                Window);
    void handle_client_change_from_icon_desktop(const Desktop*, const Desktop*, 
                                                Window);
    void handle_client_change_from_moving_desktop(const Desktop*, const Desktop*, 
                                                Window);
    void handle_client_change_from_resizing_desktop(const Desktop*, const Desktop*, 
                                                Window);

    /// The stream of changes to read from
    ChangeStream &m_changes;

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

    /// The event handler's logger
    Log &m_logger;

    /// The focus cycler
    FocusCycle &m_focus_cycle;

    /** Whether or not to relayer the visible windows - this allows this class
     * to avoid restacking windows on every `ChangeLayer`, and instead only do 
     * it once at the end of `handle_queued_changes`. */
    bool m_should_relayer;
    
    /** Similar to `m_should_relayer`, this indicates whether the change handler
     * should reposition all the icon windows at the end of `handle_queued_changes`.
     */
    bool m_should_reposition_icons;
};
#endif
