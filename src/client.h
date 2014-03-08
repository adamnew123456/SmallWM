/** @file */
#ifndef __SMALLWM_CLIENT__
#define __SMALLWM_CLIENT__

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "actions.h"
#include "common.h"
#include "shared.h"

/**
 * Whether a ClientManager is moving or resizing a client, or doing neither.
 */
enum MoveResizeState {
    /// The ClientManager is currently moving a client.
    MVR_MOVE,
    /// The ClientManager is currently resizing a client.
    MVR_RESIZE,
    /// The ClientManager is doing neither.
    MVR_NONE,
};

// Clients and sub-managers need to know their manager to make redraw requests.
class ClientManager;

/**
 * An individual window manager client, which is a layer on top of a single X11 window.
 */
class Client {
public:
    Client(const WMShared&, ClientManager&, const ClassActions&, Window);
    ~Client();

    Window window() const;
    void snap(SnapDir);
    void maximize();
    void run_actions(std::shared_ptr<Client>);
    void close();
    void destroy();

    void activate();
    void deactivate();

private:
    /// The ClientManager that owns this window.
    ClientManager &m_manager;
    
    /// All the state this client shares with the window manager.
    const WMShared &m_shared;

    /// The actions to do whenever this client is drawn to the screen.
    const ClassActions &m_actions;

    /// The X11 window of this client.
    Window m_window;

    /// Whether or not this client can be interacted with or not.
    bool m_active;
};

/// A shorter way of referring to a client managed via the ClientManager.
typedef std::shared_ptr<Client> ClientRef;

/**
 * Manages how clients are layered on top of each other.
 */
class LayerManager {
public:
    /// Initialize the manager.
    LayerManager(WMShared &shared, ClientManager &manager) :
        m_manager(manager), m_shared(shared)
    {};

    void set_layer(ClientRef, Layer);
    void move_up(ClientRef);
    void move_down(ClientRef);
    
    void set_as_dialog(ClientRef);
    void relayer_clients();

    void remove(ClientRef);
private:
    /// A relation between each client and its layer.
    std::map<ClientRef, Layer> m_layers;

    /// Shared window-manager data.
    WMShared &m_shared;

    /// The owner of this layer manager.
    ClientManager &m_manager;
};

/**
 * Manages how clients are arranged among virtual desktops.
 */
class DesktopManager {
public:
    /// Initialize the default desktop and the manager.
    DesktopManager(WMShared &shared, ClientManager &manager) : 
        m_current_desktop(1),
        m_manager(manager), m_shared(shared)
    {}

    void set_desktop(ClientRef, Desktop);
    void next_desktop(ClientRef);
    void prev_desktop(ClientRef);

    void flip_sticky_flag(ClientRef);
    void show_next_desktop();
    void show_prev_desktop();
    void redraw_clients();

    void remove(ClientRef);
private:
    /// A relation between each client and its desktop.
    std::map<ClientRef, Desktop> m_desktops;

    /// An relation between each client and its stickiness.
    std::map<ClientRef, bool> m_stickies;

    /// The current desktop which is being viewed.
    Desktop m_current_desktop;

    /// Shared window-manager data.
    WMShared &m_shared;

    /// The owner of this DesktopManager.
    ClientManager &m_manager;
};

/**
 * Manages how clients are moved and resized.
 */
class MoveResizeManager {
public:
    /// Initialize the manager.
    MoveResizeManager(WMShared &shared, ClientManager &manager) :
        m_manager(manager), m_shared(shared), m_state(MVR_NONE)
    {};

    void begin_move(ClientRef, Dimension, Dimension);
    void begin_resize(ClientRef, Dimension, Dimension);
    void handle_motion_event(const XEvent&);
    void end_move_resize();

    bool is_being_moved(Window) const;
    void raise_placeholder() const;

private:
    void create_placeholder(Window, XWindowAttributes);

    /// What, if anything, the MoveResizeManager is doing right now.
    MoveResizeState m_state;

    /// The absolute location of the pointer
    Dimension2D m_ptr_loc;

    /// The old location/size of the client's window
    Dimension2D m_old_params;

    /// The client whose window is currently being moved.
    ClientRef m_client;

    /// The placeholder which is displayed instead of the client.
    Window m_placeholder;

    /// Shared window-manager data.
    WMShared &m_shared;

    /// The owner of this MoveResizeManager.
    ClientManager &m_manager;
};

/**
 * Manages which windows are focused and which are not.
 */
class FocusManager {
public:
    /// Initialize the manager
    FocusManager(WMShared &shared, ClientManager &manager) :
        m_manager(manager), m_shared(shared),
        m_focused(None)
    {};

    void change_focus(Window);
    void unfocus();
    Window get_focused() const;

private:
    /// The currently focused window.
    Window m_focused;
    
    /// Shared window-manager data.
    WMShared &m_shared;

    /// The owner of this FocusManager.
    ClientManager &m_manager;
};

/**
 * A single icon, which is used as a proxy for a single client when it is hidden.
 */
struct Icon {
    /// The window used for drawing the icon.
    Window window;

    /// The graphics context used to draw the pixmap and icon text.
    GC gc;

    /// Whether or not this icon has a graphical portion.
    bool has_pixmap;
    /// The graphical icon itself, if this has one.
    Pixmap pixmap;
    /// The dimensions of the pixmap;
    Dimension2D pixmap_size;
};

/**
 * Manages hiding clients, unhiding them, and repainting their icons.
 */
class IconManager {
public:
    /// Initialize the manager.
    IconManager(WMShared &shared, ClientManager &manager) :
        m_manager(manager), m_shared(shared)
    {};

    void to_icon(ClientRef);
    void from_icon(Window);

    void redraw_icon(Window);
    void relayer_icons();
    void reflow_icons();

    // TODO: Start here
    bool is_iconified(Window);
    void remove(ClientRef);
private:
    /// Relates each icon to its client.
    std::map<Icon*,ClientRef> m_icons;

    /// Relates each icon window to its icon.
    std::map<Window,Icon*> m_icon_wins;

    /// Relates each 

    /// Shared window-manager data.
    WMShared &m_shared;

    /// The owner of this IconManager.
    ClientManager &m_manager;
};

/**
 * A single entity which manages all the current client windows, as well as all dialogs.
 */
class ClientManager {
public:
    /// Initialize some variables
    ClientManager(WMShared &shared) : 
        m_shared(shared), m_desktops(shared, *this),
        m_layers(shared, *this), m_moveresize(shared, *this),
        m_focus(shared, *this), m_icons(shared, *this)
    {};

    void register_classaction(std::string, ClassActions);

    void create_client(Window);
    ClientRef get_client(Window);
    void remove_client(Window);

    /// Gets the DesktopManager assigned to this instance.
    DesktopManager &desktops()
        { return m_desktops; };

    /// Gets the LayerManager assigned to this instance.
    LayerManager &layers()
        { return m_layers; };

    /// Gets the MoveResizeManager assigned to this instance.
    MoveResizeManager &moveresize()
        { return m_moveresize; };

    /// Gets the FocusManager assigned to this instance.
    FocusManager &focus()
        { return m_focus; };

    /// Gets the IconManager assigned to this instance. 
    IconManager &icons()
        { return m_icons; }

private:
    /// The shared state owned by the window manager.
    WMShared &m_shared;

    /// An association between class names and their actions.
    std::map<std::string, ClassActions> m_actions;

    /// An association between an X11 window and its client.
    std::map<Window, ClientRef> m_clients;

    /// A collection which holds the desktop and stickiness of each ClientRef.
    DesktopManager m_desktops;

    /// A collection which holds the layer of each ClientRef.
    LayerManager m_layers;

    /// A layer which handles how clients are moved and resized.
    MoveResizeManager m_moveresize;

    /// A layer which handles which client is focused.
    FocusManager m_focus;

    /// A layer which handles how icons are shown and interacted with.
    IconManager m_icons;
};
#endif
