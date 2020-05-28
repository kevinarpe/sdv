//
// Created by kca on 1/5/2020.
//

#include "TreeItem.h"
#include <QDebug>

namespace SDV {

// public explicit
TreeItem::
TreeItem(JsonNodeType jsonNodeType, IsRichText isRichText, const QString& text, TreeItem* parent /*= nullptr*/)
    : m_jsonNodeTypes{jsonNodeType}, m_textFormat{isRichText.textFormat}, m_text{text}, m_parent{parent}
{
    if (nullptr != parent) {
        parent->m_childVec.append(this);
    }
}

// public
TreeItem::
~TreeItem()
{
//    qDebug() << "~TreeItem()";
    qDeleteAll(m_childVec);
}

// public
int
TreeItem::
rowIndex()
const
{
    if (nullptr == m_parent) {
        return 0;
    }
    const int x = m_parent->m_childVec.indexOf(const_cast<TreeItem*>(this));
//    qDebug() << "rowIndex(): " << x;
    return x;
}

}  // namespace SDV
