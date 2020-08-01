//
// Created by kca on 25/7/2020.
//

#include "TextViewJsonNode.h"
#include "TextViewPosition.h"

namespace SDV {

// public static
std::shared_ptr<TextViewJsonNode>
TextViewJsonNode::
createRoot(JsonNodeType type,
           const QString& text,
           const TextViewPosition& pos)
{
    std::shared_ptr<TextViewJsonNode> x = std::make_shared<TextViewJsonNode>(Private{}, type, text, pos);
    return x;
}

// public
std::shared_ptr<TextViewJsonNode>
TextViewJsonNode::
addChild(JsonNodeType type,
         const QString& text,
         const TextViewPosition& pos)
{
    std::shared_ptr<TextViewJsonNode> child = std::make_shared<TextViewJsonNode>(Private{}, type, text, pos);
    child->m_nullableParent = this;
    if (m_type == JsonNodeType::ArrayBegin)
    {
        child->m_arrayIndex = m_childVec.size();
    }
    m_childVec.push_back(child);
    return child;
}

// public
int
TextViewJsonNode::
compare(const TextViewPosition& pos)
{
    if (m_pos.lineIndex < pos.lineIndex) {
        return -1;
    }
    if (m_pos.lineIndex > pos.lineIndex) {
        return +1;
    }
    if (m_pos.charIndex > pos.charIndex) {
        return +1;
    }
    // else if (jsonNode->pos().lineIndex == pos.lineIndex)
    const int endCharIndex = m_pos.charIndex + m_text.length();
    if (endCharIndex <= pos.charIndex) {
        return -1;
    }
    return 0;
}

}  // namespace SDV
