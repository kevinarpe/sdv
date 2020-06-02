//
// Created by kca on 31/5/2020.
//

#ifndef SDV_PLAINTEXTEDITDECORATOR_H
#define SDV_PLAINTEXTEDITDECORATOR_H

#include <QObject>
#include "PrettyWriterResult.h"
class QLabel;

namespace SDV {

class PlainTextEdit;
class JsonNode;
class TreeNodeExpanderWidget;

class PlainTextEditDecorator : public QObject
{
    Q_OBJECT

public:
    using Base = QObject;
    PlainTextEditDecorator(PlainTextEdit& action);
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
        TreeNodeExpanderWidget* m_expander;
        QLabel* m_sizeLabel;
    };
    std::vector<TreeNode> m_freeTreeNodeVec;
    std::vector<TreeNode> m_usedTreeNodeVec;
    qreal m_textBlockMinHeight;
};

}  // namespace SDV

#endif //SDV_PLAINTEXTEDITDECORATOR_H
