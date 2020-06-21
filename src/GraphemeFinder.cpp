//
// Created by kca on 16/6/2020.
//

#include "GraphemeFinder.h"
#include "TextBoundaryFinder.h"
#include "TextViewTextCursor.h"

namespace SDV {

// public
GraphemeFinder::
GraphemeFinder()
//    : m_finder{std::make_unique<TextBoundaryFinder>()}
    : m_finder{std::make_shared<TextBoundaryFinder>()}
{}

// public
GraphemeFinder::Result
GraphemeFinder::
positionForFontWidth(const QString& textLine,
                     const QFontMetricsF& fontMetricsF,
                     const qreal fontWidth,
                     const IncludeTextCursor includeTextCursor)
{
    const GraphemeFinder::Result& x =
        positionForFontWidth(m_finder.get(), textLine, fontMetricsF, fontWidth, includeTextCursor);
    return x;
}

// public static
GraphemeFinder::Result
GraphemeFinder::
positionForFontWidth(TextBoundaryFinder* const finder,
                     const QString& textLine,
                     const QFontMetricsF& fontMetricsF,
                     const qreal fontWidth,
                     const IncludeTextCursor includeTextCursor)
{
    assert(nullptr != finder);
    finder->reset(QTextBoundaryFinder::BoundaryType::Grapheme, textLine);
    int graphemeBeginIndex = 0;
    int graphemeIndex = 0;
    qreal graphemeWidth = 0;
    qreal width = 0;
    while (true)
    {
        const int graphemeEndIndex = finder->toNextBoundary();
        if (-1 == graphemeEndIndex)
        {
            if (IncludeTextCursor::Yes == includeTextCursor)
            {
                const qreal graphemeWidth = fontMetricsF.horizontalAdvance(TextViewTextCursor::SPACE_CHAR);
                const Result& x = Result{.charIndex = graphemeBeginIndex,
                    .graphemeIndex = graphemeIndex,
                    .grapheme = TextViewTextCursor::SPACE_GRAPHEME,
                    .fontWidth = TextSegmentFontWidth{.beforeGrapheme = width, .grapheme = graphemeWidth}};
                return x;
            }
            else {
                const Result& x = Result::invalid();
                return x;
            }
        }
        const int graphemeLength = graphemeEndIndex - graphemeBeginIndex;
        const QString& grapheme = textLine.mid(graphemeBeginIndex, graphemeLength);
        graphemeWidth = fontMetricsF.horizontalAdvance(grapheme);
        width += graphemeWidth;
        if (width >= fontWidth)
        {
            const Result& x =
                Result{
                    .charIndex = graphemeBeginIndex,
                    .graphemeIndex = graphemeIndex,
                    .grapheme = grapheme,
                    .fontWidth = TextSegmentFontWidth{.beforeGrapheme = width - graphemeWidth, .grapheme = graphemeWidth}
                };
            return x;
        }
        graphemeBeginIndex = graphemeEndIndex;
        ++graphemeIndex;
    }
}

}  // namespace SDV
