//
// Created by kca on 17/6/2020.
//

#ifndef SDV_TEXTVIEWGRAPHEMECURSOR_H
#define SDV_TEXTVIEWGRAPHEMECURSOR_H

#include <memory>
#include <QFontMetricsF>
#include "TextViewGraphemePosition.h"
#include "TextBoundaryFinder.h"
#include "TextSegmentFontWidth.h"

namespace SDV {

class TextViewDocumentView;

/**
 * Tracks the location of the text cursor on single line of text.  This class is careful to "step" by full graphemes, instead of QChars.
 * This helps with wide graphemes that use two QChars, such as emoji "😋".
 */
class TextViewGraphemeCursor
{
public:
    explicit TextViewGraphemeCursor(const std::shared_ptr<TextViewDocumentView>& docView)
        : m_docView{docView}, m_pos{TextViewGraphemePosition::invalid()}
    {}

    void reset();
    const TextViewGraphemePosition& pos() const { return m_pos; }
    const QString& textLine() const;

    bool isAtEnd() const
    {
        const QString& line = textLine();
        const int len = line.length();
        return len == m_pos.pos.charIndex;
    }
    void left();
    void right();
    void home();
    void end();
    void setCharIndex(int charIndex);
    void setLineIndexThenHome(int lineIndex);
    void setLineIndexThenEnd(int lineIndex);
    TextSegmentFontWidth horizontalMove(const QFontMetricsF& fontMetricsF, qreal fontWidth);
    void setPosition(const TextViewPosition& pos);

private:
    struct Private;
    std::shared_ptr<TextViewDocumentView> m_docView;
    /**
     * Always points to the QChar index *after* the current grapheme.
     *
     * For example: In the text "😋r", there are two graphemes, and the QString length is three QChars.
     * (Note: The first grapheme "😋" requires two QChars.)
     *
     * When 0 == m_pos.graphemeIndex, 2 == m_graphemeFinder.position().
     * When 1 == m_pos.graphemeIndex, 3 == m_graphemeFinder.position().
     */
    TextBoundaryFinder m_graphemeFinder;
    TextViewGraphemePosition m_pos;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWGRAPHEMECURSOR_H
