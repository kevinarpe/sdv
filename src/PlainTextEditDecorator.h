//
// Created by kca on 31/5/2020.
//

#ifndef SDV_PLAINTEXTEDITDECORATOR_H
#define SDV_PLAINTEXTEDITDECORATOR_H

#include <QObject>
class QLabel;

// TODO: DELETE ME???
namespace SDV {

class PlainTextEdit;
class JsonNode;
class TreeNodeExpander;

class PlainTextEditDecorator : public QObject
{
    Q_OBJECT

public:
    using Base = QObject;
    PlainTextEditDecorator(PlainTextEdit& parent);
    ~PlainTextEditDecorator() override = default;

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct Private;
    // Intentional: non-const reference
    PlainTextEdit& m_parent;
    bool m_updateBeforeShow;
    struct TreeNode
    {
        JsonNode* m_jsonNode;
        TreeNodeExpander* m_expander;
        QMetaObject::Connection m_expanderConnection;
        QLabel* m_sizeLabel;
    };
    std::vector<TreeNode> m_freeTreeNodeVec;
    std::vector<TreeNode> m_usedTreeNodeVec;
    qreal m_textBlockMinHeight;
    std::unordered_map<JsonNode*, bool> m_jsonNode_To_IsExpanded_Map;
};

}  // namespace SDV

#endif //SDV_PLAINTEXTEDITDECORATOR_H
