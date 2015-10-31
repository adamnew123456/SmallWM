#include <iostream>
#include <vector>

#include <UnitTest++.h>
#include "model/focus-cycle.h"
#include "logging/logging.h"
#include "logging/stream.h"

const int last_window = 9;
StreamLog log(std::cerr);

struct FocusCycleFixture
{
    FocusCycle cycle;
    std::vector<Window> windows;

    FocusCycleFixture() :
        cycle(&log)
    {
        for (int win = 0; win < last_window + 1; win++) 
        {
            windows.push_back(win);
        }

        cycle.update_window_list(windows);
    }
};

SUITE(FocusModelSuite)
{
    /**
     * Ensures that FocusCycle.get_next returns the correct value in cases when
     * it doesn't wrap around.
     */
    TEST_FIXTURE(FocusCycleFixture, test_gets_windows)
    {
        // It starts at the first window, but get_next() doesn't return it, so
        // we have to start at the second window.
        for (int win = 1; win < last_window + 1; win++)
        {
            CHECK_EQUAL(cycle.get_next(), win);
        }
    }

    /**
     * Ensures that FocusCycle.get_prev returns the correct value in cases when
     * it doesn't wrap around.
     */
    TEST_FIXTURE(FocusCycleFixture, test_gets_windows_reverse)
    {
        cycle.set_focus(last_window);

        // It starts at the first window, but get_next() doesn't return it, so
        // we have to start at the second window.
        for (int win = last_window - 1; win >= 0; win--)
        {
            CHECK_EQUAL(cycle.get_prev(), win);
        }
    }
    
    /**
     * This ensures that the FocusCycle wraps around when it gets past the 
     * last window.
     */
    TEST_FIXTURE(FocusCycleFixture, test_wraparound)
    {
        for (int win = 1; win < last_window + 1; win++)
        {
            cycle.get_next();
        }

        CHECK_EQUAL(cycle.get_next(), 0);
    }

    /**
     * This ensures that the FocusCycle wraps around when it gets past the 
     * first window (cycling backwards)
     */
    TEST_FIXTURE(FocusCycleFixture, test_wraparound_reverse)
    {
        cycle.set_focus(last_window);
        for (int win = last_window - 1; win >= 0; win--)
        {
            cycle.get_prev();
        }

        CHECK_EQUAL(cycle.get_prev(), last_window);
    }

    /**
     * This ensures that setting the focus to a valid window causes the window
     * after it to be returned by get_next.
     */
    TEST_FIXTURE(FocusCycleFixture, test_set_focus)
    {
        // As before, setting the focus means that the window *after* the current
        // focus will be returned by get_next
        cycle.set_focus(5);
        for (int win = 6; win < last_window; win++)
        {
            CHECK_EQUAL(cycle.get_next(), win);
        }
    }

    /**
     * This ensures that setting the focus to an invalid window does nothing.
     */
    TEST_FIXTURE(FocusCycleFixture, test_set_invalid)
    {
        // As before, setting the focus means that the window *after* the current
        // focus will be returned by get_next
        cycle.set_focus(100);
        CHECK_EQUAL(cycle.get_next(), 1);
    }
}

int main()
{
    return UnitTest::RunAllTests();
}
