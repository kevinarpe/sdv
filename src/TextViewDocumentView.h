//
// Created by kca on 10/6/2020.
//

#ifndef SDV_TEXTVIEWDOCUMENTVIEW_H
#define SDV_TEXTVIEWDOCUMENTVIEW_H

#include <memory>
#include <set>

namespace SDV {

class TextViewDocument;

class TextViewDocumentView
{
public:
    TextViewDocumentView();

    const TextViewDocument& doc() const;
    void setDoc(const std::shared_ptr<TextViewDocument>& doc);
    void setVisible(int lineIndex, bool isVisible);
    const std::set<int>& visibleLineIndexSet() { return m_visibleLineIndexSet; }
    int prevLineIndex(int lineIndex) const;
    int nextLineIndex(int lineIndex) const;

private:
    std::shared_ptr<TextViewDocument> m_doc;
    // Intentional: Yes, sorted set (std::set) has more indirection and uses more memory (due to nodes) than flat map,
    // but we need high(er) performance inserts.  Flat maps have poor insert performance.
    // Ref: https://medium.com/@gaussnoise/why-standardizing-flat-map-is-a-bad-idea-7efb59fe6cea
    std::set<int> m_visibleLineIndexSet;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWDOCUMENTVIEW_H
