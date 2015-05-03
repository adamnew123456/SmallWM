#include <iostream>
#include <vector>

#include <UnitTest++.h>
#include "model/screen.h"

// Useful for doing 'screen_of_coord' testing so that I can avoid repeating code
struct CrtOfCoordTest 
{
    int x, y;
    Crt *expected_screen;
};

void test_screen_expectations(CrtManager &mgr, CrtOfCoordTest *tests, int n_tests)
{
    std::cout << "TESTING\n";
    for (int i = 0; i < n_tests; i++)
    {
        CrtOfCoordTest test = tests[i];
        std::cout << "\t(" << test.x << ", " << test.y << ") --> " << test.expected_screen << "\n";
        CHECK_EQUAL(mgr.screen_of_coord(test.x, test.y), test.expected_screen);
    }
}

SUITE(CrtMemberSuite)
{
    TEST(OnlyOneCrt)
    {
        /**
         * Ensures that a single-screen configuration has only the root
         * screen defined.
         */
        CrtManager mgr;

        std::vector<Box> screens;
        screens.push_back(Box(0, 0, 1366, 768));

        mgr.rebuild_graph(screens);

        // Ensure that:
        //
        // 1. There is one Crt, the root.
        // 2. It has the Box we expect.
        // 3. Some coords on the screen go back to the root screen.
        // 4. Some coords off the screen go back to static_cast<Crt*>(0) (no screen).
        Crt *root = mgr.root();

        CHECK(root != static_cast<Crt*>(0));
        CHECK_EQUAL(root->left, static_cast<Crt*>(0));
        CHECK_EQUAL(root->right, static_cast<Crt*>(0));
        CHECK_EQUAL(root->top, static_cast<Crt*>(0));
        CHECK_EQUAL(root->bottom, static_cast<Crt*>(0));

        CHECK_EQUAL(screens[0], mgr.box_of_screen(root));

        CHECK_EQUAL(root, mgr.screen_of_box(screens[0]));

        CrtOfCoordTest tests[] = {
            // Check 3 types of inside points:
            // 1. The left edge.
            // 2. The top edge.
            // (Omit the right and bottom since those are the next monitor over)
            // 3. The inside.
            {0, 500, root},
            {1000, 0, root},
            {1000, 500, root},

            // Check 4 types of outside points:
            // 1. The right edge.
            // 2. The bottom edge.
            // 3. Out in the right area.
            // 4. Out in the bottom area.
            {1366, 0, static_cast<Crt*>(0)},
            {0, 768, static_cast<Crt*>(0)},
            {1500, 500, static_cast<Crt*>(0)},
            {500, 1500, static_cast<Crt*>(0)},
        };
        test_screen_expectations(mgr, tests, 7);
    }

    TEST(SingleCrtsToRightAndBottom)
    {
        /**
         * Ensure that screens other than the root are detected
         */
        CrtManager mgr;

        std::vector<Box> screens;

        /*
         * +----------+---------+
         * | 1500x500 | 500x500 |
         * |          |         |
         * +----------+---------+
         * | 1500x500 |
         * |          |
         * +----------+
         */
        screens.push_back(Box(0, 0, 1500, 500));
        screens.push_back(Box(1500, 0, 500, 500));
        screens.push_back(Box(0, 500, 1500, 500));

        mgr.rebuild_graph(screens);

        // The root should have a bottom and right peer
        Crt *root = mgr.root();
        Crt *right = root->right;
        Crt *bottom = root->bottom;

        // Check that the boundaries don't have anything beyond them
        CHECK_EQUAL(root->top, static_cast<Crt*>(0));
        CHECK_EQUAL(root->left, static_cast<Crt*>(0));
        CHECK_EQUAL(right->top, static_cast<Crt*>(0));
        CHECK_EQUAL(right->right, static_cast<Crt*>(0));
        CHECK_EQUAL(right->bottom, static_cast<Crt*>(0));
        CHECK_EQUAL(bottom->left, static_cast<Crt*>(0));
        CHECK_EQUAL(bottom->bottom, static_cast<Crt*>(0));
        CHECK_EQUAL(bottom->right, static_cast<Crt*>(0));

        // Checks the interlinks between the screens
        CHECK_EQUAL(bottom->top, root);
        CHECK_EQUAL(right->left, root);

        // Now check the screen boundaries
        CHECK_EQUAL(mgr.box_of_screen(root), screens[0]);
        CHECK_EQUAL(mgr.box_of_screen(right), screens[1]);
        CHECK_EQUAL(mgr.box_of_screen(bottom), screens[2]);

        CHECK_EQUAL(mgr.screen_of_box(screens[0]), root);
        CHECK_EQUAL(mgr.screen_of_box(screens[1]), right);
        CHECK_EQUAL(mgr.screen_of_box(screens[2]), bottom);

        CrtOfCoordTest tests[] = {
            // Finally, check the following coordinates and ensure that they
            // are on their right places:
            // 1. Far-left edge -> Root screen
            // 2. Far-top edge -> Root screen
            // 3. Vertical middle (between root and right) -> Right screen
            // 4. Horizontal middle (between root and bottom) -> Bottom screen
            // 5. Internal of the root screen
            // 6. Internal of the right screen
            // 7. Internal of the bottom screen
            {0, 250, root},
            {500, 0, root},
            {1500, 250, right},
            {1000, 500, bottom},
            {1000, 250, root},
            {1750, 250, right},
            {750, 750, bottom},
            
            // Test a few external points as well
            // 1. Below the bottom
            // 2. Right of the right
            // 3. Right of the bottom and below the right
            {500, 1500, static_cast<Crt*>(0)},
            {2500, 250, static_cast<Crt*>(0)},
            {1750, 750, static_cast<Crt*>(0)},
        };
        test_screen_expectations(mgr, tests, 10);
    }
}

int main()
{
    return UnitTest::RunAllTests();
}
