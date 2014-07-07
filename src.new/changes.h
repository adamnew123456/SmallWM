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

    virtual bool operator==(const Change &other)
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

    bool operator==(const Change &other)
    {
        if (!other.is_layer_change())
            return false;

        ChangeLayer *cast_other = (ChangeLayer*)&other;
        return (cast_other->window == window &&
                cast_other->layer == layer);
    }

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

    bool operator==(const Change &other)
    {
        if (!other.is_focus_change())
            return false;

        ChangeFocus *cast_other = (ChangeFocus*)&other;
        return (cast_other->prev_focus == prev_focus &&
                cast_other->next_focus == next_focus);
    }

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

    bool operator==(const Change &other)
    {
        if (!other.is_client_desktop_change())
            return false;

        ChangeClientDesktop *cast_other = (ChangeClientDesktop*)&other;
        return (cast_other->window == window &&
                cast_other->new_desktop == new_desktop);
    }

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

    bool operator==(const Change &other)
    {
        if (!other.is_current_desktop_change())
            return false;

        ChangeCurrentDesktop *cast_other = (ChangeCurrentDesktop*)&other;
        return cast_other->desktop == desktop;
    }

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

    bool operator==(const Change &other)
    {
        if (!other.is_location_change())
            return false;

        ChangeLocation *cast_other = (ChangeLocation*)&other;
        return (cast_other->window == window &&
                cast_other->x == x &&
                cast_other->y == y);
    }

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

    bool operator==(const Change &other)
    {
        if (!other.is_size_change())
            return false;

        ChangeSize *cast_other = (ChangeSize*)&other;
        return (cast_other->window == window &&
                cast_other->w == w &&
                cast_other->h == h);
    }

    Window window;
    Dimension w;
    Dimension h;
};
#endif
