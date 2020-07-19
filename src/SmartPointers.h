//
// Created by kca on 12/7/2020.
//

#ifndef SDV_SMARTPOINTERS_H
#define SDV_SMARTPOINTERS_H

#include <memory>

namespace SDV {

struct SmartPointers
{
private:
    // Ref: https://stackoverflow.com/a/20131949/257299
    template<typename T>
    static constexpr auto DoNothingDeleterTemplateVariable = [](T*){};

public:
    struct Shared
    {
        template<typename T>
        static std::shared_ptr<T>
        createWithoutDeleter(T* const ptr)
        {
            assert(nullptr != ptr);
            const std::shared_ptr<T> x = std::shared_ptr<T>{ptr, DoNothingDeleterTemplateVariable<T>};
            return x;
        }
    };

    struct Unique
    {
        template<typename T>
        static std::unique_ptr<T>
        createWithoutDeleter(T* const ptr)
        {
            assert(nullptr != ptr);
            const std::unique_ptr<T> x = std::unique_ptr<T>{ptr, DoNothingDeleterTemplateVariable<T>};
            return x;
        }
    };
};

}  // namespace SDV

#endif //SDV_SMARTPOINTERS_H
