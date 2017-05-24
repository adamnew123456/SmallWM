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
#endif
