/** @file */
#ifndef __SMALLWM_MODEL_CHANGE__
#define __SMALLWM_MODEL_CHANGE__

#include "common.h"

/**
 * This is the root of a hierarchy which forms the layer between the parts
 * of SmallWM which interact with Xlib directly, and the model which is
 * as Xlib independent as possible.
 */
struct Change
{
    ChangeType type;

    virtual bool is_layer_change() const
    { return false; }

    virtual bool is_focus_change() const
    { return false; }

    virtual bool is_client_desktop_change() const
    { return false; }

    virtual bool is_current_desktop_change() const
    { return false; }

    virtual bool is_location_change() const
    { return false; }

    virtual bool is_size_change() const
    { return false; }
};

/// Indicates a change in the stacking order of a window
struct ChangeLayer : Change
{
    ChangeLayer(Window win, Layer new_layer) :
        window(win), layer(new_layer)
    {};

    bool is_layer_change() const
    { return true; }

    Window window;
    Layer layer;
};

/// Indicates a change in the input focus
struct ChangeFocus : Change
{
    ChangeFocus(Window old_focus, Window new_focus) :
        type(CHANGE_FOCUS), prev_focus(old_focus), next_focus(new_focus)
    {};

    bool is_focus_change() const
    { return true; }

    Window prev_focus;
    Window next_focus;
};

/// Indicates a change in the desktop of a client
struct ChangeClientDesktop : Change
{
    ChangeClientDesktop(Window win, Desktop new_desktop) :
        type(CHANGE_CLIENT_DESKTOP), window(win), desktop(new_desktop)
    {};

    bool is_client_desktop_change() const
    { return true; }

    Window window;
    Desktop new_desktop;
};

/// Indicates a change in the currently visible desktop
struct ChangeCurrentDesktop : Change
{
    ChangeCurrentDesktop(Desktop new_desktop) :
        type(CHANGE_CURRENT_DESKTOP), desktop(new_desktop)
    {};

    bool is_current_desktop_change() const
    { return true; }

    Desktop desktop;
};

/// Indicates a change in the location of a window
struct ChangeLocation : Change
{
    ChangeLocation(Window win, Dimension _x, Dimension _y) :
        type(CHANGE_LOCATION), window(win), x(_x), y(_y)
    {};

    bool is_location_change() const
    { return true; }

    Window window;
    Dimension x;
    Dimension y;
};

/// Indicates a change in the size of a window
struct ChangeSize : Change
{
    ChangeSize(Window win, Dimension _w, Dimension _h) :
        window(win), w(_w), h(_h)
    {};

    bool is_size_change() const
    { return true; }

    Window window;
    Dimension w;
    Dimension h;
};

#endif
