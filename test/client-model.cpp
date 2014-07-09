#include <algorithm>
#include <memory>
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
                dynamic_cast<const ChangeClientDesktop*>(iterator->get());
            std::shared_ptr<Desktop> desktop(new UserDesktop(0));
            CHECK_EQUAL(ChangeClientDesktop(a, desktop), *the_change);
        }
        iterator++;

        // Secondly, it is stacked relative to other windows
        {
            const ChangeLayer *the_change = dynamic_cast<const ChangeLayer*>(iterator->get());
            CHECK_EQUAL(ChangeLayer(a, DEF_LAYER), *the_change);
        }
        iterator++;

        // Finally, this is the end of the event stream
        CHECK_EQUAL(model.changes_end(), iterator);
        model.flush_changes();
    }
};

int main()
{
    return UnitTest::RunAllTests();
}
