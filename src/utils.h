/** @file */
#ifndef __SMALLWM_UTILS__
#define __SMALLWM_UTILS__

#include <cstdlib>
#include <cstring>
#include <map>

unsigned long try_parse_ulong(const char *string, unsigned long default_);
unsigned long try_parse_ulong_nonzero(const char *string, unsigned long default_);
void strip_string(const char *text, const char *remove, char *buffer);

/**
 * A generic functor useful for sorting a vector according to a map of items.
 *
 * For example, consider:
 *
 *  - A map: {a:1, b:0, c:2, d:4, e:3}
 *  - A vector: [a, b, c, d, e]
 *
 * This class will sort that vector as:
 *  [b, a, c, e, d]
 */
template<typename Key, typename Value>
class MappedVectorSorter 
{
public:
    /// Initializes the builtin map and other parameters
    MappedVectorSorter(std::map<Key, Value> &map, bool reversed) :
        m_map(map), m_reversed(reversed)
    {};

    /** Sorts to keys in the map according to their values.
     * @param a The first key value.
     * @param b The second key value.
     * @return true if map[a] < map[b], otherwise false
     */
    bool operator()(Key a, Key b)
    {
        if (!m_reversed)
            return m_map[a] < m_map[b];
        else
            return m_map[a] > m_map[b];
    };

private:
    /// An association to sort the vector by.
    std::map<Key, Value> &m_map;

    // Whether to sort in ascending [false] or descending [true] order
    bool m_reversed;
};

#endif
