//
// Created by kca on 16/6/2020.
//

#ifndef SDV_GRAPHEMEFINDER_H
#define SDV_GRAPHEMEFINDER_H

#include <memory>
#include <QString>
#include <QFontMetricsF>
#include "TextSegmentFontWidth.h"

namespace SDV {

class TextBoundaryFinder;

class GraphemeFinder
{
public:
    GraphemeFinder();
    ~GraphemeFinder();

    struct Result
    {
        int charIndex;
        int graphemeIndex;
        QString grapheme;
        TextSegmentFontWidth fontWidth;

        bool isValid() const {
            return charIndex >= 0 && graphemeIndex >= 0 && grapheme.length() > 0 && fontWidth.isValid();
        }

        static Result invalid() {
            return Result{
                .charIndex = -1, .graphemeIndex = -1, .grapheme = QString{}, .fontWidth = TextSegmentFontWidth::invalid()
            };
        }
    };

    enum class IncludeTextCursor { Yes, No };

    /**
     * @param includeTextCursor
     *        if IncludeTextCursor::Yes, then return space (0x20) grapheme when past end of text
     *        <br>if IncludeTextCursor::No, then return Result::invalid() when past end of text
     */
    Result
    positionForFontWidth(const QString& textLine,
                         const QFontMetricsF& fontMetricsF,
                         qreal fontWidth,
                         IncludeTextCursor includeTextCursor);

    static Result
    positionForFontWidth(TextBoundaryFinder* finder,
                         const QString& textLine,
                         const QFontMetricsF& fontMetricsF,
                         qreal fontWidth,
                         IncludeTextCursor includeTextCursor);

private:
    std::unique_ptr<TextBoundaryFinder> m_finder;
};

}  // namespace SDV

#endif //SDV_GRAPHEMEFINDER_H
