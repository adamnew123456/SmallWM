#include "focus-cycle.h"

/**
 * Adds a new window to the end of the focus cycle.
 */
void FocusCycle::add(Window window)
{
    m_windows.push_back(window);
}

/**
 * Adds a new window immediately after the other window. If the other window
 * is not present, then nothing happens.
 */
void FocusCycle::add_after(Window window, Window after)
{
    std::list<Window>::iterator insert_pos = std::find(m_windows.begin(),
                                                       m_windows.end(),
                                                       after);

    if (insert_pos == m_windows.end())
        return;

    insert_pos++;
    m_windows.insert(insert_pos, window);
}

/**
 * Removes a window from the focus cycle.
 *
 * If the focused window is removed, one of two things can happen:
 * (1) If move_back is true, and the window list has at least 1 other element,
 *     the focus will move backward.
 * (2) Otherwise, the focus is invalidated.
 *
 * If the focus ends up being invalidated, then false is returned. If the focus is
 * still valid, true is returned.
 */
bool FocusCycle::remove(Window window, bool move_back)
{
    if (m_currently_focused && *m_current_focus == window)
    {
        bool subcycle_has_windows = m_has_subcycle && !m_subcycle->empty();

        if (move_back && (m_windows.size() > 1 || subcycle_has_windows))
            backward();
        else
            m_currently_focused = false;
    }

    m_windows.remove(window);
    return m_currently_focused;
}

/**
 * Sets the current subcycle, if there isn't already one.
 */
void FocusCycle::set_subcycle(FocusCycle &subcycle)
{
    if (m_has_subcycle)
        return;

    m_has_subcycle = true;
    m_subcycle_in_use = false;
    m_subcycle = &subcycle;
}

/**
 * Clears the current subcycle, if there is one.
 */
void FocusCycle::clear_subcycle()
{
    m_has_subcycle = false;
    m_subcycle_in_use = false;
}

/**
 * Checks to see if the current focus is valid.
 */
bool FocusCycle::valid() const
{
    return m_currently_focused || (m_subcycle_in_use && m_subcycle->valid());
}

/**
 * Returns true if there are no windows in this cycle (or its subcycle)
 */
bool FocusCycle::empty() const
{
    bool empty = m_windows.empty();
    if (empty && m_has_subcycle)
        return m_subcycle->empty();
    else
        return empty;
}

/**
 * Retrieves the current focus, either from us or possibly the subcycle, if
 * it is active.
 */
Window FocusCycle::get() const
{
    if (m_subcycle_in_use)
        return m_subcycle->get();
    else
        return *m_current_focus;
}

/**
 * Sets the current focus to the given window (which could possibly be in a
 * subcycle)
 *
 * If the window is not present then the current focus is invalidated and
 * false is returned. Otherwise, true is returned.
 */
bool FocusCycle::set(Window window)
{
    m_current_focus = std::find(m_windows.begin(),
                                m_windows.end(),
                                window);

    m_currently_focused = m_current_focus != m_windows.end();

    if (!m_currently_focused && m_has_subcycle)
        m_subcycle_in_use = m_subcycle->set(window);
    else if (m_currently_focused)
        m_subcycle_in_use = false;

    return m_currently_focused || m_subcycle_in_use;
}

/**
 * Removes the current focus.
 */
void FocusCycle::unset()
{
    m_subcycle_in_use = false;
    m_currently_focused = false;
}

/**
 * Moves the focus forward, which can have several different results:
 *
 * (1) If the current focus is invalid, then the focus is set to the
 *     first window in the cycle.
 * (2) If the current focus is the last window in the cycle, and:
 *   (a) There is a subcycle: Moves the focus into the subcycle.
 *       (i) If the subcycle wraps, and there are windows in our cycle, then
 *           the focus moves to the first window in our cycle.
 *       (ii) If the subcycle wraps, and there are not windows in our cycle,
 *            then the focus stays in the subcycle after it wraps.
 *   (b) There is not a subcycle: Moves the focus to the first window in the
 *       cycle.
 *
 *  Returns true if we wrapped around to the front, or false otherwise.
 */
bool FocusCycle::forward()
{
    bool we_wrapped = false;

    // If the subcycle became invalid since the last time we looked at it,
    // then we should also reset
    if (m_subcycle_in_use && !m_subcycle->valid())
    {
        we_wrapped = true;
        m_subcycle_in_use = false;
    }

    if (m_subcycle_in_use)
    {
        bool subcycle_wrapped = m_subcycle->forward();

        if (subcycle_wrapped && !m_windows.empty())
        {
            we_wrapped = true;
            m_current_focus = m_windows.begin();
            m_currently_focused = true;
            m_subcycle_in_use = false;
        }
        else if (subcycle_wrapped)
            we_wrapped = true;
    }
    else if (m_currently_focused)
    {
        m_current_focus++;
        if (m_current_focus == m_windows.end())
        {
            if (m_has_subcycle && !m_subcycle->empty())
            {
                m_subcycle_in_use = true;
                m_currently_focused = false;

                if (!m_subcycle->valid())
                    m_subcycle->forward();

            }
            else
            {
                we_wrapped = true;
                m_current_focus = m_windows.begin();
            }
        }
    }
    else if (!m_windows.empty())
    {
        m_currently_focused = true;
        m_current_focus = m_windows.begin();
    }
    else if (m_has_subcycle && !m_subcycle->empty())
    {
        m_currently_focused = false;
        m_subcycle_in_use = true;

        if (!m_subcycle->valid())
            m_subcycle->forward();
    }
    else
        we_wrapped = true; // No to anything, but we did hit the end

    return we_wrapped;
}

/**
 * Moves the focus backward. See forward() for how the focus is determined;
 * there are several cases to consider.
 *
 *  Returns true if we wrapped around to the back, or false otherwise.
 */
bool FocusCycle::backward()
{
    bool we_wrapped = false;

    if (m_subcycle_in_use && !m_subcycle->valid())
    {
        we_wrapped = true;
        m_subcycle_in_use = false;
    }

    if (m_subcycle_in_use)
    {
        bool subcycle_wrapped = m_subcycle->backward();

        if (subcycle_wrapped && !m_windows.empty())
        {
            we_wrapped = true;
            m_current_focus = m_windows.end();
            m_current_focus--;
            m_currently_focused = true;
            m_subcycle_in_use = false;
        }
        else if (subcycle_wrapped)
            we_wrapped = true;
    }
    else if (m_currently_focused)
    {
        if (m_current_focus == m_windows.begin())
        {
            if (m_has_subcycle && !m_subcycle->empty())
            {
                m_subcycle_in_use = true;
                m_currently_focused = false;

                if (!m_subcycle->valid())
                    m_subcycle->backward();
            }
            else
            {
                we_wrapped = true;
                m_current_focus = m_windows.end();
                m_current_focus--;
            }
        }
        else
            m_current_focus--;
    }
    else if (m_has_subcycle && !m_subcycle->empty())
    {
        m_currently_focused = false;
        m_subcycle_in_use = true;

        if (!m_subcycle->valid())
            m_subcycle->backward();
    }
    else if (!m_windows.empty())
    {
        m_currently_focused = true;
        m_current_focus = m_windows.end();
        m_current_focus--;

    }
    else
        we_wrapped = true; // No to anything, but we did hit the end

    return we_wrapped;
}

/**
 * Converts the focus cycle to a textual representation, which is written to
 * an output stream.
 */
void FocusCycle::dump(std::ostream &output, int depth)
{

    // Starting at 2 to offset from the desktop name (see ClientModel::dump)
    // .. Focus cycle         0 => 2
    // .... data
    // ...... Focus cycle     1 => 6
    // ........ data
    // .......... Focus cycle 2 => 10
    // ............ data
    std::string indent = std::string(2 + 4*depth, ' ');

    output << indent << "Focus cycle\n";
    output << indent << "  Has a focused window? " <<
        (m_currently_focused ? "yes" : "no") << "\n";

    for (std::list<Window>::iterator winiter = m_windows.begin();
         winiter != m_windows.end();
         winiter++)
    {
        output << indent << "  " << std::hex << *winiter;

        if (m_currently_focused && m_current_focus == winiter)
            output << "*";

        output << "\n";
    }

    output << indent << "  Subcycle? " <<
        (m_has_subcycle ? "yes" : "no") << "\n";

    if (m_has_subcycle)
        m_subcycle->dump(output, depth + 1);
}
