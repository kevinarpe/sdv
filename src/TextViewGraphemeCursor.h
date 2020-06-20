//
// Created by kca on 17/6/2020.
//

#ifndef SDV_TEXTVIEWGRAPHEMECURSOR_H
#define SDV_TEXTVIEWGRAPHEMECURSOR_H

#include <memory>
#include "TextViewPosition.h"
#include "TextBoundaryFinder.h"
#include "TextSegmentFontWidth.h"

namespace SDV {

class TextViewDocumentView;

class TextViewGraphemeCursor
{
public:
    explicit TextViewGraphemeCursor(const std::shared_ptr<TextViewDocumentView>& docView)
        : m_docView{docView}, m_pos{TextViewPosition::invalid()}
    {}

    void reset();
    const TextViewPosition& pos() const { return m_pos; }
    const QString& textLine() const;

    bool isAtEnd() const
    {
        const QString& line = textLine();
        const int len = line.length();
        return len == m_pos.charIndex;
    }
    void left();
    void right();
    void home();
    void end();
    void setLineIndexThenHome(int lineIndex);
    void setLineIndexThenEnd(int lineIndex);
    TextSegmentFontWidth horizontalMove(const QFontMetricsF& fontMetricsF, qreal fontWidth);

private:
    struct Private;
    std::shared_ptr<TextViewDocumentView> m_docView;
    /**
     * Always points to the QChar index *after* the current grapheme.
     *
     * For example: In the text "😋r", the QString length is 3 QChars.  The first grapheme "😋" requires two QChars.
     * When 0 == m_pos.graphemeIndex, 2 == m_graphemeFinder.position().
     * When 1 == m_pos.graphemeIndex, 3 == m_graphemeFinder.position().
     */
    TextBoundaryFinder m_graphemeFinder;
    TextViewPosition m_pos;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWGRAPHEMECURSOR_H
