//
// Created by kca on 1/5/2020.
//

#ifndef SDV_TREEITEM_H
#define SDV_TREEITEM_H

#include <QString>
#include <QVector>
#include "Constants.h"

namespace SDV {

class TreeItem
{
public:
    explicit TreeItem(JsonNodeType jsonNodeType, IsRichText isRichText, const QString& text, TreeItem* parent = nullptr)
        : m_jsonNodeTypes{jsonNodeType}, m_textFormat{isRichText.textFormat}, m_text{text}, m_parent{parent}
    {
        if (nullptr != parent) {
            parent->m_childVec.append(this);
        }
    }
    ~TreeItem();

    void appendChild(TreeItem* child);
    int rowIndex() const;
//    QVector<TreeItem*>& childVec() { return m_childVec; }
    const QVector<TreeItem*>& childVec() const { return m_childVec; }

    TreeItem* const m_parent;
    JsonNodeTypes m_jsonNodeTypes;
    const Qt::TextFormat m_textFormat;
    QString m_text;

private:
    QVector<TreeItem*> m_childVec;
};

}  // namespace SDV

#endif //SDV_TREEITEM_H
