/** @file */
#ifndef __SMALLWM_CLIENTMANAGER__
#define __SMALLWM_CLIENTMANAGER__

#include <algorithm>
#include <list>
#include <map>
#include <vector>

#include "actions.h"
#include "clients.h"
#include "common.h"
#include "desktops.h"
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
 * Data which is used to draw an application icon.
 */
struct Icon
{
    /// Initialize everything to zero
    Icon() :
        window(None), client(None), gc(0),
        has_pixmap(false), pixmap(0),
        pixmap_size(0, 0)
    {};

    /// The window that the icon is drawn upon
    Window window;

    /// The client window which this icon stands in for
    Window client;

    /// The graphics context used for the title text and the pixmap 
    /// (i.e. the graphical icon itself)
    GC gc;

    /// Whether or not this icon has an associated pixmap
    bool has_pixmap;

    /// The pixmap graphic itself
    Pixmap pixmap;

    /// The dimensions of the pixmap
    Dimension2D pixmap_size;
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
    protected DesktopManager
{
public:
    /// Initialize the share data and the current desktop
    ClientManager(WMShared &shared) :
        ClientContainer::ClientContainer(shared),
        LayerManager::LayerManager((ClientContainer*)this, shared),
        DesktopManager::DesktopManager((ClientContainer*)this, shared),
        m_shared(shared), m_current_focus(None), m_revert_focus(true)
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

    Icon *get_icon_of_client(Window);
    Icon *get_icon_of_icon(Window);
    Window get_from_placeholder(Window);

    void register_action(std::string, ClassActions);

    void handle_motion(const XEvent&);
    virtual void state_transition(Window, ClientState);

    void create(Window);
    void destroy(Window);
    void close(Window);

    void snap(Window, SnapDir);
    void maximize(Window);

    void redraw_icon(Window);

    virtual void relayer();
    virtual void redesktop();

private:
    void apply_actions(Window);

    void unmap(Window);
        
    void focus(Window);
    void unfocus();
    void remove_from_focus_history(Window);

    void make_icon(Window);
    void reflow_icons();
    void delete_icon(Icon*);

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

    /// A relation between each iconified client and its icon data
    std::map<Window,Icon*> m_icons;

    /// Data used to manage moving/resizing clients
    MoveResize m_mvr;

    /// A stack of previously focused windows
    std::list<Window> m_focus_history;

    /// The currently focused window
    Window m_current_focus;

    /// Whether or not the unfocus routine should revert the focus
    bool m_revert_focus;
};
#endif
