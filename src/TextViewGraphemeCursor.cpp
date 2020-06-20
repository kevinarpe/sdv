//
// Created by kca on 17/6/2020.
//

#include "TextViewGraphemeCursor.h"
#include "TextViewDocumentView.h"
#include "TextViewDocument.h"
#include "GraphemeFinder.h"

namespace SDV {

static const QString EMPTY_TEXT_LINE{};
static const QChar SPACE_CHAR{QLatin1Char{' '}};
/** Used for end of line cursor */
static const QString SPACE_GRAPHEME{SPACE_CHAR};

struct TextViewGraphemeCursor::Private
{
    static void
    setGrapheme(TextViewGraphemeCursor& self, const QString& line,
                const int graphemeBeginIndex, const int graphemeEndIndex)
    {
        const int graphemeLength = graphemeEndIndex - graphemeBeginIndex;
        assert(1 == graphemeLength || 2 == graphemeLength);
        self.m_pos.grapheme = line.mid(graphemeBeginIndex, graphemeLength);
    }

    static void
    home(TextViewGraphemeCursor& self)
    {
        self.m_pos.charIndex = self.m_pos.graphemeIndex = 0;
        self.m_graphemeFinder.toStart();

        const int graphemeEndIndex = self.m_graphemeFinder.toNextBoundary();
        // Empty line
        if (-1 == graphemeEndIndex) {
            self.m_pos.grapheme = SPACE_GRAPHEME;
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

        self.m_pos.charIndex = line.length();
        self.m_pos.grapheme = SPACE_GRAPHEME;

        while (true) {
            const int b = self.m_graphemeFinder.toNextBoundary();
            if (-1 == b) {
                break;
            }
            self.m_pos.graphemeIndex++;
        }
        // "End" is special.  Text cursor is one position beyond last char in text line.
        self.m_pos.graphemeIndex++;
        assert(self.m_pos.graphemeIndex <= self.m_pos.charIndex);
    }
};

// public
void
TextViewGraphemeCursor::
reset()
{
    m_pos.lineIndex = m_pos.charIndex = m_pos.graphemeIndex = 0;
    m_pos.grapheme = SPACE_GRAPHEME;
    const QString& line = textLine();

    if (line.isEmpty())
    {
        m_graphemeFinder.reset(QTextBoundaryFinder::BoundaryType::Grapheme, QString{});
        m_pos.grapheme = SPACE_GRAPHEME;
    }
    else {
        m_graphemeFinder.reset(QTextBoundaryFinder::BoundaryType::Grapheme, line);
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
    if (0 == m_pos.lineIndex && lineVec.empty()) {
        return EMPTY_TEXT_LINE;
    }
    else {
        const QString& x = lineVec[m_pos.lineIndex];
        return x;
    }
}

// public
void
TextViewGraphemeCursor::
left()
{
    assert(m_pos.charIndex > 0);
    const QString& line = textLine();

    const int graphemeEndIndex = m_graphemeFinder.position();
    assert(graphemeEndIndex > 0);

    int graphemeBeginIndex = m_graphemeFinder.toPreviousBoundary();
    assert(-1 != graphemeBeginIndex);

    // If not text cursor at end of line...
    if (m_pos.charIndex < line.length())
    {
        graphemeBeginIndex = m_graphemeFinder.toPreviousBoundary();
        assert(-1 != graphemeBeginIndex);
    }
    // Intentional: Always points to the QChar index *after* the current grapheme.
    const int graphemeEndIndex2 = m_graphemeFinder.toNextBoundary();
    assert(-1 != graphemeEndIndex2);

    m_pos.charIndex = graphemeBeginIndex;
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
    assert(m_pos.charIndex < line.length());

    if (m_graphemeFinder.position() == line.length())
    {
        m_pos.charIndex++;
        m_pos.grapheme = SPACE_GRAPHEME;
    }
    else {
        const int graphemeBeginIndex = m_graphemeFinder.position();
        assert(graphemeBeginIndex == m_pos.charIndex + m_pos.grapheme.length());

        const int graphemeEndIndex = m_graphemeFinder.toNextBoundary();
        assert(-1 != graphemeEndIndex);

        m_pos.charIndex = graphemeBeginIndex;
        Private::setGrapheme(*this, line, graphemeBeginIndex, graphemeEndIndex);
    }
    m_pos.graphemeIndex++;
    assert(m_pos.graphemeIndex <= m_pos.charIndex);
}

// public
void
TextViewGraphemeCursor::
home()
{
    assert(m_pos.charIndex > 0);
    Private::home(*this);
}

// public
void
TextViewGraphemeCursor::
end()
{
    const QString& line = textLine();
    assert(m_pos.charIndex < line.length());
    Private::end(*this);
}

// public
void
TextViewGraphemeCursor::
setLineIndexThenHome(const int lineIndex)
{
    assert(lineIndex != m_pos.lineIndex);
    m_pos.lineIndex = lineIndex;
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

    m_pos.charIndex = r.charIndex;
    m_pos.graphemeIndex = r.graphemeIndex;
    m_pos.grapheme = r.grapheme;
    return r.fontWidth;
}

}  // namespace SDV