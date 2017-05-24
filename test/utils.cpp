#include <cstring>

#include <UnitTest++.h>
#include "utils.h"

TEST(test_strip_string)
{
    char buffer[100];
    /*
     * This test data checks a number of things:
     *
     *  1. That data not part of the removal list is not removed.
     *  2. That data on the front, back and the internal of the text
     *     is removed.
     */
    strip_string("  \t\ntest text\t\n ", " \t\n", buffer);
    CHECK_EQUAL(std::strcmp(buffer, "testtext"), 0);
}

TEST(test_try_parse_ulong)
{
    // Try a positive number in the long range
    CHECK_EQUAL(try_parse_ulong("123456", 0), 123456);

    // Try a positive number in an alternate base
    CHECK_EQUAL(try_parse_ulong("0x123", 0), 0x123);

    // Make sure negatives return the default
    CHECK_EQUAL(try_parse_ulong("-123", 0), 0);

    // Make sure that garbage returns the default
    CHECK_EQUAL(try_parse_ulong("asdfjkl;", 0), 0);
}

TEST(test_try_parse_ulong_nonzero)
{
    // Try a positive number in the long range
    CHECK_EQUAL(try_parse_ulong_nonzero("123456", 0), 123456);

    // Try a positive number in an alternate base
    CHECK_EQUAL(try_parse_ulong_nonzero("0x123", 0), 0x123);

    // Make sure negatives return the default
    CHECK_EQUAL(try_parse_ulong_nonzero("-123", 0), 0);

    // Make sure that garbage returns the default
    CHECK_EQUAL(try_parse_ulong_nonzero("asdfjkl;", 0), 0);

    // Ensure that zero returns the default
    CHECK_EQUAL(try_parse_ulong_nonzero("0", 42), 42);
}

int main()
{
    return UnitTest::RunAllTests();
}
