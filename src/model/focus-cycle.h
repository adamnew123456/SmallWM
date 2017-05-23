/** @file */
#ifndef __SMALLWM_FOCUS_CYCLE__
#define __SMALLWM_FOCUS_CYCLE__

#include <algorithm>
#include <list>

#include "common.h"

/**
 * Handles a focus cycle for a group of windows, which provides an order
 * across those windows as well as a 'cursor' that can be used to move
 * the focus forward/backward.
 */
class FocusCycle
{
public:
    FocusCycle() :
        m_currently_focused(false),
        m_has_subcycle(false),
        m_subcycle_in_use(false),
        m_subcycle(NULL)
    {};

    void add(Window);
    void add_after(Window, Window);
    bool remove(Window, bool);

    void set_subcycle(FocusCycle&);
    void clear_subcycle();

    bool empty();
    bool valid();
    Window get();
    bool set(Window);
    void unset();

    bool forward();
    bool backward();

private:
    /// Whether or not the current focus is actually on any window
    bool m_currently_focused;

    /// The list of windows attached to this cycle
    std::list<Window> m_windows;

    /**
     * The currently focused window - note that this might be invalid if no
     * window is currently focused
     */
    std::list<Window>::iterator m_current_focus;

    /// Whether or not there is a subcycle
    bool m_has_subcycle;

    /// Whether we are using our elements, or waiting on the subcycle
    bool m_subcycle_in_use;

    /// The currently attached subcycle, if any
    FocusCycle *m_subcycle;
};

#endif
