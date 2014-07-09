#include <algorithm>
#include <utility>

#include <UnitTest++.h>
#include "client-model.h"

const Window a = 1,
      b = 2,
      c = 3;

const unsigned long long max_desktops = 5;

struct ClientModelFixture
{
    ClientModelFixture() :
        model(max_desktops)
    {};

    ~ClientModelFixture()
    {};

    ClientModel model;
};

SUITE(ClientModelMemberSuite)
{
    TEST_FIXTURE(ClientModelFixture, test_default_members)
    {
        // Make sure that there are no clients by default
        CHECK_EQUAL(false, model.is_client(a));
        CHECK_EQUAL(false, model.is_client(b));

        // Add a new client, and ensure that it is present
        model.add_client(a, IS_VISIBLE, Dimension2D(1, 1), Dimension2D(1, 1));

        // Make sure that a is now listed as a client
        CHECK_EQUAL(true, model.is_client(a));

        // Check the event stream for the most recent events
        ClientModel::change_iter iterator = model.changes_begin();

        // First, the window appears on a desktop
        CHECK((*iterator)->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(*iterator);
            const Desktop *desktop(new UserDesktop(0));
            CHECK_EQUAL(ChangeClientDesktop(a, desktop), *the_change);
        }
        iterator++;

        // Secondly, it is stacked relative to other windows
        {
            const ChangeLayer *the_change = dynamic_cast<const ChangeLayer*>(*iterator);
            CHECK_EQUAL(ChangeLayer(a, DEF_LAYER), *the_change);
        }
        iterator++;

        // Make sure it was focused
        {
            const ChangeFocus *the_change = dynamic_cast<const ChangeFocus*>(*iterator);
            CHECK_EQUAL(ChangeFocus(None, a), *the_change);
        }
        iterator++;

        // Finally, this is the end of the event stream
        CHECK_EQUAL(model.changes_end(), iterator);
        model.flush_changes();

        // Then, remove the added client
        model.remove_client(a);
        CHECK_EQUAL(false, model.is_client(a));
    }

    TEST_FIXTURE(ClientModelFixture, test_visibility)
    {
        // Add a new client and ensure that it is present
        model.add_client(a, IS_VISIBLE, Dimension2D(1, 1), Dimension2D(1, 1));

        // Make sure that the client is visible by default
        CHECK(model.is_visible(a));

        // Make sure moving clients are invisible
        model.start_moving(a);
        CHECK(!model.is_visible(a));
        model.stop_moving(a, Dimension2D(2, 2));
        CHECK(model.is_visible(a));

        // Make sure resizing clients are invisible
        model.start_resizing(a);
        CHECK(!model.is_visible(a));
        model.stop_resizing(a, Dimension2D(2, 2));
        CHECK(model.is_visible(a));

        // Make sure that iconified clients are invisible
        model.iconify(a);
        CHECK(!model.is_visible(a));
        model.deiconify(a);
        CHECK(model.is_visible(a));

        // Move a client to a different desktop and make sure it is invisible
        model.client_next_desktop(a);
        CHECK(!model.is_visible(a));
        model.client_prev_desktop(a);
        CHECK(model.is_visible(a));

        model.client_prev_desktop(a);
        CHECK(!model.is_visible(a));
        model.client_next_desktop(a);
        CHECK(model.is_visible(a));

        // View a different desktop and make sure the client is invisible
        model.next_desktop();
        CHECK(!model.is_visible(a));
        model.prev_desktop();
        CHECK(model.is_visible(a));

        model.prev_desktop();
        CHECK(!model.is_visible(a));
        model.next_desktop();
        CHECK(model.is_visible(a));

        // Stick a window, and then change desktops, making sure the stuck
        // window is still visible
        model.toggle_stick(a);

        model.next_desktop();
        CHECK(model.is_visible(a));
        model.prev_desktop();
        CHECK(model.is_visible(a));

        model.prev_desktop();
        CHECK(model.is_visible(a));
        model.next_desktop();
        CHECK(model.is_visible(a));

        // Remove the stickiness and then make sure that the tests display the
        // same results as last time
        model.toggle_stick(a);

        model.next_desktop();
        CHECK(!model.is_visible(a));
        model.prev_desktop();
        CHECK(model.is_visible(a));

        model.prev_desktop();
        CHECK(!model.is_visible(a));
        model.next_desktop();
        CHECK(model.is_visible(a));
    }

    TEST_FIXTURE(ClientModelFixture, test_finder_functions)
    {
        // Make sure that the `find_*` functions return the correct results
        model.add_client(a, IS_VISIBLE, Dimension2D(1, 1), Dimension2D(1, 1));

        const Desktop *desktop_of = model.find_desktop(a);
        CHECK(*desktop_of == UserDesktop(0));
        CHECK(model.find_layer(a) == DEF_LAYER);
    }
};

int main()
{
    return UnitTest::RunAllTests();
}
