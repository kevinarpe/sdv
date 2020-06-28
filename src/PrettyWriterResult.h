//
// Created by kca on 31/5/2020.
//

#ifndef SDV_PRETTYWRITERRESULT_H
#define SDV_PRETTYWRITERRESULT_H

#include <memory>
#include <QString>
#include <QTextLayout>
#include "Constants.h"
#include "LineSegment.h"

namespace SDV {

class JsonNode;

// Ref: https://stackoverflow.com/questions/4819936/why-no-default-move-assignment-move-constructor
// Ref: https://stackoverflow.com/questions/18290523/is-a-default-move-constructor-equivalent-to-a-member-wise-move-constructor
// Ref: https://i.stack.imgur.com/C2EUm.png
struct PrettyWriterResult
{
    struct Pos
    {
        int lineIndex = -1;
        int charIndex = -1;
    };
    QString jsonText;
    // TODO: Remove later...
    QVector<QTextLayout::FormatRange> formatRangeVec;
    std::shared_ptr<JsonNode> rootNode;
    /** All nodes are owned indirectly by {@link #m_rootNode}. */
    QMap<JsonNode*, Pos> nodeToPosMap;
    /** All nodes are owned indirectly by {@link #m_rootNode}. */
    QVector<QVector<JsonNode*>> lineIndex_To_NodeVec;

    struct JsonNodeLineSegment
    {
        JsonNodeType jsonNodeType;
        int lineIndex;
        LineSegment seg;
    };
    std::vector<JsonNodeLineSegment> jsonNodeLineSegmentVec;
};

}  // namespace SDV

#endif //SDV_PRETTYWRITERRESULT_H
