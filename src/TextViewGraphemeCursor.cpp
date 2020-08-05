//
// Created by kca on 17/6/2020.
//

#include "TextViewGraphemeCursor.h"
#include "TextViewDocumentView.h"
#include "TextViewDocument.h"
#include "GraphemeFinder.h"
#include "TextViewTextCursor.h"
#include "TextViewGraphemePosition.h"

namespace SDV {

static const QString kEmptyTextLine{};

struct TextViewGraphemeCursor::Private
{
    static void
    setGrapheme(TextViewGraphemeCursor& self, const QString& line,
                const int graphemeBeginIndex, const int graphemeEndIndex)
    {
        const int graphemeLength = graphemeEndIndex - graphemeBeginIndex;
        assert(1 == graphemeLength || 2 == graphemeLength);
        const QString& grapheme = line.mid(graphemeBeginIndex, graphemeLength);
        self.m_pos.grapheme = grapheme;
        self.m_pos.isEndOfLine = false;
    }

    static void
    home(TextViewGraphemeCursor& self)
    {
        self.m_graphemeFinder.toStart();
        self.m_pos.pos.charIndex = self.m_pos.graphemeIndex = 0;

        const int graphemeEndIndex = self.m_graphemeFinder.toNextBoundary();
        // Empty line
        if (-1 == graphemeEndIndex)
        {
            self.m_pos.grapheme = TextViewGraphemePosition::kEndOfLineGrapheme;
            self.m_pos.isEndOfLine = true;
        }
        else {
            const QString& line = self.textLine();
            setGrapheme(self, line, 0, graphemeEndIndex);
        }
    }

    static void
    end(TextViewGraphemeCursor& self)
    {
        const QString& line = self.textLine();
        self.m_pos.pos.charIndex = line.length();
        self.m_pos.grapheme = TextViewGraphemePosition::kEndOfLineGrapheme;
        self.m_pos.isEndOfLine = true;

        while (true) {
            const int b = self.m_graphemeFinder.toNextBoundary();
            if (-1 == b) {
                break;
            }
            self.m_pos.graphemeIndex++;
        }
        // "End" is special.  Text cursor is one position beyond last char in text line.
        self.m_pos.graphemeIndex++;
        assert(self.m_pos.graphemeIndex <= self.m_pos.pos.charIndex);
    }
};

// public
void
TextViewGraphemeCursor::
reset()
{
    // Important: textLine() will assert if m_pos is invalid.
    m_pos.pos.lineIndex = m_pos.pos.charIndex = m_pos.graphemeIndex = 0;
    m_pos.grapheme = TextViewGraphemePosition::kEndOfLineGrapheme;
    m_pos.isEndOfLine = true;
    const QString& line = textLine();
    m_graphemeFinder.reset(QTextBoundaryFinder::BoundaryType::Grapheme, line);

    if (line.isEmpty() == false)
    {
        const int graphemeBeginIndex = 0;
        const int graphemeEndIndex = m_graphemeFinder.toNextBoundary();
        assert(-1 != graphemeEndIndex);
        Private::setGrapheme(*this, line, graphemeBeginIndex, graphemeEndIndex);
    }
}

// public
const QString&
TextViewGraphemeCursor::
textLine()
const
{
    assert(m_pos.isValid());
    const std::vector<QString>& lineVec = m_docView->doc().lineVec();
    if (0 == m_pos.pos.lineIndex && lineVec.empty()) {
        return kEmptyTextLine;
    }
    else {
        const QString& x = lineVec[m_pos.pos.lineIndex];
        return x;
    }
}

// public
void
TextViewGraphemeCursor::
left()
{
    assert(m_pos.pos.charIndex > 0);
    const QString& line = textLine();

    const int graphemeEndIndex = m_graphemeFinder.position();
    assert(graphemeEndIndex > 0);

    int graphemeBeginIndex = m_graphemeFinder.toPreviousBoundary();
    assert(-1 != graphemeBeginIndex);

    // If not text cursor at end of line...
    if (m_pos.pos.charIndex < line.length())
    {
        graphemeBeginIndex = m_graphemeFinder.toPreviousBoundary();
        assert(-1 != graphemeBeginIndex);
    }
    // Intentional: Always points to the QChar index *after* the current grapheme.
    const int graphemeEndIndex2 = m_graphemeFinder.toNextBoundary();
    assert(-1 != graphemeEndIndex2);

    m_pos.pos.charIndex = graphemeBeginIndex;
    m_pos.graphemeIndex--;
    assert(m_pos.graphemeIndex >= 0);
    Private::setGrapheme(*this, line, graphemeBeginIndex, graphemeEndIndex2);
}

// public
void
TextViewGraphemeCursor::
right()
{
    const QString& line = textLine();
    assert(m_pos.pos.charIndex < line.length());

    if (m_graphemeFinder.position() == line.length())
    {
        m_pos.pos.charIndex++;
        m_pos.grapheme = TextViewGraphemePosition::kEndOfLineGrapheme;
        m_pos.isEndOfLine = true;
    }
    else {
        const int graphemeBeginIndex = m_graphemeFinder.position();
        assert(graphemeBeginIndex == m_pos.pos.charIndex + m_pos.grapheme.length());

        const int graphemeEndIndex = m_graphemeFinder.toNextBoundary();
        assert(-1 != graphemeEndIndex);

        m_pos.pos.charIndex = graphemeBeginIndex;
        Private::setGrapheme(*this, line, graphemeBeginIndex, graphemeEndIndex);
    }
    m_pos.graphemeIndex++;
    assert(m_pos.graphemeIndex <= m_pos.pos.charIndex);
}

// public
void
TextViewGraphemeCursor::
home()
{
    assert(m_pos.pos.charIndex > 0);
    Private::home(*this);
}

// public
void
TextViewGraphemeCursor::
end()
{
    const QString& line = textLine();
    assert(m_pos.pos.charIndex < line.length());
    Private::end(*this);
}

// public
void
TextViewGraphemeCursor::
setCharIndex(const int charIndex)
{
    // Eh.  Not so efficient!
    if (charIndex <= m_pos.pos.charIndex)
    {
        m_graphemeFinder.toStart();
        m_pos.graphemeIndex = 0;
    }
    const int p = m_graphemeFinder.position();
    if (p < charIndex)
    {
        while (true) {
            const int b = m_graphemeFinder.toNextBoundary();
            assert(-1 != b);
            assert(b <= charIndex);
            if (b == charIndex) {
                break;
            }
            m_pos.graphemeIndex++;
        }
    }
    m_pos.pos.charIndex = charIndex;
    const int graphemeBeginIndex = m_graphemeFinder.position();
    const int graphemeEndIndex = m_graphemeFinder.toNextBoundary();
    if (-1 == graphemeEndIndex)
    {
        assert(charIndex == m_graphemeFinder.text().length());
        m_pos.grapheme = TextViewGraphemePosition::kEndOfLineGrapheme;
        m_pos.isEndOfLine = true;
    }
    else {
        assert(charIndex < m_graphemeFinder.text().length());
        const QString& line = textLine();
        Private::setGrapheme(*this, line, graphemeBeginIndex, graphemeEndIndex);
    }
}

// public
void
TextViewGraphemeCursor::
setLineIndexThenHome(const int lineIndex)
{
    assert(lineIndex != m_pos.pos.lineIndex);
    m_pos.pos.lineIndex = lineIndex;
    const QString& line = textLine();
    m_graphemeFinder.reset(QTextBoundaryFinder::BoundaryType::Grapheme, line);
    Private::home(*this);
}

// public
void
TextViewGraphemeCursor::
setLineIndexThenEnd(const int lineIndex)
{
    setLineIndexThenHome(lineIndex);
    Private::end(*this);
}

// public
TextSegmentFontWidth
TextViewGraphemeCursor::
horizontalMove(const QFontMetricsF& fontMetricsF, const qreal fontWidth)
{
    assert(fontWidth >= 0.0);
    const QString& line = textLine();

    const GraphemeFinder::Result& r =
        GraphemeFinder::positionForFontWidth(
            &m_graphemeFinder, line, fontMetricsF, fontWidth, GraphemeFinder::IncludeTextCursor::Yes);

    m_pos.pos.charIndex = r.charIndex;
    m_pos.graphemeIndex = r.graphemeIndex;
    m_pos.grapheme = r.grapheme;
    m_pos.isEndOfLine = r.isEndOfLine;
    return r.fontWidth;
}

// public
void
TextViewGraphemeCursor::
setPosition(const TextViewPosition& pos)
{
    if (pos.lineIndex != m_pos.pos.lineIndex)
    {
        setLineIndexThenHome(pos.lineIndex);
    }
    setCharIndex(pos.charIndex);
}

}  // namespace SDV
