#include <UnitTest++.h>
#include "model/changes.h"

Window win = 1;

SUITE(ChangeStreamSuite)
{
    TEST(test_is_empty_by_default)
    {
        ChangeStream stream;

        CHECK(!stream.has_more());
        CHECK_EQUAL(stream.get_next(), static_cast<ChangeStream::change_ptr>(0));
    }

    TEST(test_pushed_elems_can_be_read)
    {
        ChangeStream stream;
        ChangeSize const *change = new ChangeSize(win, 42, 42);

        stream.push(change);

        CHECK(stream.has_more());
        CHECK_EQUAL(stream.get_next(), change);

        CHECK(!stream.has_more());
        CHECK_EQUAL(stream.get_next(), static_cast<ChangeStream::change_ptr>(0));
    }

    TEST(test_multiple_elems_retain_order)
    {
        ChangeStream stream;
        ChangeSize const *change1 = new ChangeSize(win, 42, 42);
        ChangeSize const *change2 = new ChangeSize(win, 21, 21);

        stream.push(change1);
        stream.push(change2);

        CHECK(stream.has_more());
        CHECK_EQUAL(stream.get_next(), change1);

        CHECK(stream.has_more());
        CHECK_EQUAL(stream.get_next(), change2);

        CHECK(!stream.has_more());
        CHECK_EQUAL(stream.get_next(), static_cast<ChangeStream::change_ptr>(0));
    }

    TEST(test_flush_clears_elems)
    {
        ChangeStream stream;
        ChangeSize const *change1 = new ChangeSize(win, 42, 42);
        ChangeSize const *change2 = new ChangeSize(win, 21, 21);

        stream.push(change1);
        stream.push(change2);
        
        stream.flush();

        CHECK(!stream.has_more());
        CHECK_EQUAL(stream.get_next(), static_cast<ChangeStream::change_ptr>(0));
    }
}

int main()
{
    return UnitTest::RunAllTests();
}
