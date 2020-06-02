//
// Created by kca on 31/5/2020.
//

#ifndef SDV_PRETTYWRITERRESULT_H
#define SDV_PRETTYWRITERRESULT_H

#include <memory>
#include <QString>
#include <QTextLayout>

namespace SDV {

class JsonNode;

// Ref: https://stackoverflow.com/questions/4819936/why-no-default-move-assignment-move-constructor
// Ref: https://stackoverflow.com/questions/18290523/is-a-default-move-constructor-equivalent-to-a-member-wise-move-constructor
// Ref: https://i.stack.imgur.com/C2EUm.png
struct PrettyWriterResult
{
    struct Pos {
        int lineIndex = -1;
        int charIndex = -1;
    };
    QString m_jsonText;
    QVector<QTextLayout::FormatRange> m_formatRangeVec;
    std::shared_ptr<JsonNode> m_rootNode;
    /** All nodes are owned indirectly by {@link #m_rootNode}. */
    QMap<JsonNode*, Pos> m_nodeToPosMap;
    /** All nodes are owned indirectly by {@link #m_rootNode}. */
    QVector<QVector<JsonNode*>> m_lineIndex_To_NodeVec;
};

}  // namespace SDV

#endif //SDV_PRETTYWRITERRESULT_H
