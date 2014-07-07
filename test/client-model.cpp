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
    }
};

int main()
{
    return UnitTest::RunAllTests();
}
