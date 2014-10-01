#include <algorithm>

#include <UnitTest++.h>
#include "model/x-model.h"

const Window the_icon = 1,
      the_placeholder = 2,
      the_client = 3;

struct XModelFixture
{
    XModel model;
};

#define NULL_OF(type) static_cast<type*>(0)

SUITE(XModelMemberSuite)
{
    TEST_FIXTURE(XModelFixture, test_icon_getters_with_no_icon)
    {
        // Ensure that the getters related to icons return expected results
        // when no icon is registered
        CHECK_EQUAL(model.find_icon_from_client(the_client), NULL_OF(Icon));
        CHECK_EQUAL(model.find_icon_from_icon_window(the_icon), NULL_OF(Icon));
    }

    TEST_FIXTURE(XModelFixture, test_icon_getters_with_icon)
    {
        // Ensure that using the getters on a registered icon return expected
        // results
        Icon *icon_data = new Icon(the_client, the_icon, NULL_OF(XGC));
        model.register_icon(icon_data);

        CHECK_EQUAL(model.find_icon_from_client(the_client), icon_data);
        CHECK_EQUAL(model.find_icon_from_icon_window(the_icon), icon_data);

        std::vector<Icon*> icons;
        model.get_icons(icons);
        CHECK_EQUAL(icons.size(), 1);
        CHECK_EQUAL(icons[0], icon_data);

        delete icon_data;
    }

    TEST_FIXTURE(XModelFixture, test_icon_list_with_multiple_icons)
    {
        Icon *icon_data = new Icon(the_client, the_icon, NULL_OF(XGC));
        model.register_icon(icon_data);

        const Window the_other_client = 4,
              the_other_icon = 5;
        Icon *icon_data2 = new Icon(the_other_client, the_other_icon, NULL_OF(XGC));
        model.register_icon(icon_data2);

        std::vector<Icon*> icons;
        model.get_icons(icons);
        CHECK_EQUAL(icons.size(), 2);
        CHECK_EQUAL(*(std::find(icons.begin(), icons.end(), icon_data)),
                    icon_data);
        CHECK_EQUAL(*(std::find(icons.begin(), icons.end(), icon_data2)),
                    icon_data2);
    }

    TEST_FIXTURE(XModelFixture, test_icon_getters_with_removed_icon)
    {
        // First, add an icon and ensure that data from the icon is produced
        Icon *icon_data = new Icon(the_client, the_icon, NULL_OF(XGC));
        model.register_icon(icon_data);

        CHECK_EQUAL(model.find_icon_from_client(the_client), icon_data);
        CHECK_EQUAL(model.find_icon_from_icon_window(the_icon), icon_data);

        // Lastly, ensure that the unregistered icon provides the same results
        // as when no client was registered at all
        model.unregister_icon(icon_data);
        delete icon_data;
        CHECK_EQUAL(model.find_icon_from_client(the_client), NULL_OF(Icon));
        CHECK_EQUAL(model.find_icon_from_icon_window(the_icon), NULL_OF(Icon));

        // This is an addition to this test, which covers the fix made in
        // commit 8017aeb17fba. When an icon was removed, a NULL pointer
        // would be returned inside of the list produced by XModel::get_icons
        //
        // The preference is that XModel::get_icons *not* return NULL icons
        // as a part of its listing, and instead only return valid icons
        std::vector<Icon*> icons;
        model.get_icons(icons);
        CHECK_EQUAL(icons.size(), 0);
    }

    TEST_FIXTURE(XModelFixture, test_move_resize_getters_with_no_client)
    {
        // Ensure that using the getters related to move/resize information
        // returns the expected results when no client exists
        CHECK_EQUAL(model.get_move_resize_placeholder(), None);
        CHECK_EQUAL(model.get_move_resize_client(), None);
        CHECK_EQUAL(model.get_move_resize_state(), MR_INVALID);
    }

    TEST_FIXTURE(XModelFixture, test_move_resize_getters_with_moving_client)
    {
        // Ensure that the getters return the correct values when a cleint
        // is being moved
        model.enter_move(the_client, the_placeholder, Dimension2D(0, 0));
        CHECK_EQUAL(model.get_move_resize_placeholder(), the_placeholder);
        CHECK_EQUAL(model.get_move_resize_client(), the_client);
        CHECK_EQUAL(model.get_move_resize_state(), MR_MOVE);
    }

    TEST_FIXTURE(XModelFixture, test_move_resize_getters_after_move)
    {
        // First, move a client and then check the values
        model.enter_move(the_client, the_placeholder, Dimension2D(0, 0));

        // Stop moving a client and ensure that the values are as they were 
        // originally
        model.exit_move_resize();
        CHECK_EQUAL(model.get_move_resize_placeholder(), None);
        CHECK_EQUAL(model.get_move_resize_client(), None);
        CHECK_EQUAL(model.get_move_resize_state(), MR_INVALID);
    }

    TEST_FIXTURE(XModelFixture, test_move_resize_getters_with_resizing_client)
    {
        // Ensure that the getters return the correct values when a cleint
        // is being moved
        model.enter_resize(the_client, the_placeholder, Dimension2D(0, 0));
        CHECK_EQUAL(model.get_move_resize_placeholder(), the_placeholder);
        CHECK_EQUAL(model.get_move_resize_client(), the_client);
        CHECK_EQUAL(model.get_move_resize_state(), MR_RESIZE);
    }

    TEST_FIXTURE(XModelFixture, test_move_resize_getters_after_resize)
    {
        // First, move a client and then check the values
        model.enter_resize(the_client, the_placeholder, Dimension2D(0, 0));

        // Stop moving a client and ensure that the values are as they were 
        // originally
        model.exit_move_resize();
        CHECK_EQUAL(model.get_move_resize_placeholder(), None);
        CHECK_EQUAL(model.get_move_resize_client(), None);
        CHECK_EQUAL(model.get_move_resize_state(), MR_INVALID);
    }

    TEST_FIXTURE(XModelFixture, test_move_and_then_resize)
    {
        // Start by moving the client and testing the result
        model.enter_move(the_client, the_placeholder, Dimension2D(0, 0));

        // Then, try to resize and ensure that nothing changes
        model.enter_resize(the_client, the_placeholder, Dimension2D(0, 0));
        CHECK_EQUAL(model.get_move_resize_placeholder(), the_placeholder);
        CHECK_EQUAL(model.get_move_resize_client(), the_client);
        CHECK_EQUAL(model.get_move_resize_state(), MR_MOVE);
    }

    TEST_FIXTURE(XModelFixture, test_resize_and_then_move)
    {
        // Start by resizing the client and testing the result
        model.enter_resize(the_client, the_placeholder, Dimension2D(0, 0));

        // Then, try to move and ensure that nothing changes
        model.enter_move(the_client, the_placeholder, Dimension2D(0, 0));
        CHECK_EQUAL(model.get_move_resize_placeholder(), the_placeholder);
        CHECK_EQUAL(model.get_move_resize_client(), the_client);
        CHECK_EQUAL(model.get_move_resize_state(), MR_RESIZE);
    }

    TEST_FIXTURE(XModelFixture, test_move_pointer_updates)
    {
        // Start by moving the client and testing the result
        model.enter_move(the_client, the_placeholder, Dimension2D(0, 0));

        // Update the pointer, and ensure that the difference is correct
        Dimension2D diff;
        diff = model.update_pointer(42, 42);
        CHECK_EQUAL(DIM2D_X(diff), 42);
        CHECK_EQUAL(DIM2D_Y(diff), 42);

        // Make another move, and ensure that the relative difference is again
        // correct
        diff = model.update_pointer(0, 84); // (0, 84) - (42, 42) = (-42, 42)
        CHECK_EQUAL(DIM2D_X(diff), -42);
        CHECK_EQUAL(DIM2D_Y(diff), 42);

        model.exit_move_resize();
    }

    TEST_FIXTURE(XModelFixture, test_resize_pointer_updates)
    {
        // Start by moving the client and testing the result
        model.enter_resize(the_client, the_placeholder, Dimension2D(0, 0));

        // Update the pointer, and ensure that the difference is correct
        Dimension2D diff;
        diff = model.update_pointer(42, 42);
        CHECK_EQUAL(DIM2D_X(diff), 42);
        CHECK_EQUAL(DIM2D_Y(diff), 42);

        // Make another move, and ensure that the relative difference is again
        // correct
        diff = model.update_pointer(0, 84); // (0, 84) - (42, 42) = (-42, 42)
        CHECK_EQUAL(DIM2D_X(diff), -42);
        CHECK_EQUAL(DIM2D_Y(diff), 42);

        model.exit_move_resize();
    }
};

int main()
{
    return UnitTest::RunAllTests();
}
