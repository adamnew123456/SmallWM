/** @file */
#ifndef __SMALLWM_CLIENTMANAGER__
#define __SMALLWM_CLIENTMANAGER__

#include <algorithm>
#include <map>
#include <vector>

#include "actions.h"
#include "clients.h"
#include "common.h"
#include "desktops.h"
#include "icons.h"
#include "layers.h"
#include "shared.h"
#include "utils.h"

/**
 * Data which is used to manage a window which is currently being moved or
 * resized.
 */
struct MoveResize
{
    /// Initialize everything to zero
    MoveResize() :
        window(None), client(None), ptr_loc(0, 0)
    {};

    /// The placeholder window which the user manipulates
    Window window;

    /// The client window that is being moved/resized
    Window client;

    /// The most recent pointer location
    Dimension2D ptr_loc;
};

/**
 * A container and 'state manager' for all of the clients. It manages how clients
* transition between ClientStates, by rejecting invalid states and performing
 * the appropriate operations on valid state transitions.
 *
 * It is currently a bit large - perhaps I'll refactor it soon, but until then
 * keeping the members spread across different source file keeps the code
 * readable.
 */
class ClientManager : 
    protected ClientContainer, protected LayerManager,
    protected DesktopManager, protected IconManager
{
public:
    /// Initialize the share data and the current desktop
    ClientManager(WMShared &shared) :
        ClientContainer::ClientContainer(shared),
        LayerManager::LayerManager((ClientContainer*)this, shared),
        DesktopManager::DesktopManager((ClientContainer*)this, shared),
        IconManager::IconManager(shared),
        m_shared(shared)
    {};

    using ClientContainer::is_client;
    using ClientContainer::is_visible;
    using ClientContainer::clients_begin;
    using ClientContainer::clients_end;
    using ClientContainer::get_state;

    using LayerManager::raise_layer;
    using LayerManager::lower_layer;
    using LayerManager::set_layer;

    using DesktopManager::flip_sticky_flag;
    using DesktopManager::set_desktop;
    using DesktopManager::to_next_desktop;
    using DesktopManager::to_prev_desktop;
    using DesktopManager::next_desktop;
    using DesktopManager::prev_desktop;

    using IconManager::get_icon_of_client;
    using IconManager::get_icon_of_icon;
    using IconManager::redraw_icon;

    Window get_from_placeholder(Window);

    void register_action(std::string, ClassActions);

    void handle_motion(const XEvent&);
    virtual void state_transition(Window, ClientState);

    void create(Window);
    void destroy(Window);
    void close(Window);

    void snap(Window, SnapDir);
    void maximize(Window);

    virtual void relayer();
    virtual void redesktop();

private:
    void apply_actions(Window);
        
    void focus(Window);
    void unfocus(Window);
    void unfocus_unsafe(Window);

    void create_placeholder(const XWindowAttributes&);
    void begin_moving(Window, const XWindowAttributes&);
    void begin_resizing(Window, const XWindowAttributes&);
    void end_move_resize();
    void end_move_resize_unsafe();

    /// Shared window manager data
    WMShared &m_shared;

    /// All the registered ClassActions
    std::map<std::string, ClassActions> m_actions;

    /// A relation between each client window and its current state
    std::map<Window,ClientState> m_clients;

    /// Data used to manage moving/resizing clients
    MoveResize m_mvr;
};
#endif
