//
// Created by kca on 12/7/2020.
//

#include "TextViewTextStatsService.h"
#include "TextViewDocument.h"
#include "TextViewDocumentView.h"
#include "QStrings.h"
#include "TextViewSelection.h"
#include "Constants.h"

namespace SDV {

struct TextViewTextStatsService::Private
{
    static void
    updateStats(TextViewTextStatsService& self)
    {
        const std::vector<QString>& lineVec = self.m_docView->doc().lineVec();
        std::size_t size = lineVec.size();
        self.m_lineIndex_To_Counts.resize(size);

        for (std::size_t i = 0; i < size; ++i)
        {
            const QString& line = lineVec[i];

            self.m_textBoundaryFinder.reset(QTextBoundaryFinder::BoundaryType::Grapheme, line);

            const int graphemeCount = self.m_textBoundaryFinder.countAll();
            const int utf8ByteCount = QStrings::utf8ByteCount(line, 0, line.length());

            Counts& counts = self.m_lineIndex_To_Counts[i];
            counts.graphemeCount = graphemeCount;
            counts.utf8ByteCount = utf8ByteCount;
        }
    }

    /**
     * Method {@link #updateStats(...)} must be called first!
     */
    static Result
    calcAllStats(TextViewTextStatsService& self)
    {
        Result result{
            .lineCount = static_cast<int>(self.m_docView->doc().lineVec().size()),
            .graphemeCount = 0,
            .utf8ByteCount = 0,
        };
        for (const Counts& counts : self.m_lineIndex_To_Counts)
        {
            result.graphemeCount += counts.graphemeCount;
            result.utf8ByteCount += counts.utf8ByteCount;
        }
        addNewLines(&result);
        return result;
    }

    static void
    addNewLines(Result* result)
    {
        assert(nullptr != result);
        // Note(1): Windows new-line ("\r\n") is two graphemes.
        // Node(2): Last line does not include new-line.
        result->graphemeCount += (result->lineCount - 1) * Constants::kNewLine.length();
        result->utf8ByteCount += (result->lineCount - 1) * Constants::kNewLine.length();
    }

    static Result
    calcStatsSingleLine(TextViewTextStatsService& self, const TextViewPosition& begin, const TextViewPosition& end)
    {
        const std::vector<QString>& lineVec = self.m_docView->doc().lineVec();
        const QString& line = lineVec[begin.lineIndex];
        Result result{
            .lineCount = 1,
            .graphemeCount = 0,
            .utf8ByteCount = 0,
        };
        // Is selection a single, whole line?
        if (0 == begin.charIndex && line.length() == end.charIndex)
        {
            const Counts& counts = self.m_lineIndex_To_Counts[begin.lineIndex];
            result.graphemeCount += counts.graphemeCount;
            result.utf8ByteCount += counts.utf8ByteCount;
        }
        // Selection is a single, partial line.
        else {
            self.m_textBoundaryFinder.reset(QTextBoundaryFinder::BoundaryType::Grapheme, line);

            result.graphemeCount =
                self.m_textBoundaryFinder.countRange(begin.charIndex, end.charIndex - begin.charIndex + 1);

            result.utf8ByteCount =
                QStrings::utf8ByteCount(line, begin.charIndex, end.charIndex - begin.charIndex + 1);
        }
        return result;
    }

    static void
    calcStatsFirstLine(TextViewTextStatsService& self, const TextViewPosition& begin, Result* result)
    {
        assert(nullptr != result);

        const std::vector<QString>& lineVec = self.m_docView->doc().lineVec();
        const QString& line = lineVec[begin.lineIndex];

        // Is selection first line a whole line?
        if (0 == begin.charIndex)
        {
            const Counts& counts = self.m_lineIndex_To_Counts[begin.lineIndex];
            result->graphemeCount += counts.graphemeCount;
            result->utf8ByteCount += counts.utf8ByteCount;
        }
        // Is selection first line zero-length?
        else if (line.length() == begin.charIndex)
        {
            // @DebugBreakpoint
            int dummy = 1;
        }
        // Selection first line is a partial line.
        else {
            self.m_textBoundaryFinder.reset(QTextBoundaryFinder::BoundaryType::Grapheme, line);

            const int graphemeCount = self.m_textBoundaryFinder.countRange(begin.charIndex, line.length() - begin.charIndex);
            result->graphemeCount += graphemeCount;

            const int utf8ByteCount = QStrings::utf8ByteCount(line, begin.charIndex, line.length() - begin.charIndex);
            result->utf8ByteCount += utf8ByteCount;
        }
    }

    static void
    calcStatsLastLine(TextViewTextStatsService& self, const TextViewPosition& end, Result* result)
    {
        assert(nullptr != result);

        const std::vector<QString>& lineVec = self.m_docView->doc().lineVec();
        const QString& line = lineVec[end.lineIndex];

        // Is selection last line a whole line?
        if (line.length() == end.charIndex)
        {
            const Counts& counts = self.m_lineIndex_To_Counts[end.lineIndex];
            result->graphemeCount += counts.graphemeCount;
            result->utf8ByteCount += counts.utf8ByteCount;
        }
        // Is selection last line zero-length?
        else if (0 == end.charIndex)
        {
            // @DebugBreakpoint
            int dummy = 1;
        }
        // Selection last line is a partial line.
        else {
            self.m_textBoundaryFinder.reset(QTextBoundaryFinder::BoundaryType::Grapheme, line);

            const int graphemeCount = self.m_textBoundaryFinder.countRange(0, end.charIndex);
            result->graphemeCount += graphemeCount;

            const int utf8ByteCount = QStrings::utf8ByteCount(line, 0, end.charIndex);
            result->utf8ByteCount += utf8ByteCount;
        }
    }

    static void
    calcStatsOtherLines(TextViewTextStatsService& self, const TextViewPosition& begin, const TextViewPosition& end, Result* result)
    {
        const TextViewDocumentView::const_iterator beginIter = self.m_docView->findOrAssert(begin.lineIndex);
        const TextViewDocumentView::const_iterator lastIter = self.m_docView->findOrAssert(end.lineIndex);

        for (TextViewDocumentView::const_iterator iter = 1 + beginIter; lastIter != iter; ++iter)
        {
            const int& lineIndex = *iter;
            const Counts& counts = self.m_lineIndex_To_Counts[lineIndex];
            result->graphemeCount += counts.graphemeCount;
            result->utf8ByteCount += counts.utf8ByteCount;
        }
    }
};

TextViewTextStatsService::
TextViewTextStatsService(const std::shared_ptr<TextViewDocumentView>& docView)
    : m_docView{docView}
{}

/** {@inheritDoc} */
TextViewTextStatsService::Result
TextViewTextStatsService::
setDoc(const std::shared_ptr<TextViewDocument>& doc)
{
    m_docView->setDoc(doc);
    Private::updateStats(*this);
    const Result x = Private::calcAllStats(*this);
    return x;
}

/** {@inheritDoc} */
TextViewTextStatsService::Result
TextViewTextStatsService::
calcStats(const TextViewSelection& selection)
{
    assert(selection.isValid());
    const TextViewPosition& begin = selection.begin.isLessThan(selection.end) ? selection.begin : selection.end;
    const TextViewPosition& end = selection.begin.isLessThan(selection.end) ? selection.end : selection.begin;

    // Is selection a single line?
    if (begin.lineIndex == end.lineIndex)
    {
        const Result x = Private::calcStatsSingleLine(*this, begin, end);
        return x;
    }
    Result result{
        .lineCount = end.lineIndex - begin.lineIndex + 1,
        .graphemeCount = 0,
        .utf8ByteCount = 0,
    };
    // Order does not matter for any of these methods.
    Private::calcStatsFirstLine(*this, begin, &result);
    Private::calcStatsLastLine(*this, end, &result);
    Private::calcStatsOtherLines(*this, begin, end, &result);
    Private::addNewLines(&result);

    return result;
}

}  // namespace SDV
