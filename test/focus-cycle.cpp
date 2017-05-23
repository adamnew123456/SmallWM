#include <iostream>
#include <vector>

#include <UnitTest++.h>
#include "model/focus-cycle.h"

SUITE(FocusCycleSuite)
{
    TEST(cycle_starts_invalid)
    {
        // By default, the cycle should start invalidated
        FocusCycle cycle;
        CHECK(!cycle.valid());
    }

    TEST(cycle_starts_empty)
    {
        // By default, the cycle should start empty
        FocusCycle cycle;
        CHECK(cycle.empty());
    }

    TEST(cycle_stays_invalid_after_add)
    {
        // The cycle shouldn't change its own focus after a window is added
        FocusCycle cycle;
        cycle.add(1);
        CHECK(!cycle.valid());
    }

    TEST(cycle_not_empty_after_add)
    {
        // Cycles with elements shouldn't be empty
        FocusCycle cycle;
        cycle.add(1);
        CHECK(!cycle.empty());
    }

    TEST(cycle_add_after)
    {
        // Inserting after a particular element should work
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(3);
        cycle.add_after(2, 1);

        CHECK(!cycle.forward());
        CHECK_EQUAL(cycle.get(), 1);

        CHECK(!cycle.forward());
        CHECK_EQUAL(cycle.get(), 2);

        CHECK(!cycle.forward());
        CHECK_EQUAL(cycle.get(), 3);
    }

    TEST(cycle_bad_add_after)
    {
        // Inserting after a nonexistent element should not change anything
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(3);
        cycle.add_after(2, 42);

        CHECK(!cycle.forward());
        CHECK_EQUAL(cycle.get(), 1);

        CHECK(!cycle.forward());
        CHECK_EQUAL(cycle.get(), 3);
    }

    TEST(cycle_stays_invalid_after_bad_set)
    {
        // Setting to a bad value should invaldate the cycle
        FocusCycle cycle;
        cycle.add(1);
        cycle.set(1);
        CHECK(cycle.valid());

        cycle.set(42);
        CHECK(!cycle.valid());
    }

    TEST(cycle_unset)
    {
        // Unsetting the focus should invalidate the cycle
        FocusCycle cycle;
        cycle.add(1);
        cycle.set(1);

        cycle.unset();
        CHECK(!cycle.valid());
    }

    TEST(cycle_move_forward_empty_invalid)
    {
        // Moving forward in an empty cycle should do nothing but always wrap
        FocusCycle cycle;
        CHECK(cycle.forward());

        CHECK(!cycle.valid());
    }

    TEST(cycle_move_forward_invalid)
    {
        // Moving forward from an invalid state should put us at the beginning
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(2);
        CHECK(!cycle.forward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_move_backward_empty_invalid)
    {
        // Moving backward in an empty cycle should do nothing but always wrap
        FocusCycle cycle;
        CHECK(cycle.backward());

        CHECK(!cycle.valid());
    }

    TEST(cycle_move_backward_invalid)
    {
        // Moving backward from an invalid state should put us at the end
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(2);
        CHECK(!cycle.backward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 2);
    }

    TEST(cycle_add_set)
    {
        // The cycle should be settable after we add an element
        FocusCycle cycle;
        cycle.add(1);
        cycle.set(1);

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_remove_current_backward)
    {
        // Removing the current element should move backward when move_back is true
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(2);
        cycle.set(2);

        CHECK(cycle.remove(2, true));

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_remove_current_invalidate)
    {
        // Removing the current element should invalidate when move_back is false
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(2);
        cycle.set(2);

        CHECK(!cycle.remove(2, false));

        CHECK(!cycle.valid());
    }

    TEST(cycle_remove_current_backward_empty)
    {
        // Removing from a 1-length list should force us to invalidate, even
        // if we ask to move back
        FocusCycle cycle;
        cycle.add(1);
        cycle.set(1);

        CHECK(!cycle.remove(1, true));

        CHECK(!cycle.valid());
    }

    TEST(cycle_forward)
    {
        // Moving the cycle forward should changed the focused element (but
        // not wrap)
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(2);
        cycle.set(1);
        CHECK(!cycle.forward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 2);
    }

    TEST(cycle_backward)
    {
        // Moving the cycle backward should changed the focused element (but
        // not wrap)
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(2);
        cycle.set(2);
        CHECK(!cycle.backward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_forward_wrap)
    {
        // Going forward past the last element should cause the cycle to wrap
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(2);
        cycle.set(1);
        CHECK(!cycle.forward());
        CHECK(cycle.forward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_backward_wrap)
    {
        // Going backward past the first element should cause the cycle to wrap
        FocusCycle cycle;
        cycle.add(1);
        cycle.add(2);
        cycle.set(2);
        CHECK(!cycle.backward());
        CHECK(cycle.backward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 2);
    }

    TEST(cycle_subcycle_set)
    {
        // We should be able to set the current focus to something in the
        // subcycle
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;
        subcycle.add(2);

        cycle.set_subcycle(subcycle);
        cycle.set(2);

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 2);
    }

    TEST(cycle_forward_wrap_to_subcycle)
    {
        // If we wrap forward off the edge of our cycle, we should move into the subcycle
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;
        subcycle.add(2);

        cycle.set_subcycle(subcycle);
        cycle.set(1);

        CHECK(!cycle.forward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 2);
    }

    TEST(cycle_forward_wrap_from_subcycle)
    {
        // If we wrap forward off the edge of the subcycle, we should move into our cycle
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;
        subcycle.add(2);

        cycle.set_subcycle(subcycle);
        cycle.set(1);

        CHECK(!cycle.forward());
        CHECK(cycle.forward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_backward_wrap_to_subcycle)
    {
        // If we wrap backward off the edge of our cycle, we should move into the subcycle
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;
        subcycle.add(2);

        cycle.set_subcycle(subcycle);
        cycle.set(1);

        CHECK(!cycle.backward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 2);
    }

    TEST(cycle_backward_wrap_from_subcycle)
    {
        // If we wrap backward off the edge of the subcycle, we should move into our cycle
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;
        subcycle.add(2);

        cycle.set_subcycle(subcycle);
        cycle.set(1);

        CHECK(!cycle.backward());
        CHECK(cycle.backward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_no_elements_loop_subcycle)
    {
        // If there are no windows in the main cycle, we should be able to loop
        // around the subcycle
        FocusCycle cycle;
        FocusCycle subcycle;
        subcycle.add(1);

        cycle.set_subcycle(subcycle);
        cycle.set(1);

        CHECK(cycle.forward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_subcycle_backward_remove)
    {
        // If we remove the first element, we can fall backward into the subcycle.
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;
        subcycle.add(2);

        cycle.set_subcycle(subcycle);
        cycle.set(1);
        cycle.remove(1, true);

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 2);
    }

    TEST(cycle_subcycle_empty_forward)
    {
        // If the subcycle is empty, then cycling the top cycle should work as
        // before
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;

        cycle.set_subcycle(subcycle);
        cycle.set(1);
        CHECK(cycle.forward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_subcycle_empty_backward)
    {
        // If the subcycle is empty, then cycling the top cycle should work as
        // before
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;

        cycle.set_subcycle(subcycle);
        cycle.set(1);
        CHECK(cycle.backward());

        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_subcycle_becomes_invalid_forward)
    {
        // If the subcycle becomes invalid, then we should start the cycle at
        // the top of the main list after moving forward
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;
        subcycle.add(2);

        cycle.set_subcycle(subcycle);
        cycle.set(2);
        // Invalidate the subcycle by assigning a bogus focus
        subcycle.set(42);

        CHECK(!cycle.valid());

        // This should reset us to the start of the top cycle
        CHECK(cycle.forward());
        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 1);
    }

    TEST(cycle_subcycle_becomes_invalid_backward)
    {
        // If the subcycle becomes invalid, then we should start the cycle at
        // the back of the subcycle after moving backward
        FocusCycle cycle;
        cycle.add(1);

        FocusCycle subcycle;
        subcycle.add(2);

        cycle.set_subcycle(subcycle);
        cycle.set(2);
        // Invalidate the subcycle by assigning a bogus focus
        subcycle.set(42);

        CHECK(!cycle.valid());

        // This should reset us to the back of the subcycle
        CHECK(cycle.backward());
        CHECK(cycle.valid());
        CHECK_EQUAL(cycle.get(), 2);
    }
}

int main()
{
    return UnitTest::RunAllTests();
}
