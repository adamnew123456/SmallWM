#include "focus-cycle.h"

/**
 * Updates the list of currently visible windows.
 *
 * @param[in] The list of currently visible windows.
 */
void FocusCycle::update_window_list(const std::vector<Window> &windows)
{
    m_focus_list.clear();
    m_window_indexes.clear();

    // Unfortunately, C++ doesn't support multiple inline declarations, unlike
    // C99. Grumble grumble...
    unsigned int win_idx = 0;
    for (std::vector<Window>::const_iterator win_iter = windows.begin();
         win_iter != windows.end();
         win_iter++, win_idx++)
    {
        m_focus_list.push_back(*win_iter);
        m_window_indexes[*win_iter] = win_idx;
    }

    m_current_focus = 0;
}

/**
 * Updates the currently focused window.
 *
 * @param window The window that is currently focused.
 */
void FocusCycle::set_focus(Window window)
{
    if (window == None)
        return;

    if (m_window_indexes.count(window) == 0)
    {
        // This window isn't actually inside the known list of windows. Since
        // the window list needs to be updated every time it changes, this
        // shouldn't happen.
        m_logger->log(LOG_ERR) <<
            "Tried to change focus to a window that FocusCycle does not know "
            "about." << Log::endl;
        return;
    }

    m_current_focus = m_window_indexes[window];
}

/**
 * Finds the next window which can get the focus.
 *
 * @return The next window, or None.
 */
Window FocusCycle::get_next()
{
    if (m_focus_list.size() == 0)
        return None;

    if (m_current_focus == m_focus_list.size() - 1)
        m_current_focus = 0;
    else
        m_current_focus++;

    return m_focus_list[m_current_focus];
}

/**
 * Finds the previous window which can get the focus.
 * 
 * @return The previous window or none.
 */
Window FocusCycle::get_prev()
{
    if (m_focus_list.size() == 0)
        return None;

    if (m_current_focus == 0)
        m_current_focus = m_focus_list.size() - 1;
    else
        m_current_focus--;

    return m_focus_list[m_current_focus];
}
