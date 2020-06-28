//
// Created by kca on 25/6/2020.
//

#ifndef SDV_LINESEGMENT_H
#define SDV_LINESEGMENT_H

namespace SDV {

struct LineSegment
{
    /**
     * {@link QChar} offset in a {@link QString} line of text
     * <p>
     * This should be at a grapheme boundary.
     */
    int charIndex;

    /**
     * Number of {@link QChar}s from a {@link QString} line of text
     * <p>
     * This should be at a grapheme boundary.
     */
    int length;

    bool isValid() const
    {
        const bool x = (charIndex >= 0 && length > 0);
        return x;
    }

    bool isOverlapping(const LineSegment& rhs) const;

    /**
     * @tparam LineSegmentParent
     *         either {@link PainterBackgroundFunctor} or {@link PainterForegroundFunctor}
     */
    // Ref: https://stackoverflow.com/a/46128321/257299
    template <typename LineSegmentParent>
    struct NonOverlapCompare
    {
        bool operator()(const LineSegmentParent& lhs, const LineSegmentParent& rhs) const
        {
            const LineSegment& lhs2 = lhs.seg;
            const LineSegment& rhs2 = rhs.seg;
            if (lhs2.isOverlapping(rhs2)) {
                return false;
            }
            const bool x = (lhs2.charIndex < rhs2.charIndex);
            return x;
        }
    };
};

}  // namespace SDV

#endif //SDV_LINESEGMENT_H
