//
// Created by kca on 10/6/2020.
//

#ifndef SDV_TEXTVIEWDOCUMENTVIEW_H
#define SDV_TEXTVIEWDOCUMENTVIEW_H

#include <memory>
#include <set>
#include <vector>

namespace SDV {

class TextViewDocument;

class TextViewDocumentView
{
public:
    using const_iterator = std::vector<int>::const_iterator;

    TextViewDocumentView();

    const TextViewDocument& doc() const;
    void setDoc(const std::shared_ptr<TextViewDocument>& doc);
    void setRangeVisible(int firstVisibleLineIndexInclusive, int lastVisibleLineIndexInclusive, bool isVisible);

    std::size_t visibleLineCount() { return m_visibleLineIndexVec.size(); }
    const_iterator visibleLineBegin() const { return m_visibleLineIndexVec.begin(); }
    const_iterator visibleLineEnd() const { return m_visibleLineIndexVec.end(); }

    int firstVisibleLineIndex() const;
    int lastVisibleLineIndex() const;
    int nextVisibleLineIndex(int lineIndex, int stepCount) const;

    /**
     * Maps visible line index to normalised line index -- used by vertical scroll bar.
     *
     * Example: Assume there are five lines, and lines at indices 1 and 2 are hidden.
     * The normalised line index mappings will be 0->0, 3->1, 4->2.
     */
    int findNormalisedLineIndex(int lineIndex) const;

    const_iterator find(int lineIndex) const;

private:
    struct Private;
    std::shared_ptr<TextViewDocument> m_doc;
    // Intentional: Yes, sorted set (std::set) has more indirection and uses more memory (due to nodes) than flat map,
    // but we need high(er) performance inserts.  Flat maps have poor insert performance.
    // Ref: https://medium.com/@gaussnoise/why-standardizing-flat-map-is-a-bad-idea-7efb59fe6cea
//    std::set<int> m_visibleLineIndexSet;

    /**
     * Maps normalised line index (used by vertical scroll bar) to visible line index.
     *
     * This is a flat (ordered) set where members must be naturally ordered (smallest to largest) for binary search.
     * Normally, flat (ordered) sets have slower insert performance relative to node-based ordered sets.
     * However, this problem is avoided via restricted write access with method setRangeVisible(int, int, bool).
     */
    std::vector<int> m_visibleLineIndexVec;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWDOCUMENTVIEW_H
