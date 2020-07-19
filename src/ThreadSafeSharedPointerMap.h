//
// Created by kca on 13/7/2020.
//

#ifndef SDV_THREADSAFESHAREDPOINTERMAP_H
#define SDV_THREADSAFESHAREDPOINTERMAP_H

#include <mutex>
#include "SharedPointerMap.h"

namespace SDV {

/**
 * @tparam T
 *         Example: {@code std::shared_ptr<...>}
 *
 * @see SharedPointerMap
 */
template<typename T>
class ThreadSafeSharedPointerMap final
{
public:
    ThreadSafeSharedPointerMap() = default;
    ~ThreadSafeSharedPointerMap() = default;
    ThreadSafeSharedPointerMap(const ThreadSafeSharedPointerMap&) = delete;
    ThreadSafeSharedPointerMap(ThreadSafeSharedPointerMap&&) = delete;
    ThreadSafeSharedPointerMap& operator=(const ThreadSafeSharedPointerMap&) = delete;
    ThreadSafeSharedPointerMap& operator=(ThreadSafeSharedPointerMap&&) = delete;

    int insertNewOrAssert(const std::shared_ptr<T>& p)
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        const int x = m_map.insertNewOrAssert(p);
        return x;
    }

    const std::shared_ptr<T>&
    getOrAssert(const int id)
    const
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        const std::shared_ptr<T>& x = m_map.getOrAssert(id);
        return x;
    }

    void eraseOrAssert(const int id)
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_map.eraseOrAssert(id);
    }

private:
    mutable std::mutex m_mutex;
    SharedPointerMap<T> m_map;
};

}  // namespace SDV

#endif //SDV_THREADSAFESHAREDPOINTERMAP_H
