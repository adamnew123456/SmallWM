/** @file */
#ifndef __SMALLWM_UNIQUE_MULTIMAP__
#define __SMALLWM_UNIQUE_MULTIMAP__

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

/**
 * Think of a 2-layer tree:
 *
 * @code
 *                  *
 *        __________|___________
 *       /       |      |       \
 *      C1      C2      C3      C4
 *    __|___    |      / \
 *   /  |   \   |     /   \
 *  M1  M2  M3  M4   M5   M6 
 * @endcode
 *
 * This two-level tree has the following structure:
 *
 *  - * is the root, which is the UniqueMultimap itself.
 *  - Cn is a category. Categories make up the second level of the tree, and
 *    must be unique across the whole of UniqueMultimap. Categories can
 *    _only_ appear at this level, and this whole level is made up exclusively
 *    of categories.
 *  - Mn is a member. Members make up the third level of a tree, and must
 *    belong to one category - no more and no less. Members can _only_ appear
 *    on this level, and this whole level is made up exclusively of members.
 *    Members are unique to the UniqueMultimap - that is, no member
 *    can belong to more than one category.
 */
template <typename category_t, typename member_t, typename category_comparator_t = std::less<category_t>>
class UniqueMultimap
{
public:
    typedef typename std::vector<member_t>::const_iterator member_iter;

    /**
     * Returns whether or not a category value has a category in this object.
     */
    bool is_category(category_t const &category)
    {
        return m_category_members.count(category) == 1;
    }

    /**
     * Returns whether or not a member is in this object.
     */
    bool is_member(member_t const &member)
    {
        return m_member_to_category.count(member) == 1;
    }

    /**
     * Adds a new category with no elements.
     *
     * @param[in] category The category to insert.
     * @return Whether (true) or not (false) the insertion was successful.
     */
    bool add_category(category_t const &category)
    {
        // Avoid including duplicates
        if (is_category(category))
            return false;
        
        m_category_members[category] = new std::vector<member_t>();
        return true;
    }

    /**
     * Gets the category of a particular element. Note that the return value
     * is undefined if the element does not exist.
     * @param[in] element The element to find the category of.
     * @return The category.
     */
    category_t &get_category_of(member_t const &element)
    {
        return m_member_to_category[element];
    }

    /**
     * The starting iterator for the elements of a category.
     * @param[in] category The category to get the elements of.
     */
    member_iter get_members_of_begin(category_t const &category)
    {
        return m_category_members[category]->begin();
    }

    /**
     * The starting iterator for the elements of a category.
     * @param[in] category The category to get the elements of.
     */
    member_iter get_members_of_end(category_t const &category)
    {
        return m_category_members[category]->end();
    }

    /**
     * Gets the number of elements of a particular category.
     * @param[in] category The category to get the number of elements of.
     * @return The number of elements in a category.
     */
    size_t count_members_of(category_t const &category)
    {
        return m_category_members[category]->size();
    }

    /**
     * Adds a new element to an existing category.
     * @param[in] category The category of the new element.
     * @param[in] element The new element to add.
     * @return Whether (true) or not (false) the addition was successful.
     */
    bool add_member(category_t const &category, member_t const &member)
    {
        if (is_member(member) || !is_category(category))
            return false;

        m_category_members[category]->push_back(member);
        m_member_to_category[member] = category;
        return true;
    }

    /**
     * Moves an element from one category to another.
     * @param[in] element The element to move.
     * @param[in] new_category The category to move the element to.
     * @return Whether (true) or not (false) the move was successful.
     */
    bool move_member(member_t const &member, category_t const &new_category)
    {
        if (!is_member(member) || !is_category(new_category))
            return false;

        remove_member(member);
        add_member(new_category, member);
        return true;
    }

    /**
     * Removes an element from its current category.
     * @param[in] element The element to remove.
     * @return Whether (true) or not (false) the removal was successful.
     */
    bool remove_member(member_t const &member)
    {
        if (!is_member(member))
            return false;

        category_t const &old_category = m_member_to_category[member];
        typename std::vector<member_t>::iterator member_location = std::find(
            m_category_members[old_category]->begin(),
            m_category_members[old_category]->end(),
            member
        );

        m_category_members[old_category]->erase(member_location);
        m_member_to_category.erase(member);
        return true;
    }
private:
    /// The 'top-down' mapping from categories to their members
    std::map<category_t, std::vector<member_t>*, category_comparator_t> m_category_members;
    /// The 'bottom-up' mapping from members to their categories
    std::map<member_t, category_t> m_member_to_category;
};

/**
 * Sorts according to the categories of values contained inside of a
 * UniqueMultimap.
 */
template <typename category_t, typename member_t>
class UniqueMultimapSorter
{
public:
    UniqueMultimapSorter(UniqueMultimap<category_t, member_t> &data) :
        m_uniquemultimap(data)
    {};

    /**
     * Returns if A < B.
     */
    bool operator()(member_t const &a, member_t const &b)
    {
        const category_t &a_category = m_uniquemultimap.get_category_of(a);
        const category_t &b_category = m_uniquemultimap.get_category_of(b);
        return a_category < b_category;
    }

private:
    UniqueMultimap<category_t, member_t> m_uniquemultimap;
};

#endif
