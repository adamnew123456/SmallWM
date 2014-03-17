/** @file */
#ifndef __SMALLWM_CLIENTMANAGER__
#define __SMALLWM_CLIENTMANAGER__

#include <algorithm>
#include <map>
#include <vector>

#include "actions.h"
#include "common.h"
#include "shared.h"
#include "utils.h"

/**
 * A complete listing of all the possible states that each client is capable of
 * being in.
 *
 * Keeping an explicit FSM in the code makes it _much_ easier to modify. The FSM
 * itself is implemented in ClientManager::state_transition
 */
enum ClientState {
    /// Active clients are visible to the user and can be interacted with
    CS_ACTIVE = 1,
    /// Visible clients are visible but do not have the input focused
    CS_VISIBLE,
    /// Iconified clients are not visible, but have an icon on the top of the screen
    CS_ICON,
    /// Invisible clients are on a different desktop from the one the user is 
    /// currently on
    CS_INVISIBLE,
    /// Clients which are currently being moved by the user
    CS_MOVING,
    /// Clients which are currently being resized by the user
    CS_RESIZING,
    /// A client has already been destroyed
    CS_DESTROY,
};

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
class ClientManager
{
public:
    /// Initialize the share data and the current desktop
    ClientManager(WMShared &shared) :
        m_shared(shared), m_current_desktop(1)
    {};

    bool is_client(Window);
    bool is_visible(Window);
    Icon *get_icon_of_client(Window);
    Icon *get_icon_of_icon(Window);
    Window get_from_placeholder(Window);

    void register_action(std::string, ClassActions);

    ClientState get_state(Window);
    void handle_motion(const XEvent&);
    void state_transition(Window, ClientState);

    void create(Window);
    void destroy(Window);
    void close(Window);

    void snap(Window, SnapDir);
    void maximize(Window);

    void redraw_icon(Window);

    void raise_layer(Window);
    void lower_layer(Window);
    void set_layer(Window, Layer);
    void relayer();

    void flip_sticky_flag(Window);
    void to_next_desktop(Window);
    void to_prev_desktop(Window);
    void update_desktop();

    void next_desktop();
    void prev_desktop();

private:
    void set_state(Window, ClientState);
    void apply_actions(Window);

    void map(Window);
    void unmap(Window);
        
    void focus(Window);
    void unfocus(Window);
    void unfocus_unsafe(Window);

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

    /// A relation between each client and its current layer
    std::map<Window, Layer> m_layers;

    /// The current desktop the user is viewing
    Desktop m_current_desktop;

    /// A relation between each client and its desktop
    std::map<Window, Desktop> m_desktops;

    /// A relation between each client and its stickiness
    std::map<Window, bool> m_sticky;
};
#endif
