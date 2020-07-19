//
// Created by kca on 18/7/2020.
//

#ifndef SDV_NEXTIDSERVICE_H
#define SDV_NEXTIDSERVICE_H

#include <atomic>

namespace SDV {

/**
 * Why is this a separate class?  Because it is surprisingly easy to make a mistake!
 */
// @ThreadSafe
class NextIdService
{
public:
    static NextIdService withFirstId(const int firstId)
    {
        NextIdService x = NextIdService{firstId};
        return x;
    }

    NextIdService() = delete;
    NextIdService(const NextIdService&) = delete;

    // GCC says: "implicitly deleted because the default definition would be ill-formed"
    // Why?  std::atomic is neither copyable nor movable.
//    NextIdService(NextIdService&&) = default;
    NextIdService(NextIdService&& other) : m_prevId{other.m_prevId.load()} {}

    NextIdService& operator=(const NextIdService&) = delete;
    NextIdService& operator=(NextIdService&&) = delete;

    int nextId()
    {
        // Intentional: add-then-fetch (*NOT* fetch-then-add) to guarantee parallel threads with get different values.
        const int x = ++m_prevId;
        return x;
    }

private:
    explicit NextIdService(const int firstId) : m_prevId{firstId - 1} {}

    std::atomic<int> m_prevId;
};

}  // namespace SDV

#endif //SDV_NEXTIDSERVICE_H
