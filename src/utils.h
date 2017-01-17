/** @file */
#ifndef __SMALLWM_UTILS__
#define __SMALLWM_UTILS__

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>

unsigned long try_parse_ulong(const char *string, unsigned long default_);
unsigned long try_parse_ulong_nonzero(const char *string, unsigned long default_);
void strip_string(const char *text, const char *remove, char *buffer);

template<class InputIt, class T>
bool contains(InputIt start, InputIt end, T& value)
{
    InputIt result = std::find(start, end, value);
    return result != end;
}

/**
 * A sorter that uses the elements being sorted as keys to a map, which are
 * sorted by their values in the map.
 */
template <class Key, class Value>
class MapSorter
{
public:
    MapSorter(std::map<Key, Value> map): m_map(map) {}

    bool operator()(const Key &a, const Key &b)
    {
        return m_map[a] < m_map[b];
    }

private:
    std::map<Key, Value> m_map;
};

/**
 * This appears similar to a stack, but it works slightly differently, the
 * difference being that an element can only exist in the stack once - if
 * the same element is put on the stack twice, then only one copy will end
 * up (the element farthest down the stack - that is, the one that would be
 * popped *last* - is removed in favor of the one that is closer to the top).
 *
 * The interface is essentially the same as a std::stack, but without the
 * comparison operators. However, it is only usable with value types.
 *
 * For example, consider the following:
 *
 *   x = UniqueStack<char>();
 *   x.empty() == true;
 *   x.size() == 0;
 *
 *   x.push(1);
 *   x.empty() == false;
 *   x.size() == 1;
 *
 *   x.push(2);
 *   x.push(1); // Note that 1 is duplicated
 *
 *   // Since 1 is duplicated, the highest priority 1 is kept
 *   x.top() == 1;
 *   x.pop();
 *   x.empty() == false;
 *   x.size() == 2;
 *
 *   // Elements that are not duplicated are returned in LIFO order
 *   x.top() == 2;
 *   x.pop();
 *
 *   // Note that the lowest priority 1 is removed
 *   x.empty() == true;
 */
template<typename value_t>
class UniqueStack
{
public:
    bool empty() const
    { return m_value_to_index.empty(); }

    size_t size() const
    { return m_value_to_index.size(); }

    bool remove(value_t const &value)
    {
        if (m_value_to_index.count(value) == 0)
            return false;

        int idx = m_value_to_index[value];
        m_index_to_value.erase(idx);
        m_value_to_index.erase(value);
        return true;
    }

    value_t top() const
    {
        // Since keys in std::map are sorted, the last key (i.e. the first in
        // reverse order) is the one we care about
        unsigned long last_key = m_index_to_value.rbegin()->first;

        // We have to do this to keep const around (since operator[] creates
        // values)
        return m_index_to_value.find(last_key)->second;
    };

    void pop()
    {
        unsigned long last_key = m_index_to_value.rbegin()->first;
        value_t const &last_value = m_index_to_value[last_key];

        m_index_to_value.erase(last_key);
        m_value_to_index.erase(last_value);
    };

    void push(value_t const &member)
    {
        // If there is a duplicate, then purge it
        if (m_value_to_index.count(member) == 1)
        {
            unsigned long old_key = m_value_to_index[member];
            m_value_to_index.erase(member);
            // Don't purge from m_index_to_value, since it will be overwritten
        }

        unsigned long last_key;
        if (empty())
            last_key = 0;
        else
            last_key = m_index_to_value.rbegin()->first;

        m_index_to_value[last_key + 1] = member;
        m_value_to_index[member] = last_key + 1;
    };
private:
    // We have to keep two, so that we can retrieve elements by either keys
    // or values. Indexing by keys are useful to figure out the last element
    // in the sequence, and indexing by values are useful to find duplicates.
    std::map<unsigned long, value_t> m_index_to_value;
    std::map<value_t, unsigned long> m_value_to_index;
};
#endif
