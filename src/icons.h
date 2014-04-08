#ifndef __SMALLWM_ICONS__
#define __SMALLWM_ICONS__

#include "common.h"
#include "shared.h"

/**
 * Data which is used to draw an icon and to restore the client.
 */
struct Icon
{
    /// Initialize everything to zero.
    Icon() :
        window(None), client(None), gc(0), has_pixmap(false), pixmap(0),
        pixmap_size(0, 0)
    {};

    /// The client that this icon stands for
    Window client;

    /// The window that the icon's graphics are drawn upon
    Window window;

    /// The graphics context used to draw upon the window
    GC gc;

    /// Whether or not this icon has a graphical pixmap associated with it
    bool has_pixmap;

    /// The pixmap graphic itself
    Pixmap pixmap;

    /// The size of the pixmap
    Dimension2D pixmap_size;
};

/**
 * Manages the creation, removal, and drawing of icons.
 */
class IconManager
{
public:
    IconManager(WMShared &shared) :
        m_shared(shared)
    {};

    void redraw_icon(Window);

    Icon *get_icon_of_client(Window);
    Icon *get_icon_of_icon(Window);

protected:
    void create_icon(Window);
    void raise_icons();
    void reflow_icons();
    void delete_icon(Icon*);

private:
    /// A relation between icon windows and their owned icons
    std::map<Window,Icon*> m_icons;

    /// A lookup table for relating client windows to icons
    std::map<Window,Window> m_client_icons;

    /// The window manager share data
    WMShared &m_shared;
};
#endif
