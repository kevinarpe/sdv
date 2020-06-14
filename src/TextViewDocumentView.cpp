//
// Created by kca on 10/6/2020.
//

#include "TextViewDocumentView.h"
#include <numeric>
#include <algorithm>
#include "TextViewDocument.h"
#include "Algorithm.h"

namespace SDV {

struct TextViewDocumentView::Private
{
    static std::vector<int>::iterator
    find(TextViewDocumentView& self, const int lineIndex)
    {
        std::vector<int>::iterator iter =
            std::lower_bound(self.m_visibleLineIndexVec.begin(), self.m_visibleLineIndexVec.end(), lineIndex);

        assert(self.m_visibleLineIndexVec.end() != iter);
        return iter;
    }

    static std::vector<int>::const_iterator
    cfind(const TextViewDocumentView& self, const int lineIndex)
    {
        std::vector<int>::const_iterator iter =
            std::lower_bound(self.m_visibleLineIndexVec.begin(), self.m_visibleLineIndexVec.end(), lineIndex);

        assert(self.m_visibleLineIndexVec.end() != iter);
        return iter;
    }
};

// public
TextViewDocumentView::
TextViewDocumentView()
    : m_doc{TextViewDocument::staticEmpty()}
{}

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
setRangeVisible(const int firstVisibleLineIndexInclusive, const int lastVisibleLineIndexInclusive, const bool isVisible)
{
    assert(firstVisibleLineIndexInclusive <= lastVisibleLineIndexInclusive);
    auto first = Private::find(*this, firstVisibleLineIndexInclusive);
    auto last = Private::find(*this, lastVisibleLineIndexInclusive);
    std::fill(first, last, isVisible);
    // TODO: How to notify observers of this change!?
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
    auto iter = Private::cfind(*this, lineIndex);
    int adjStepCount = 0;
    if (stepCount < 0) {
        const int maxNegativeStepCount = static_cast<int>(m_visibleLineIndexVec.begin() - iter);
        adjStepCount = std::max(stepCount, maxNegativeStepCount);
    }
    else {
        const int maxPositiveStepCount = static_cast<int>(m_visibleLineIndexVec.end() - iter);
        adjStepCount = std::min(stepCount, maxPositiveStepCount);
    }
    const int x = iter[adjStepCount];
    return x;
}

// public
int
TextViewDocumentView::
findNormalisedLineIndex(const int lineIndex)
const
{
    auto iter = Private::cfind(*this, lineIndex);
    const int x = iter - m_visibleLineIndexVec.begin();
    return x;
}

// public
TextViewDocumentView::const_iterator
TextViewDocumentView::
find(const int lineIndex)
const
{
    auto iter = Private::cfind(*this, lineIndex);
    return iter;
}

}  // namespace SDV
