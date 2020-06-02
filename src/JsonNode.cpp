//
// Created by kca on 30/5/2020.
//

#include "JsonNode.h"

namespace SDV {

// public
JsonNode::
JsonNode(std::optional<JsonNode*> optionalParent, JsonNodeType type, const QString& text)
    : m_optionalParent{optionalParent}, m_type{type}, m_text{text}
{
    if (optionalParent) {
        (*optionalParent)->m_childVec.append(this);
    }
}

// public
JsonNode::
~JsonNode()
{
    qDeleteAll(m_childVec);
}

}  // namespace SDV
