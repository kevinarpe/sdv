//
// Created by kca on 21/6/2020.
//

#ifndef SDV_TEXTVIEWLINETEXTFORMAT_H
#define SDV_TEXTVIEWLINETEXTFORMAT_H

#include <memory>
#include <cassert>

namespace SDV {

class PaintEventFunctor;

// TODO: Allow overlapping.  Why?  JSON text formatting plus find highlighting.
struct TextViewLineTextFormat
{
    int charIndex;
    int length;
    std::shared_ptr<PaintEventFunctor> paintEventFunctor;

    TextViewLineTextFormat(int p_charIndex, int p_length, std::shared_ptr<PaintEventFunctor> p_paintEventFunctor)
        : charIndex{p_charIndex}, length{p_length}, paintEventFunctor{p_paintEventFunctor}
    {
        assert(isValid());
    }

    bool isValid() const
    {
        const bool x = (charIndex >= 0 && length > 0 && nullptr != paintEventFunctor.get());
        return x;
    }

    bool isOverlapping(const TextViewLineTextFormat& rhs) const
    {
        // Ref: https://stackoverflow.com/a/12888920/257299
        // Ref: https://stackoverflow.com/a/325964/257299
        const int maxBegin = std::max(charIndex, rhs.charIndex);
        const int minEnd = std::min(charIndex + length, rhs.charIndex + rhs.length);
        // Why operator< ?  Begin and end overlap is allowed.
        // Ex: [0, 2]->[0, 3) and [3, 5]->[3, 6)
        // The first segment has three indices: 0, 1, 2.  Inclusive begin = 0, inclusive end = 2, exclusive end = 3
        // The second segment has three indices: 3, 4, 5.  Inclusive begin = 3, inclusive end = 5, exclusive end = 6
        const bool x = (maxBegin < minEnd);
        return x;
    }

//    using TextFormatSet = std::set<std::shared_ptr<TextViewLineTextFormat>, TextViewLineTextFormat::NonOverlapCompare>;

    // Ref: https://stackoverflow.com/a/46128321/257299
    struct NonOverlapCompare
    {
        bool operator()(const TextViewLineTextFormat& lhs,
                        const TextViewLineTextFormat& rhs) const
        {
            if (lhs.isOverlapping(rhs)) {
                return false;
            }
            const bool x = (lhs.charIndex < rhs.charIndex);
            return x;
        }
    };
};

}  // namespace SDV

#endif //SDV_TEXTVIEWLINETEXTFORMAT_H
