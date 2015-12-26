#include <UnitTest++.h>
#include "model/changes.h"

Window win = 1;

SUITE(ChangeStreamSuite)
{
    TEST(IsEmptyByDefault)
    {
        ChangeStream stream;

        CHECK(!stream.has_more());
        CHECK_EQUAL(stream.get_next(), static_cast<ChangeStream::change_ptr>(0));
    }

    TEST(PushedElementsCanBeRead)
    {
        ChangeStream stream;
        ChangeSize const *change = new ChangeSize(win, 42, 42);

        stream.push(change);

        CHECK(stream.has_more());
        CHECK_EQUAL(stream.get_next(), change);

        CHECK(!stream.has_more());
        CHECK_EQUAL(stream.get_next(), static_cast<ChangeStream::change_ptr>(0));
    }

    TEST(MultipleElementsRetainOrder)
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

    TEST(FlushClearsElements)
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
