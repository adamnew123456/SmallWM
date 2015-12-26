/** @file */
#ifndef __SMALLWM_MODEL_CHANGE__
#define __SMALLWM_MODEL_CHANGE__

#include <memory>
#include <ostream>
#include <queue>
#include <vector>

#include "common.h"
#include "desktop-type.h"

/**
 * This is the root of a hierarchy which forms the layer between the parts
 * of SmallWM which interact with Xlib directly, and the model which is
 * as Xlib independent as possible.
 *
 * This layer is used to notify the Xlib-interacting parts that changes have
 * occurred which require some kind of change in the user interface.
 */
struct Change
{
    virtual ~Change()
    {};

    virtual bool is_layer_change() const
    { return false; }

    virtual bool is_focus_change() const
    { return false; }

    virtual bool is_client_desktop_change() const
    { return false; }

    virtual bool is_current_desktop_change() const
    { return false; }

    virtual bool is_screen_change() const
    { return false; }

    virtual bool is_mode_change() const
    { return false; }

    virtual bool is_location_change() const
    { return false; }

    virtual bool is_size_change() const
    { return false; }

    virtual bool is_destroy_change() const
    { return false; }

    virtual bool is_unmap_change() const
    { return false; }

    virtual bool operator==(const Change &other) const
    { return false; }
};

static std::ostream &operator<<(std::ostream &out, const Change &change)
{
    out << "[Change]";
    return out;
}

/// Indicates a change in the stacking order of a window
struct ChangeLayer : Change
{
    ChangeLayer(Window win, Layer new_layer) :
        window(win), layer(new_layer)
    {};

    bool is_layer_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_layer_change())
            return false;

        const ChangeLayer &cast_other = dynamic_cast<const ChangeLayer&>(other);
        return (cast_other.window == window &&
                cast_other.layer == layer);
    }

    Window window;
    Layer layer;
};

static std::ostream &operator<<(std::ostream &out, const ChangeLayer &change)
{
    out << "[ChangeLayer Window<" << change.window << 
        "> Layer(" << static_cast<int>(change.layer) << ")]";
    return out;
}

/// Indicates a change in the input focus
struct ChangeFocus : Change
{
    ChangeFocus(Window old_focus, Window new_focus) :
        prev_focus(old_focus), next_focus(new_focus)
    {};

    bool is_focus_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_focus_change())
            return false;

        const ChangeFocus &cast_other = dynamic_cast<const ChangeFocus&>(other);
        return (cast_other.prev_focus == prev_focus &&
                cast_other.next_focus == next_focus);
    }

    Window prev_focus;
    Window next_focus;
};

static std::ostream &operator<<(std::ostream &out, const ChangeFocus &change)
{
    out << "[ChangeFocus Window<" << change.prev_focus << "> ==> Window<" 
        << change.next_focus << ">]";
    return out;
}

/// Indicates a change in the desktop of a client
struct ChangeClientDesktop : Change
{
    ChangeClientDesktop(Window win, const Desktop *old_desktop, const Desktop *new_desktop) :
        window(win), prev_desktop(old_desktop), next_desktop(new_desktop)
    {};

    bool is_client_desktop_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_client_desktop_change())
            return false;

        const ChangeClientDesktop &cast_other = dynamic_cast<const ChangeClientDesktop&>(other);

        // This is important - the way that desktop equality is checked below
        // is that either:
        //
        // (1) Both 'prev_desktop' members are NULL OR
        // (2) Both derefences 'prev_desktop' members are equal.
        //
        // If one or the other is NULL, but not both, proceeding onto (2) would
        // cause the program to crash. We want to avoid that outcome, so the
        // possibility of either being NULL without them both being NULL is
        // handled here.
        if ((cast_other.prev_desktop == 0 && prev_desktop != 0) ||
                (cast_other.prev_desktop != 0 && prev_desktop == 0))
            return false;

        if ((cast_other.next_desktop == 0 && next_desktop != 0) ||
                (cast_other.next_desktop != 0 && next_desktop == 0))
            return false;

        return (cast_other.window == window &&
                (cast_other.prev_desktop == prev_desktop ||
                 *cast_other.prev_desktop == *prev_desktop) &&
                (cast_other.next_desktop == next_desktop ||
                 *cast_other.next_desktop == *next_desktop));
    }

    Window window;
    const Desktop *prev_desktop;
    const Desktop *next_desktop;
};

static std::ostream &operator<<(std::ostream &out, const ChangeClientDesktop &change)
{
    out << "[ChangeClientDesktop Window<" << change.window << "> Desktop(" 
        << *change.prev_desktop << "-->" << *change.next_desktop <<  ")]";
    return out;
}

/// Indicates a change in the currently visible desktop
struct ChangeCurrentDesktop : Change
{
    ChangeCurrentDesktop(const Desktop *old_desktop, const Desktop *new_desktop) :
        prev_desktop(old_desktop), next_desktop(new_desktop)
    {};

    bool is_current_desktop_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_current_desktop_change())
            return false;

        const ChangeCurrentDesktop &cast_other = 
            dynamic_cast<const ChangeCurrentDesktop&>(other);

        if ((cast_other.prev_desktop == 0 && prev_desktop != 0) ||
                (cast_other.prev_desktop != 0 && prev_desktop == 0))
            return false;

        if ((cast_other.next_desktop == 0 && next_desktop != 0) ||
                (cast_other.next_desktop != 0 && next_desktop == 0))
            return false;

        return ((cast_other.prev_desktop == prev_desktop ||
                 *cast_other.prev_desktop == *prev_desktop) &&
                (cast_other.next_desktop == next_desktop ||
                 *cast_other.next_desktop == *next_desktop));
    }

    const Desktop *prev_desktop;
    const Desktop *next_desktop;
};

static std::ostream &operator<<(std::ostream &out, const ChangeCurrentDesktop &change)
{
    out << "[ChangeCurrentDesktop Desktop(" << *change.prev_desktop << "-->" << 
        *change.next_desktop << ")]";
    return out;
}

/// Indicates a change in the client's monitor
struct ChangeScreen : Change
{
    ChangeScreen(Window win, Box &_bounds) :
        window(win), bounds(_bounds)
    {};

    bool is_screen_change() const
    { return true; }


    virtual bool operator==(const Change &other) const
    {
        if (!other.is_screen_change())
            return false;

        const ChangeScreen &cast_other = 
            dynamic_cast<const ChangeScreen&>(other);

        return cast_other.window == window && cast_other.bounds == bounds;
    }

    Window window;
    Box &bounds;
};

static std::ostream &operator<<(std::ostream &out, const ChangeScreen &change)
{
    out << "[ChangeScreen] Window" << change.window << ") " << change.bounds << "\n";
    return out;
}

/// Indicates a change in the client's position/scale mode
struct ChangeCPSMode : Change
{
    ChangeCPSMode(Window win, ClientPosScale _mode) :
        window(win), mode(_mode)
    {};

    bool is_mode_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_mode_change())
            return false;

        const ChangeCPSMode &cast_other = 
            dynamic_cast<const ChangeCPSMode&>(other);

        return cast_other.window == window && cast_other.mode == mode;
    }

    Window window;
    ClientPosScale mode;
};

/// Indicates a change in the location of a window
struct ChangeLocation : Change
{
    ChangeLocation(Window win, Dimension _x, Dimension _y) :
        window(win), x(_x), y(_y)
    {};

    bool is_location_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_location_change())
            return false;

        const ChangeLocation &cast_other = dynamic_cast<const ChangeLocation&>(other);
        return (cast_other.window == window &&
                cast_other.x == x &&
                cast_other.y == y);
    }

    Window window;
    Dimension x;
    Dimension y;
};

static std::ostream &operator<<(std::ostream &out, const ChangeLocation &change)
{
    out << "[ChangeLocation Window<" << change.window 
        << "> Location(" << change.x << "," << change.y << ")]";
    return out;
}

/// Indicates a change in the size of a window
struct ChangeSize : Change
{
    ChangeSize(Window win, Dimension _w, Dimension _h) :
        window(win), w(_w), h(_h)
    {};

    bool is_size_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_size_change())
            return false;

        const ChangeSize &cast_other = dynamic_cast<const ChangeSize&>(other);
        return (cast_other.window == window &&
                cast_other.w == w &&
                cast_other.h == h);
    }

    Window window;
    Dimension w;
    Dimension h;
};

static std::ostream &operator<<(std::ostream &out, const ChangeSize &change)
{
    out << "[ChangeSize Window<" << change.window 
        << "> Size(" << change.w << "," << change.h << ")]";
    return out;
}

/// Indicates that a client has been removed from the model
struct DestroyChange : Change
{
    DestroyChange(Window win, const Desktop *old_desktop, Layer old_layer) :
        window(win), desktop(old_desktop), layer(old_layer)
    {};

    bool is_destroy_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_destroy_change())
            return false;

        const DestroyChange &cast_other = 
            dynamic_cast<const DestroyChange&>(other);
        return (cast_other.window == window &&
                (cast_other.desktop == desktop ||
                 *cast_other.desktop == *desktop) &&
                cast_other.layer == layer);
    }

    Window window;
    const Desktop *desktop;
    Layer layer;
};

/**
 * Indicates that a client is still in the model, but requires special 
 * because it is no longer valid (this is a kludge to handle unmapped
 * windows, without destroying their state inside SmallWM)
 */
struct UnmapChange : Change
{
    UnmapChange(Window win) : window(win)
    {};

    bool is_unmap_change() const
    { return true; }

    virtual bool operator==(const Change &other) const
    {
        if (!other.is_unmap_change())
            return false;

        const UnmapChange &cast_other =
            dynamic_cast<const UnmapChange&>(other);
        return cast_other.window == window;
    }

    Window window;
};

/**
 * Contains a series of changes. Changes can be pushed to the ChangeStream, and
 * then retrieved later.
 */
class ChangeStream
{
public:
    typedef Change const * change_ptr;
    typedef std::vector<change_ptr>::iterator change_iter;

    bool has_more();
    change_ptr get_next();

    void push(change_ptr);
    void flush();

private:
    std::queue<change_ptr> m_changes;
};

#endif
