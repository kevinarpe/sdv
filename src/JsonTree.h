//
// Created by kca on 31/5/2020.
//

#ifndef SDV_JSONTREE_H
#define SDV_JSONTREE_H

#include <memory>
#include <QString>
#include <QTextLayout>
#include "Constants.h"
#include "LineSegment.h"
#include "TextViewPosition.h"

namespace SDV {

class JsonNode;

// Ref: https://stackoverflow.com/questions/4819936/why-no-default-move-assignment-move-constructor
// Ref: https://stackoverflow.com/questions/18290523/is-a-default-move-constructor-equivalent-to-a-member-wise-move-constructor
// Ref: https://i.stack.imgur.com/C2EUm.png
struct JsonTree
{
    // TODO: Remove later...
    QString jsonText;
    std::shared_ptr<JsonNode> rootNode;
    /** All nodes are owned indirectly by {@link #m_rootNode}. */
    std::unordered_map<JsonNode*, TextViewPosition> nodeToPosMap;
    /** All nodes are owned indirectly by {@link #m_rootNode}. */
    std::vector<std::vector<JsonNode*>> lineIndex_To_NodeVec;
};

}  // namespace SDV

#endif //SDV_JSONTREE_H
