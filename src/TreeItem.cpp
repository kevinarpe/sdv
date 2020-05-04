//
// Created by kca on 1/5/2020.
//

#include "TreeItem.h"
#include <QDebug>

namespace SDV {

// public
TreeItem::
~TreeItem()
{
    qDebug() << "~TreeItem()";
    qDeleteAll(m_childVec);
}

// public
void
TreeItem::
appendChild(TreeItem* child)
{
    assert(nullptr != child);
    m_childVec.append(child);
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
