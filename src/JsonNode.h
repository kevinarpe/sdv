//
// Created by kca on 30/5/2020.
//

#ifndef SDV_JSONNODE_H
#define SDV_JSONNODE_H

#include <optional>
#include <QString>
#include <QVector>
#include "Constants.h"

namespace SDV {

class JsonNode
{
public:
    JsonNode(std::optional<JsonNode*> optionalParent, JsonNodeType type, const QString& text);
    ~JsonNode();

    std::optional<JsonNode*> optionalParent() const { return m_optionalParent; }
    JsonNodeType type() const { return m_type; }
    const QString& text() const { return m_text; }
    const QVector<JsonNode*> childVec() const { return m_childVec; }

private:
    // TODO: Convert to <const JsonNode*>?
    // Ref: https://stackoverflow.com/questions/55060223/is-the-content-of-a-const-stdoptional-always-const
    std::optional<JsonNode*> m_optionalParent;
    JsonNodeType m_type;
    // TODO: std::optional?  object/array begin is dumb and can be cached!  Maybe QString sharing is good enough.
    QString m_text;
    QVector<JsonNode*> m_childVec;
};

}  // namespace SDV

#endif //SDV_JSONNODE_H
