//
// Created by kca on 10/6/2020.
//

#include "TextViewDocumentView.h"
#include "TextViewDocument.h"

namespace SDV {

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
    const std::size_t size = doc->textLineVec().size();
    for (std::size_t i = 0; i < size; ++i)
    {
        m_visibleLineIndexSet.insert(m_visibleLineIndexSet.end(), i);
    }
}

// public
void
TextViewDocumentView::
setVisible(const int lineIndex, const bool isVisible)
{
    assert(lineIndex >= 0 && lineIndex < m_doc->textLineVec().size());
    if (isVisible) {
        m_visibleLineIndexSet.insert(lineIndex);
    }
    else {
        m_visibleLineIndexSet.erase(lineIndex);
    }
}

// public
int
TextViewDocumentView::
prevLineIndex(const int lineIndex) const
{
    auto i = m_visibleLineIndexSet.find(lineIndex);
    assert(m_visibleLineIndexSet.end() != i);
    if (m_visibleLineIndexSet.begin() == i) {
        return lineIndex;
    }
    else {
        --i;
        const int x = *i;
        return x;
    }
}

// public
int
TextViewDocumentView::
nextLineIndex(const int lineIndex) const
{
    auto i = m_visibleLineIndexSet.find(lineIndex);
    assert(m_visibleLineIndexSet.end() != i);
    ++i;
    if (m_visibleLineIndexSet.end() == i) {
        return lineIndex;
    }
    else {
        const int x = *i;
        return x;
    }
}

}  // namespace SDV
