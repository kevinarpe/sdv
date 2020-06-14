//
// Created by kca on 5/6/2020.
//

#ifndef SDV_TEXTVIEWDOCUMENT_H
#define SDV_TEXTVIEWDOCUMENT_H

#include <vector>
#include <memory>
#include <QString>

namespace SDV {

class TextViewDocument
{
public:
    static const std::shared_ptr<TextViewDocument>& staticEmpty() { return s_empty; }

    explicit TextViewDocument(std::vector<QString>&& lineVec)
        : m_lineVec{lineVec}
    {}

    const std::vector<QString>& lineVec() const { return m_lineVec; }

private:
    struct Private;

    static std::shared_ptr<TextViewDocument> s_empty;

    TextViewDocument() = default;

    // more efficient to store a single QString, then do substrings.
    // keep impl private, but expose public interface as necessary.
    /** Zero or more text blocks of one or more lines of text. */
    std::vector<QString> m_lineVec;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWDOCUMENT_H
