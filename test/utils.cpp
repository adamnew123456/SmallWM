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

TEST(test_UniqueStack)
{
    // Ensure that UniqueStack acts in a LIFO manner, as long as no duplicates
    // are used
    UniqueStack<char> stack;
    CHECK(stack.empty());
    CHECK_EQUAL(stack.size(), 0);

    stack.push('a');
    stack.push('b');
    stack.push('c');

    CHECK(!stack.empty());
    CHECK_EQUAL(stack.size(), 3);

    CHECK_EQUAL(stack.top(), 'c');
    stack.pop();
    CHECK_EQUAL(stack.top(), 'b');
    stack.pop();
    CHECK_EQUAL(stack.top(), 'a');
    stack.pop();

    CHECK(stack.empty());
    CHECK_EQUAL(stack.size(), 0);

    // Now, try with a duplicate value, and ensure that the duplicate is
    // 'promoted' and not stored
    stack.push('a');
    stack.push('b');
    stack.push('a');

    CHECK(!stack.empty());
    CHECK_EQUAL(stack.size(), 2);

    CHECK_EQUAL(stack.top(), 'a');
    stack.pop();
    CHECK_EQUAL(stack.top(), 'b');
    stack.pop();

    CHECK(stack.empty());
    CHECK_EQUAL(stack.size(), 0);

    // Ensure that a values can be removed
    stack.push('a');
    stack.push('b');
    stack.push('c');

    CHECK(!stack.empty());
    CHECK_EQUAL(stack.size(), 3);

    CHECK_EQUAL(stack.remove('c'), true);

    CHECK(!stack.empty());
    CHECK_EQUAL(stack.size(), 2);

    CHECK_EQUAL(stack.top(), 'b');
    stack.pop();
    CHECK_EQUAL(stack.top(), 'a');
    stack.pop();

    CHECK(stack.empty());
    CHECK_EQUAL(stack.size(), 0);

    // Ensure that duplicate removals don't do anything, and removals of
    // non-existent items don't either
    stack.push('a');
    CHECK_EQUAL(stack.top(), 'a');

    CHECK_EQUAL(stack.remove('a'), true);
    CHECK_EQUAL(stack.remove('a'), false);

    CHECK_EQUAL(stack.remove('X'), false);

    CHECK(stack.empty());
    CHECK_EQUAL(stack.size(), 0);
}

int main()
{
    return UnitTest::RunAllTests();
}
