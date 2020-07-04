//
// Created by kca on 30/5/2020.
//

#include "JsonNode.h"

namespace SDV {

// public
JsonNode::
JsonNode(JsonNode* nullableParent, JsonNodeType type, const QString& text)
    : m_nullableParent{nullableParent}, m_type{type}, m_text{text}
{
    if (nullptr != nullableParent) {
        nullableParent->m_childVec.push_back(this);
    }
}

// public
JsonNode::
~JsonNode()
{
    for (JsonNode* n : m_childVec) {
        delete n;
    }
}

}  // namespace SDV
