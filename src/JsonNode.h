//
// Created by kca on 30/5/2020.
//

#ifndef SDV_JSONNODE_H
#define SDV_JSONNODE_H

#include <vector>
#include <QString>
#include "Constants.h"

namespace SDV {

class JsonNode
{
public:
    JsonNode(JsonNode* nullableParent, JsonNodeType type, const QString& text);
    ~JsonNode();

    JsonNode* nullableParent() const { return m_nullableParent; }
    JsonNodeType type() const { return m_type; }
    const QString& text() const { return m_text; }
    const std::vector<JsonNode*>& childVec() const { return m_childVec; }

private:
    JsonNode* m_nullableParent;
    JsonNodeType m_type;
    QString m_text;
    std::vector<JsonNode*> m_childVec;
};

}  // namespace SDV

#endif //SDV_JSONNODE_H
