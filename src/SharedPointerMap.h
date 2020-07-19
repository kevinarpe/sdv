//
// Created by kca on 13/7/2020.
//

#ifndef SDV_SHAREDPOINTERMAP_H
#define SDV_SHAREDPOINTERMAP_H

#include <unordered_map>
#include <memory>
#include <algorithm>
#include <cassert>
#include "Algorithm.h"

namespace SDV {

/**
 * @tparam T
 *         Example: {@code std::shared_ptr<...>}
 *
 * @see ThreadSafeSharedPointerMap
 */
template<typename T>
class SharedPointerMap final
{
public:
    SharedPointerMap() : m_nextId{1} {}
    // Super explicit!
    ~SharedPointerMap() = default;
    SharedPointerMap(const SharedPointerMap&) = delete;
    SharedPointerMap(SharedPointerMap&&) = delete;
    SharedPointerMap& operator=(const SharedPointerMap&) = delete;
    SharedPointerMap& operator=(SharedPointerMap&&) = delete;

    int insertNewOrAssert(const std::shared_ptr<T>& p)
    {
        assertValue_NOT_Exists(p);

        const int id = m_nextId;
        ++m_nextId;
     
        std::pair<typename map_t::iterator, bool> result = m_map.emplace(id, p);
        assert(result.second);
        return id;
    }

    const std::shared_ptr<T>&
    getOrAssert(const int id)
    const
    {
        const std::shared_ptr<T>& x = Algorithm::Map::getOrAssert(m_map, id);
        return x;
    }

    void eraseOrAssert(const int id)
    {
        Algorithm::Map::eraseByKeyOrAssert(m_map, id);
    }

private:
    void assertValue_NOT_Exists(const std::shared_ptr<T>& p)
    {
        const auto it = std::find_if(m_map.begin(), m_map.end(), [&p](const auto& entry) { return entry.second == p; });
        assert(m_map.end() == it);
    }

    int m_nextId;
    using map_t = std::unordered_map<int, std::shared_ptr<T>>;
    map_t m_map;
};

}  // namespace SDV

#endif //SDV_SHAREDPOINTERMAP_H
