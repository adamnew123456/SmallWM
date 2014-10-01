#include <algorithm>
#include <utility>

#include <UnitTest++.h>
#include "model/client-model.h"

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

        // Make sure that a is now focused
        CHECK_EQUAL(a, model.get_focused());

        // Check the event stream for the most recent events
        const Change * change = model.get_next_change();

        // First, the window appears on a desktop
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            const Desktop *desktop(new UserDesktop(0));
            CHECK_EQUAL(ChangeClientDesktop(a, 0, desktop), *the_change);
        }
        delete change;

        // Secondly, it is stacked relative to other windows
        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_layer_change());
        {
            const ChangeLayer *the_change = dynamic_cast<const ChangeLayer*>(change);
            CHECK_EQUAL(ChangeLayer(a, DEF_LAYER), *the_change);
        }
        delete change;

        // Make sure it was focused
        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change = dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(None, a), *the_change);
        }
        delete change;

        // Finally, this is the end of the event stream
        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Then, remove the added client. Ensure that a 'ChangeFocus' event was
        // fired which includes the now-destroyed client.
        model.remove_client(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change = dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(a, None), *the_change);

            // Since ChangeFocus was fired, ensure that the focus was updated correctly
            CHECK_EQUAL(None, model.get_focused());
        }
        delete change;

        // Also ensure that a DestroyChange event was sent, to notify of the removed client
        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_destroy_change());
        {
            const DestroyChange *the_change = dynamic_cast<const DestroyChange*>(change);
            const Desktop *desktop(new UserDesktop(0));
            CHECK_EQUAL(DestroyChange(a, desktop, DEF_LAYER), *the_change);
        }

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        CHECK_EQUAL(false, model.is_client(a));
    }

    TEST_FIXTURE(ClientModelFixture, test_change_dropping)
    {
        // Make sure that there are no clients by default
        CHECK_EQUAL(false, model.is_client(a));
        CHECK_EQUAL(false, model.is_client(b));

        const Change * change;

        model.begin_dropping_changes();
        {
            // Add a new client, and ensure that it is present
            model.add_client(a, IS_VISIBLE, Dimension2D(1, 1), Dimension2D(1, 1));

            // Ensure that no events have passed by since we're dropping changes
            change = model.get_next_change();
            CHECK_EQUAL(change, static_cast<const Change *>(0));
        }
        model.end_dropping_changes();

        // Then, remove the added client. Ensure that a 'ChangeFocus' event was
        // fired which includes the now-destroyed client.
        model.remove_client(a);

        // Ensure that, now that we're no longer dropping changes, that a change
        // is fired
        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change = dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(a, None), *the_change);
        }
        delete change;

        // Also ensure that a DestroyChange event was sent, to notify of the removed client
        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_destroy_change());
        {
            const DestroyChange *the_change = dynamic_cast<const DestroyChange*>(change);
            const Desktop *desktop(new UserDesktop(0));
            CHECK_EQUAL(DestroyChange(a, desktop, DEF_LAYER), *the_change);
        }

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
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

    TEST_FIXTURE(ClientModelFixture, test_desktop_visibility)
    {
        // First, make sure that the first desktop is visible, and
        // no other desktops are
        CHECK(model.is_visible_desktop(model.USER_DESKTOPS[0]));
        CHECK(model.is_visible_desktop(model.ALL_DESKTOPS));
        
        for (unsigned long long desktop = 0; desktop < max_desktops;
             desktop++)
        {
            if (desktop == 0)
                continue;
            CHECK(!model.is_visible_desktop(model.USER_DESKTOPS[desktop]));
        }

        CHECK(!model.is_visible_desktop(model.ICON_DESKTOP));
        CHECK(!model.is_visible_desktop(model.MOVING_DESKTOP));
        CHECK(!model.is_visible_desktop(model.RESIZING_DESKTOP));

        // Now, move to the next desktop and ensure that the same is true
        // of all the visibility states but that USER_DESKTOPS[1] is visible
        // and USER_DESKTOPS[0] is not
        model.next_desktop();

        CHECK(model.is_visible_desktop(model.USER_DESKTOPS[1]));
        CHECK(model.is_visible_desktop(model.ALL_DESKTOPS));
        
        for (unsigned long long desktop = 0; desktop < max_desktops;
             desktop++)
        {
            if (desktop == 1)
                continue;
            CHECK(!model.is_visible_desktop(model.USER_DESKTOPS[desktop]));
        }

        CHECK(!model.is_visible_desktop(model.ICON_DESKTOP));
        CHECK(!model.is_visible_desktop(model.MOVING_DESKTOP));
        CHECK(!model.is_visible_desktop(model.RESIZING_DESKTOP));
    }

    TEST_FIXTURE(ClientModelFixture, test_finder_functions)
    {
        // Make sure that the `find_*` functions return the correct results
        model.add_client(a, IS_VISIBLE, Dimension2D(1, 1), Dimension2D(1, 1));

        const Desktop *desktop_of = model.find_desktop(a);
        CHECK(*desktop_of == UserDesktop(0));
        CHECK(model.find_layer(a) == DEF_LAYER);
    }

    TEST_FIXTURE(ClientModelFixture, test_getters)
    {
        // First, ensure that `get_clients_of` gets only clients on the given
        // desktop
        model.add_client(a, IS_VISIBLE, Dimension2D(1, 1), Dimension2D(1, 1));
        model.add_client(b, IS_VISIBLE, Dimension2D(1, 1), Dimension2D(1, 1));

        std::vector<Window> result;
        model.get_clients_of(model.USER_DESKTOPS[0], result);
        CHECK_EQUAL(2, result.size());
        if (result[0] == a)
            CHECK_EQUAL(b, result[1]);
        else if (result[0] == b)
            CHECK_EQUAL(a, result[1]);
        else
            CHECK(false);

        // Also, ensure that all clients are marked as visible
        result.clear();
        model.get_visible_clients(result);
        CHECK_EQUAL(2, result.size());
        if (result[0] == a)
            CHECK_EQUAL(b, result[1]);
        else if (result[0] == b)
            CHECK_EQUAL(a, result[1]);
        else
            CHECK(false);

        // Move a client down, and ensure that it appears before the other in
        // stacking order
        model.down_layer(b);
        result.clear();
        model.get_visible_in_layer_order(result);
        CHECK_EQUAL(2, result.size());
        CHECK_EQUAL(result[0], b);
        CHECK_EQUAL(result[1], a);

        // Now, move the client up and ensure that the layer order is reversed
        model.up_layer(b);
        model.up_layer(b);
        result.clear();
        model.get_visible_in_layer_order(result);
        CHECK_EQUAL(2, result.size());
        CHECK_EQUAL(result[0], a);
        CHECK_EQUAL(result[1], b);

        // Move a client off this desktop, and ensure that it appears there
        // Also, ensure that the visible list no longer includes it
        model.client_next_desktop(b);

        result.clear();
        model.get_clients_of(model.USER_DESKTOPS[0], result);
        CHECK_EQUAL(1, result.size());
        CHECK_EQUAL(a, result[0]);

        result.clear();
        model.get_clients_of(model.USER_DESKTOPS[1], result);
        CHECK_EQUAL(1, result.size());
        CHECK_EQUAL(b, result[0]);

        // Ensure that the visible list includes only the client on this
        // desktop; the same goes for the visible clients in layer order
        result.clear();
        model.get_visible_clients(result);
        CHECK_EQUAL(1, result.size());
        CHECK_EQUAL(a, result[0]);

        result.clear();
        model.get_visible_in_layer_order(result);
        CHECK_EQUAL(1, result.size());
        CHECK_EQUAL(a, result[0]);

        // Go to the next desktop and make sure that the visible list is
        // fixed
        model.next_desktop();

        result.clear();
        model.get_visible_clients(result);
        CHECK_EQUAL(1, result.size());
        CHECK_EQUAL(b, result[0]);

        result.clear();
        model.get_visible_in_layer_order(result);
        CHECK_EQUAL(1, result.size());
        CHECK_EQUAL(b, result[0]);
    }

    TEST_FIXTURE(ClientModelFixture, test_layer_change)
    {
        // Move a client up, and then down - ensure that, both times, the
        // proper event is sent.
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // Up
        model.up_layer(a);
        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_layer_change());
        {
            const ChangeLayer *the_change = 
                dynamic_cast<const ChangeLayer*>(change);
            CHECK_EQUAL(ChangeLayer(a, DEF_LAYER + 1), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Down
        model.down_layer(a);
        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_layer_change());
        {
            const ChangeLayer *the_change = 
                dynamic_cast<const ChangeLayer*>(change);
            CHECK_EQUAL(ChangeLayer(a, DEF_LAYER), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Set the layer
        model.set_layer(a, MIN_LAYER);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_layer_change());
        {
            const ChangeLayer *the_change = 
                dynamic_cast<const ChangeLayer*>(change);
            CHECK_EQUAL(ChangeLayer(a, MIN_LAYER), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Set the layer to the same layer, and ensure that no change is
        // fired
        model.set_layer(a, MIN_LAYER);
        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_layer_extremes)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));

        // First, put the client on the top
        model.set_layer(a, MIN_LAYER);
        model.flush_changes();

        // Then, try to move it up and ensure no changes occurred
        model.down_layer(a);
        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));

        // Put the client on the bottom and run the same test, backwards
        model.set_layer(a, MAX_LAYER);
        model.flush_changes();

        model.up_layer(a);
        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_client_desktop_change)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // First, move the client ahead and make sure it changes accordingly
        model.client_next_desktop(a);

        // The client should lose the focus, since it will not be visible soon
        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change =
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(a, None), *the_change);

            CHECK_EQUAL(None, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[0], model.USER_DESKTOPS[1]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Move the client behind and make sure it returns to its current position
        model.client_prev_desktop(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[1], model.USER_DESKTOPS[0]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Move the client back one more time and make sure that it wraps to
        // the last desktop
        model.client_prev_desktop(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[0], model.USER_DESKTOPS[max_desktops - 1]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Move the client ahead and make sure that it wraps to the first desktop
        model.client_next_desktop(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[max_desktops - 1], model.USER_DESKTOPS[0]), 
                *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Move the client, and then reset its desktop to the current one, and
        // ensure that we get the change
        model.client_next_desktop(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[0], model.USER_DESKTOPS[1]), 
                *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        model.client_reset_desktop(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[1], model.USER_DESKTOPS[0]), 
                *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // With the client's desktop reset, we shouldn't get any changes from
        // resetting the desktop again
        model.client_reset_desktop(a);

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_bad_client_desktop_change)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // First off, iconified clients cannot have their desktops changed
#define FLUSH_AFTER(expr) expr ; model.flush_changes()
        FLUSH_AFTER(model.iconify(a));
        model.client_next_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.deiconify(a));

        FLUSH_AFTER(model.iconify(a));
        model.client_prev_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.deiconify(a));

        FLUSH_AFTER(model.iconify(a));
        model.client_reset_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.deiconify(a));

        // Secondly, moving clients cannot be changed
        FLUSH_AFTER(model.start_moving(a));
        model.client_next_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(a, Dimension2D(1, 1)));

        FLUSH_AFTER(model.start_moving(a));
        model.client_prev_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(a, Dimension2D(1, 1)));

        FLUSH_AFTER(model.start_moving(a));
        model.client_reset_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(a, Dimension2D(1, 1)));

        // Neither can resizing clients
        FLUSH_AFTER(model.start_resizing(a));
        model.client_next_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(a, Dimension2D(1, 1)));

        FLUSH_AFTER(model.start_resizing(a));
        model.client_prev_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(a, Dimension2D(1, 1)));

        FLUSH_AFTER(model.start_resizing(a));
        model.client_reset_desktop(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(a, Dimension2D(1, 1)));

#undef FLUSH_AFTER
    }

    TEST_FIXTURE(ClientModelFixture, test_desktop_change)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // Move to the next desktop
        model.next_desktop();

        // The current should lose the focus, since it will not be visible soon
        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change =
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(a, None), *the_change);

            CHECK_EQUAL(None, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_current_desktop_change());
        {
            const ChangeCurrentDesktop *the_change = 
                dynamic_cast<const ChangeCurrentDesktop*>(change);
            CHECK_EQUAL(ChangeCurrentDesktop(model.USER_DESKTOPS[0], model.USER_DESKTOPS[1]), 
                *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Move the current behind and make sure it returns to its current position
        model.prev_desktop();

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_current_desktop_change());
        {
            const ChangeCurrentDesktop *the_change = 
                dynamic_cast<const ChangeCurrentDesktop*>(change);
            CHECK_EQUAL(ChangeCurrentDesktop(model.USER_DESKTOPS[1],
                model.USER_DESKTOPS[0]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Move the desktop back one more time and make sure that it wraps to
        // the last
        model.prev_desktop();

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_current_desktop_change());
        {
            const ChangeCurrentDesktop *the_change = 
                dynamic_cast<const ChangeCurrentDesktop*>(change);
            CHECK_EQUAL(ChangeCurrentDesktop(model.USER_DESKTOPS[0], 
                model.USER_DESKTOPS[max_desktops - 1]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Move the desktop ahead and make sure that it wraps to the first
        model.next_desktop();

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_current_desktop_change());
        {
            const ChangeCurrentDesktop *the_change = 
                dynamic_cast<const ChangeCurrentDesktop*>(change);
            CHECK_EQUAL(ChangeCurrentDesktop(model.USER_DESKTOPS[max_desktops - 1],
                model.USER_DESKTOPS[0]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_bad_desktop_change)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

#define FLUSH_AFTER(expr) expr ; model.flush_changes()
        // The desktop can't be changed while a window is moving
        FLUSH_AFTER(model.start_moving(a));
        model.next_desktop();

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(a, Dimension2D(1, 1)));

        FLUSH_AFTER(model.start_moving(a));
        model.prev_desktop();

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(a, Dimension2D(1, 1)));

        // The desktop can't be changed while a window is resizing
        FLUSH_AFTER(model.start_resizing(a));
        model.next_desktop();

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(a, Dimension2D(1, 1)));

        FLUSH_AFTER(model.start_resizing(a));
        model.prev_desktop();

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(a, Dimension2D(1, 1)));
#undef FLUSH_AFTER
    }

    TEST_FIXTURE(ClientModelFixture, test_stick_does_not_lose_focus)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // Ensure that a window which is stuck does not lose its focus when
        // it is moved around
        model.toggle_stick(a);
        model.flush_changes();

        model.next_desktop();

        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_current_desktop_change());
        {
            const ChangeCurrentDesktop *the_change = 
                dynamic_cast<const ChangeCurrentDesktop*>(change);
            CHECK_EQUAL(ChangeCurrentDesktop(model.USER_DESKTOPS[0], 
                model.USER_DESKTOPS[1]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Unstick it, and ensure that it was moved onto the current desktop.
        // This should not cause any focus changes.
        model.toggle_stick(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.ALL_DESKTOPS, model.USER_DESKTOPS[1]), 
                *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_iconify)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // First, iconify the client. Ensure that it loses the focus, and that
        // we get the notification about the desktop change.
        model.iconify(a);

        Change const * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change = 
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(a, None), 
                *the_change);

            CHECK_EQUAL(None, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[0], 
                model.ICON_DESKTOP), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Then, deiconify it - ensure that it lands on the current desktop,
        // and that it regains the focus once it is on the current desktop.
        model.deiconify(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.ICON_DESKTOP, 
                model.USER_DESKTOPS[0]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change = 
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(None, a), 
                *the_change);

            CHECK_EQUAL(a, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_bad_iconify)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

#define FLUSH_AFTER(expr) expr ; model.flush_changes()
        // A window which is not iconified, cannot be deiconified
        model.deiconify(a);
        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));

        // A window cannot be iconified while it is being moved
        FLUSH_AFTER(model.start_moving(a));
        model.iconify(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(a, Dimension2D(1, 1)));

        // A window cannot be iconified while it is being resized
        FLUSH_AFTER(model.start_resizing(a));
        model.iconify(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(a, Dimension2D(1, 1)));
#undef FLUSH_AFER
    }

    TEST_FIXTURE(ClientModelFixture, test_moving)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // Start moving the client - ensure that it is unfocused, and that it
        // its desktop changes
        model.start_moving(a);

        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change =
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(a, None), *the_change);

            CHECK_EQUAL(None, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change =
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[0], 
                model.MOVING_DESKTOP), *the_change);
        } 
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Stop moving the client, and ensure that we get a move event back after the client is restored.
        model.stop_moving(a, Dimension2D(42, 43));

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change =
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.MOVING_DESKTOP, 
                model.USER_DESKTOPS[0]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_location_change());
        {
            const ChangeLocation *the_change =
                dynamic_cast<const ChangeLocation*>(change);
            CHECK_EQUAL(ChangeLocation(a, 42, 43), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change =
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(None, a), *the_change);

            CHECK_EQUAL(a, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_bad_moving)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

#define FLUSH_AFTER(expr) expr ; model.flush_changes()
        // A window which is not moving, cannot cease moving
        model.stop_moving(a, Dimension2D(1, 1));
        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));

        // A window cannot be moved while it is iconified
        FLUSH_AFTER(model.iconify(a));
        model.start_moving(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.deiconify(a));

        // A window cannot be moved while it is being resized
        FLUSH_AFTER(model.start_resizing(a));
        model.start_moving(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(a, Dimension2D(1, 1)));

        // A window cannot be moved while *any* other window is being resized
        // or moved.
        model.add_client(b, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        FLUSH_AFTER(model.start_moving(b));
        model.start_moving(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(b, Dimension2D(1, 1)));

        FLUSH_AFTER(model.start_resizing(b));
        model.start_moving(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(b, Dimension2D(1, 1)));
        
#undef FLUSH_AFER
    }

    TEST_FIXTURE(ClientModelFixture, test_resizing)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // Start resizing the client - ensure that it is unfocused, and that 
        // it its desktop changes
        model.start_resizing(a);

        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change =
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(a, None), *the_change);

            CHECK_EQUAL(None, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change =
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[0], 
                model.RESIZING_DESKTOP), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Stop moving the client, and ensure that we get a move event back 
        // after the client is restored.
        model.stop_resizing(a, Dimension2D(42, 43));

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change =
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.RESIZING_DESKTOP, 
                model.USER_DESKTOPS[0]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_size_change());
        {
            const ChangeSize *the_change =
                dynamic_cast<const ChangeSize*>(change);
            CHECK_EQUAL(ChangeSize(a, 42, 43), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change =
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(None, a), *the_change);

            CHECK_EQUAL(a, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_bad_resizing)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

#define FLUSH_AFTER(expr) expr ; model.flush_changes()
        // A window which is not resizing, cannot cease resizing
        model.stop_resizing(a, Dimension2D(1, 1));
        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));

        // A window cannot be resized while it is iconified
        FLUSH_AFTER(model.iconify(a));
        model.start_resizing(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.deiconify(a));

        // A window cannot be resized while it is being moved
        FLUSH_AFTER(model.start_moving(a));
        model.start_resizing(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(a, Dimension2D(1, 1)));

        // A window cannot be moved while *any* other window is being resized
        // or moved.
        model.add_client(b, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        FLUSH_AFTER(model.start_moving(b));
        model.start_resizing(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_moving(b, Dimension2D(1, 1)));

        FLUSH_AFTER(model.start_resizing(b));
        model.start_resizing(a);

        CHECK_EQUAL(model.get_next_change(), static_cast<const Change*>(0));
        FLUSH_AFTER(model.stop_resizing(b, Dimension2D(1, 1)));

        // Unfocus whatever is currently focused, so that it doesn't taint
        // the ChangeFocus event in the next test
        FLUSH_AFTER(model.unfocus());

        // When resizing, giving an invalid size should restore the window's
        // desktop and focus, but should *not* trigger a ChangeSize event
        FLUSH_AFTER(model.start_resizing(a));

        model.stop_resizing(a, Dimension2D(0, 0));

        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change =
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.RESIZING_DESKTOP, 
                model.USER_DESKTOPS[0]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change =
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(None, a), *the_change);

            CHECK_EQUAL(a, model.get_focused());
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
#undef FLUSH_AFER
    }

    TEST_FIXTURE(ClientModelFixture, test_toggle_stick)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // First, stick a client and ensure that we get the desktop change event
        model.toggle_stick(a);

        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.USER_DESKTOPS[0], 
                model.ALL_DESKTOPS), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Then, unstick it and ensure that we get the change event again
        model.toggle_stick(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_client_desktop_change());
        {
            const ChangeClientDesktop *the_change = 
                dynamic_cast<const ChangeClientDesktop*>(change);
            CHECK_EQUAL(ChangeClientDesktop(a, model.ALL_DESKTOPS, 
                model.USER_DESKTOPS[0]), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_focus_unfocus)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // First, ensure that a is focused
        CHECK_EQUAL(a, model.get_focused());

        // Then, unfocus a and check for the event
        model.unfocus();
        
        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change = 
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(a, None), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Refocus and ensure that another event is fired
        model.focus(a);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_focus_change());
        {
            const ChangeFocus *the_change = 
                dynamic_cast<const ChangeFocus*>(change);
            CHECK_EQUAL(ChangeFocus(None, a), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }

    TEST_FIXTURE(ClientModelFixture, test_location_size_changers)
    {
        model.add_client(a, IS_VISIBLE, 
            Dimension2D(1, 1), Dimension2D(1, 1));
        model.flush_changes();

        // First, move the window manually and ensure a change happens
        model.change_location(a, 100, 100);

        const Change * change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_location_change());
        {
            const ChangeLocation *the_change =
                dynamic_cast<const ChangeLocation*>(change);
            CHECK_EQUAL(ChangeLocation(a, 100, 100), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Then, cause a resize and check for the event
        model.change_size(a, 100, 100);

        change = model.get_next_change();
        CHECK(change != 0);
        CHECK(change->is_size_change());
        {
            const ChangeSize *the_change =
                dynamic_cast<const ChangeSize*>(change);
            CHECK_EQUAL(ChangeSize(a, 100, 100), *the_change);
        }
        delete change;

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));

        // Finally, try to use an invalid size, and ensure that no change is
        // propagated
        model.change_size(a, -1, -1);

        change = model.get_next_change();
        CHECK_EQUAL(change, static_cast<const Change *>(0));
    }
};

int main()
{
    return UnitTest::RunAllTests();
}
