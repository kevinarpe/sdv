//
// Created by kca on 10/6/2020.
//

#include "TextViewDocumentView.h"
#include <numeric>
#include <algorithm>
#include "TextViewDocument.h"
#include "Algorithm.h"
#include "CountingIterator.h"

namespace SDV {

struct TextViewDocumentView::Private
{
    static std::vector<int>::iterator
    findOrAssert(TextViewDocumentView& self, const int lineIndex)
    {
        std::vector<int>::iterator iter =
            std::lower_bound(self.m_visibleLineIndexVec.begin(), self.m_visibleLineIndexVec.end(), lineIndex);

        assert(self.m_visibleLineIndexVec.end() != iter);
        assert(lineIndex == *iter);
        return iter;
    }

    static std::vector<int>::const_iterator
    findOrAssert(const TextViewDocumentView& self, const int lineIndex)
    {
        std::vector<int>::const_iterator iter =
            std::lower_bound(self.m_visibleLineIndexVec.begin(), self.m_visibleLineIndexVec.end(), lineIndex);

        assert(self.m_visibleLineIndexVec.end() != iter);
        assert(lineIndex == *iter);
        return iter;
    }

    static void
    assertValidLineIndex(const TextViewDocumentView& self, const int lineIndex)
    {
        assert(lineIndex >= 0 && lineIndex < self.m_doc->lineVec().size());
    }
};

// public
TextViewDocumentView::
TextViewDocumentView()
{
    const std::shared_ptr<TextViewDocument>& doc = TextViewDocument::staticEmpty();
    setDoc(doc);
}

// public
const TextViewDocument&
TextViewDocumentView::
doc()
const
{
    return *m_doc;
}

// public
void
TextViewDocumentView::
setDoc(const std::shared_ptr<TextViewDocument>& doc)
{
    m_doc = doc;
    m_visibleLineIndexVec.resize(doc->lineVec().size());
    std::iota(m_visibleLineIndexVec.begin(), m_visibleLineIndexVec.end(), 0);
}

// public
void
TextViewDocumentView::
showLine(const int lineIndex)
{
    Private::assertValidLineIndex(*this, lineIndex);

    const std::vector<int>::const_iterator iter =
        std::lower_bound(m_visibleLineIndexVec.begin(), m_visibleLineIndexVec.end(), lineIndex);

    if (m_visibleLineIndexVec.end() == iter || lineIndex != *iter)
    {
        m_visibleLineIndexVec.insert(iter, lineIndex);
    }
}

// public
void
TextViewDocumentView::
hideLine(const int lineIndex)
{
    Private::assertValidLineIndex(*this, lineIndex);

    const std::vector<int>::const_iterator iter =
        std::lower_bound(m_visibleLineIndexVec.begin(), m_visibleLineIndexVec.end(), lineIndex);

    if (m_visibleLineIndexVec.end() != iter && lineIndex == *iter)
    {
        m_visibleLineIndexVec.erase(iter);
    }
}

// public
void
TextViewDocumentView::
showLineRange(const int firstLineIndex, const int lastLineIndexInclusive)
{
    Private::assertValidLineIndex(*this, firstLineIndex);
    Private::assertValidLineIndex(*this, lastLineIndexInclusive);
    assert(firstLineIndex <= lastLineIndexInclusive);

    const std::vector<int>::iterator lowerIter =
        std::lower_bound(m_visibleLineIndexVec.begin(), m_visibleLineIndexVec.end(), firstLineIndex);

    const std::vector<int>::const_iterator upperIter =
        std::upper_bound(m_visibleLineIndexVec.begin(), m_visibleLineIndexVec.end(), lastLineIndexInclusive);

    const int lineCount = lastLineIndexInclusive - firstLineIndex + 1;
    if ((upperIter - lowerIter) == lineCount && firstLineIndex == *lowerIter && lastLineIndexInclusive == (*upperIter - 1))
    {
        // All values exists -- nothing to insert.
        // @DebugBreakpoint
        int dummy = 1;
    }
    else if (lowerIter == upperIter)
    {
        CountingIterator<int> begin{firstLineIndex};
        CountingIterator<int> end{1 + lastLineIndexInclusive};
        m_visibleLineIndexVec.insert(lowerIter, begin, end);
    }
    else {
        int lineIndex = firstLineIndex;

        for (std::vector<int>::iterator iter = lowerIter;
             upperIter != iter;
             ++iter, ++lineIndex)
        {
            if (m_visibleLineIndexVec.end() == iter || lineIndex != *iter)
            {
                m_visibleLineIndexVec.insert(iter, lineIndex);
            }
        }
        assert(lastLineIndexInclusive == lineIndex);
    }
}

// public
void
TextViewDocumentView::
hideLineRange(const int firstLineIndex, const int lastLineIndexInclusive)
{
    Private::assertValidLineIndex(*this, firstLineIndex);
    Private::assertValidLineIndex(*this, lastLineIndexInclusive);
    assert(firstLineIndex <= lastLineIndexInclusive);

    const std::vector<int>::iterator lowerIter =
        std::lower_bound(m_visibleLineIndexVec.begin(), m_visibleLineIndexVec.end(), firstLineIndex);

    const std::vector<int>::const_iterator upperIter =
        std::upper_bound(m_visibleLineIndexVec.begin(), m_visibleLineIndexVec.end(), lastLineIndexInclusive);

    m_visibleLineIndexVec.erase(lowerIter, upperIter);
}

// public
int
TextViewDocumentView::
firstVisibleLineIndex()
const
{
    const int x = Algorithm::frontOrDefault(m_visibleLineIndexVec, 0);
    return x;
}

// public
int
TextViewDocumentView::
lastVisibleLineIndex()
const
{
    const int x = Algorithm::backOrDefault(m_visibleLineIndexVec, 0);
    return x;
}

// public
int
TextViewDocumentView::
nextVisibleLineIndex(const int lineIndex, const int stepCount)
const
{
    auto iter = Private::findOrAssert(*this, lineIndex);
    int adjStepCount = 0;
    if (stepCount < 0) {
        const int maxNegativeStepCount = static_cast<int>(m_visibleLineIndexVec.begin() - iter);
        adjStepCount = std::max(stepCount, maxNegativeStepCount);
    }
    else {
        const int maxPositiveStepCount = static_cast<int>(m_visibleLineIndexVec.end() - iter);
        adjStepCount = std::min(stepCount, maxPositiveStepCount);
    }
    // This is a rare use of operator[] on an iterator!  It is similar to: *(iter + adjStepCount)
    const int x = iter[adjStepCount];
    return x;
}

// public
int
TextViewDocumentView::
findNormalisedLineIndex(const int lineIndex)
const
{
    // Avoid binary search if possible. :)
    if (lineIndex < m_visibleLineIndexVec.size() && lineIndex == m_visibleLineIndexVec[lineIndex])
    {
        return lineIndex;
    }
    auto iter = Private::findOrAssert(*this, lineIndex);
    const int x = iter - m_visibleLineIndexVec.begin();
    return x;
}

// public
TextViewDocumentView::const_iterator
TextViewDocumentView::
findOrAssert(const int lineIndex)
const
{
    const auto iter = Private::findOrAssert(*this, lineIndex);
    return iter;
}

}  // namespace SDV
