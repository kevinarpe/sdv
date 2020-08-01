//
// Created by kca on 25/7/2020.
//

#ifndef SDV_TEXTVIEWJSONNODE_H
#define SDV_TEXTVIEWJSONNODE_H

#include <memory>
#include <cassert>
#include "Constants.h"
#include "TextViewPosition.h"

namespace SDV {

class JsonNode;
class TextViewPosition;

class TextViewJsonNode
{
private:
    class Private
    {
    private:
        Private() = default;
        friend class TextViewJsonNode;
    };
public:
    static std::shared_ptr<TextViewJsonNode>
    createRoot(JsonNodeType type,
               const QString& text,
               const TextViewPosition& pos);

    // Effectively private ctor, but must have public access for std::make_shared(...)
    // Ref: https://github.com/isocpp/CppCoreGuidelines/issues/1205
    // Ref: https://stackoverflow.com/a/8147326/257299
    TextViewJsonNode(const Private& dummy,
                     JsonNodeType type,
                     const QString& text,
                     const TextViewPosition& pos)
        : m_nullableParent{nullptr}, m_type{type}, m_text{text}, m_pos{pos}, m_arrayIndex{-1}
    {}

    bool isRoot() const { return (nullptr != m_nullableParent); }
    /** {@code nullptr} when {@link #isRoot()} is {@code true} */
    TextViewJsonNode* nullableParent() const { return m_nullableParent; }
    JsonNodeType type() const { return m_type; }
    const QString& text() const { return m_text; }
    const TextViewPosition& pos() const { return m_pos; }
    /** -1 if parent is not {@link JsonNodeType::ArrayBegin} */
    int arrayIndex() const { return m_arrayIndex; }
    const std::vector<std::shared_ptr<TextViewJsonNode>>& childVec() const { return m_childVec; }

    std::shared_ptr<TextViewJsonNode>
    addChild(JsonNodeType type,
             const QString& text,
             const TextViewPosition& pos);

    int compare(const TextViewPosition& pos);

private:
    TextViewJsonNode* m_nullableParent;
    JsonNodeType m_type;
    QString m_text;
    TextViewPosition m_pos;
    int m_arrayIndex;
    std::vector<std::shared_ptr<TextViewJsonNode>> m_childVec;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWJSONNODE_H
