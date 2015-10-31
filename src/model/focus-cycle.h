/** @file */
#ifndef __SMALLWM_FOCUS_CYCLE__
#define __SMALLWM_FOCUS_CYCLE__

#include <map>
#include <vector>

#include "common.h"
#include "logging/logging.h"

/**
 * Handles focus cycling, which is the ability to use a single keyboard
 * action to focus each visible window in turn, rather than having to click
 * them.
 */
class FocusCycle
{
public:
    FocusCycle(Log *logger) :
        m_logger(logger)
    {};

    void update_window_list(const std::vector<Window>&);
    void set_focus(Window);
    Window get_next();
    Window get_prev();

private:
    /// The index of the currently focused window.
    unsigned int m_current_focus;

    /// The table of windows, and their locations in the focus cycle
    std::map<Window, unsigned int> m_window_indexes;

    /// The list of windows
    std::vector<Window> m_focus_list;

    /// The current logger
    Log *m_logger;
};

#endif
