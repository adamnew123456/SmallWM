/** @file */
#ifndef __SMALLWM_DESKTOPS__
#define __SMALLWM_DESKTOPS__

#include "clients.h"
#include "common.h"
#include "shared.h"
#include "utils.h"

/**
 * Manages how clients are shown and hidden when the current desktop changes.
 */
class DesktopManager
{
public:
    DesktopManager(ClientContainer *clients, WMShared &shared) :
        m_clients(clients), m_shared(shared), m_current_desktop(1)
    {};

    void flip_sticky_flag(Window);

    void set_desktop(Window, Desktop);
    void to_next_desktop(Window);
    void to_prev_desktop(Window);

    void next_desktop();
    void prev_desktop();
protected:
    bool should_be_visible(Window);
    void add_desktop(Window);
    void reset_desktop(Window);
    void update_desktop();
    void delete_desktop(Window);

private:
    /// Iterator used for accessing the desktop mapping
    typedef std::map<Window,Desktop>::iterator iterator;

    /// A relation between each client and its current desktop
    std::map<Window, Desktop> m_desktops;

    /// Whether or not a client is sticky
    std::map<Window, bool> m_sticky;

    /// The currently visible desktop
    Desktop m_current_desktop;

    /// The clients which are being managed
    ClientContainer *m_clients;

    /// The window manager's shared data
    WMShared &m_shared;
};
#endif
