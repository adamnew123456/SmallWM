#include <UnitTest++.h>
#include "model/unique-multimap.h"

bool found_in_category(UniqueMultimap<int, int> multimap, int value, int category)
{
    for (typename UniqueMultimap<int, int>::member_iter iter = multimap.get_members_of_begin(category);
        iter != multimap.get_members_of_end(category);
        ++iter)
    {
        if (*iter == value) return true;
    }

    return false;
}

struct UniqueMultimapFixture
{
    UniqueMultimap<int, int> multimap;

    UniqueMultimapFixture()
    {
        multimap.add_category(0);
        multimap.add_category(1);

        for (int i = 0; i <= 10; i++)
        {
            multimap.add_member(i % 2, i);
        }
    }
};

SUITE(UniqueMultimapTests)
{
    /**
     * Ensures that 0 and 1 are in the multimap, and other values are not.
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_is_category)
    {
        CHECK_EQUAL(multimap.is_category(0), true);
        CHECK_EQUAL(multimap.is_category(1), true);
        CHECK_EQUAL(multimap.is_category(42), false);
    }

    /**
     * Ensures that [0, 10] are all members, and other values are not.
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_is_member)
    {
        for (int i = 0; i <= 10; i++)
        {
            CHECK_EQUAL(multimap.is_member(i), true);
        }

        CHECK_EQUAL(multimap.is_member(42), false);
    }

    /**
     * Ensures that getting the category of a value returns the correct
     * category.
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_get_category_of)
    {
        for (int i = 0; i <= 10; i++)
        {
            CHECK_EQUAL(multimap.get_category_of(i), i % 2);
        }
    }

    /**
     * Ensures that getting the members of a category returns all the
     * members in that category.
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_get_members_of)
    {
        int member = 0;
        for (UniqueMultimap<int,int>::member_iter iter =
                multimap.get_members_of_begin(0);
            iter != multimap.get_members_of_end(0);
            ++iter, member += 2)
        {
            // member += 2 is because, by getting category 0, we're getting
            // all even values
            CHECK_EQUAL(*iter, member);
        }
    }

    /**
     * Ensures that counting the members of a category returns the correct
     * number of values.
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_count_members_of)
    {
        // 0, 2, 4, 6, 8 and 10
        CHECK_EQUAL(multimap.count_members_of(0), 6);
    }

    /**
     * Ensures that:
     *  - Adding a new member to an existent category succeeds
     *  - Adding a new member to a nonexistent category fails
     *  - Adding a non-new member to an existent category fails
     *  - Adding a non-new member to a nonexistent category fails
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_add_member)
    {
        // Case 1 - new member, existent category
        CHECK_EQUAL(multimap.add_member(0, 12), true);

        // Case 2 - new member, nonexistent category
        CHECK_EQUAL(multimap.add_member(3, 13), false);

        // Case 3 - non-new member, existent category
        CHECK_EQUAL(multimap.add_member(0, 8), false);

        // Case 4 - non-new member, nonexistent category
        CHECK_EQUAL(multimap.add_member(3, 8), false);
    }

    /**
     * This ensures that a new member shows up, using:
     *
     * - is_member
     * - get_category_of
     * - get_members_of
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_add_member_works)
    {
        CHECK_EQUAL(multimap.add_member(0, 12), true);

        CHECK_EQUAL(multimap.is_member(12), true);
        CHECK_EQUAL(multimap.get_category_of(12), 0);

        CHECK_EQUAL(found_in_category(multimap, 12, 0), true);
    }

    /**
     * This ensures that:
     *
     * - Moving an existent member to an existent category works
     * - Moving a nonexistent member to an existent category fails
     * - Moving an existent member to a nonexistent category fails
     * - Moving a nonexistent member to a nonexistent category fails
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_move)
    {
        // 13 is miscategorized as even on purpose
        CHECK_EQUAL(multimap.add_member(0, 13), true);

        // Case 1: Existent member, existent category
        CHECK_EQUAL(multimap.move_member(13, 1), true);

        // Case 2: Nonexistent member, existent category
        CHECK_EQUAL(multimap.move_member(14, 0), false);

        // Case 3: Existent member, nonexistent category
        CHECK_EQUAL(multimap.move_member(13, 3), false);

        // Case 4: Nonexistent member, nonexistent category
        CHECK_EQUAL(multimap.move_member(14, 3), false);
    }

    /**
     * This ensures that a moved member shows up in the right places,
     * including:
     *
     * - is_member
     * - get_category_of (returns the new)
     * - get_members_of (in the new, not in the old)
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_move_works)
    {
        // 6 is odd now. Deal with it.
        CHECK_EQUAL(multimap.move_member(6, 1), true);

        CHECK_EQUAL(multimap.is_member(6), true);
        CHECK_EQUAL(multimap.get_category_of(6), 1);

        bool found_six_in_new = found_in_category(multimap, 6, 1);
        bool found_six_in_old = found_in_category(multimap, 6, 0);
        CHECK_EQUAL(found_six_in_new && !found_six_in_old, true);
    }

    /**
     * Ensure that:
     *
     * - Removing an existent element works.
     * - Removing a nonexistent element fails.
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_remove)
    {
        CHECK_EQUAL(multimap.add_member(0, -2), true);

        // Case 1: Existent member
        CHECK_EQUAL(multimap.remove_member(-2), true);

        // Case 2: Nonexistent member
        CHECK_EQUAL(multimap.remove_member(14), false);
    }

    /**
     * Ensure that a removed member doesn't show up where it used to:
     *
     * - is_member
     * - get_members_of
     *
     * We can't test get_category_of, since it is undefined for nonexistent
     * values.
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_remove_works)
    {
        // 6 is not a number now. Deal with it.
        CHECK_EQUAL(multimap.remove_member(6), true);

        CHECK_EQUAL(multimap.is_member(6), false);
        CHECK_EQUAL(found_in_category(multimap, 6, 0), false);
    }

    /**
     * Ensures that:
     *
     * - Adding an existent category fails
     * - Adding a nonexistent category succeeds.
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_add_category)
    {
        // Case 1: Existent category
        CHECK_EQUAL(multimap.add_category(0), false);

        // Case 2: Nonexistent category
        CHECK_EQUAL(multimap.add_category(3), true);
    }

    /**
     * Ensures that adding a category actually works, by adding elements to it
     * and moving elements to it, and then ensuring that they show up in the
     * typical ways:
     *
     * - get_category_of
     * - get_members_of
     */
    TEST_FIXTURE(UniqueMultimapFixture, test_add_category_works)
    {
        CHECK_EQUAL(multimap.add_category(3), true);

        CHECK_EQUAL(multimap.add_member(3, 15), true);
        CHECK_EQUAL(multimap.move_member(9, 3), true);

        CHECK_EQUAL(multimap.get_category_of(15), 3);
        CHECK_EQUAL(multimap.get_category_of(9), 3);

        CHECK_EQUAL(found_in_category(multimap, 15, 3), true);
        CHECK_EQUAL(found_in_category(multimap, 9, 3), true);
    }
}

int main()
{
    return UnitTest::RunAllTests();
}
