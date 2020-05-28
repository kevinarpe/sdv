//
// Created by kca on 30/4/2020.
//

#include "TreeModel.h"
#include <QBrush>
#include <QDebug>
#include <QAbstractItemView>
#include <QLabel>
#include "TreeItem.h"

namespace SDV {

struct TreeModel::Private
{
    static TreeItem*
    getItem(const TreeModel& self,
            const int rowIndex,
            const int columnIndex,
            const QModelIndex& parent)
    {
        assert(rowIndex >= 0 && columnIndex >= 0);
        if (parent.isValid()) {
            TreeItem* const parentItem = static_cast<TreeItem*>(parent.internalPointer());
            if (rowIndex < parentItem->childVec().size()) {
                TreeItem* const childItem = parentItem->childVec()[rowIndex];
                return childItem;
            }
            else {
                return nullptr;
            }
        }
        else {
            if (rowIndex < self.m_rootVec.size()) {
                TreeItem* const x = self.m_rootVec[rowIndex];
                return x;
            }
            else {
                return nullptr;
            }
        }
    }

    static void
    setIndexWidget(const TreeModel& self,
                   QAbstractItemView* widget,
//                   TreeItem* const nullableParentItem,
                   TreeItem* const childItem,
                   const int rowIndex)
    {
//        const QModelIndex& modelIndex = self.createIndex(rowIndex, 0, nullableParentItem);
        const QModelIndex& modelIndex = self.createIndex(rowIndex, 0, childItem);
        QLabel* const label = new QLabel{widget};
        label->setTextFormat(childItem->m_textFormat);
        label->setText(childItem->m_text);
        widget->setIndexWidget(modelIndex, label);
        const QVector<TreeItem*>& childVec = childItem->childVec();
        const int size = childVec.size();
        for (int childRowIndex = 0; childRowIndex < size; ++childRowIndex) {
            TreeItem* const& childItem2 = childVec[childRowIndex];
            setIndexWidget(self, widget, /*childItem, */childItem2, childRowIndex);
        }
    }
};

// public explicit
TreeModel::
TreeModel(QVector<TreeItem*> rootVec, QObject* parent /*= nullptr*/)
    : Base{parent}, m_rootVec{rootVec}
{
    // If root element object or array, size is two (['{', '}'] or ['[', ']']).  Else size is one.
    assert(1 == m_rootVec.size() || 2 == m_rootVec.size());
}

// public
TreeModel::
~TreeModel()
{
    qDebug() << "~TreeModel()";
    qDeleteAll(m_rootVec);
}

// public
QModelIndex
TreeModel::
index(const int rowIndex,
      const int columnIndex,
      const QModelIndex& parent) const // override
{
    TreeItem* const item = Private::getItem(*this, rowIndex, columnIndex, parent);
    if (nullptr == item) {
        return QModelIndex{};
    }
    else {
        const QModelIndex& x = Base::createIndex(rowIndex, columnIndex, item);
        return x;
    }
}

// public
QModelIndex
TreeModel::
parent(const QModelIndex& child) const // override
{
    if ( ! child.isValid()) {
        return QModelIndex();
    }
    TreeItem* const childItem = static_cast<TreeItem*>(child.internalPointer());
    if (nullptr == childItem->m_parent) {
        return QModelIndex();
    }
    const int rowIndex = childItem->m_parent->rowIndex();
    // Ref: https://doc.qt.io/qt-5/qabstractitemmodel.html#parent
    // A common convention used in models that expose tree data structures is that only items in the first column have children.
    // For that case, when reimplementing this function in a subclass the column of the returned QModelIndex would be 0.
    const QModelIndex& x = Base::createIndex(rowIndex, 0, childItem->m_parent);
    return x;
}

// public
int
TreeModel::
rowCount(const QModelIndex& parent) const // override
{
    if (parent.column() > 0) {
        qDebug() << "parent.column(): " << parent.column();
    }
    if (parent.isValid()) {
        TreeItem* const parentItem = static_cast<TreeItem*>(parent.internalPointer());
        const int x = parentItem->childVec().size();
        return x;
    }
    else {
        const int x = m_rootVec.size();
        return x;
    }
}

/**
 * @param role
 *        See: {@link Qt::ItemDataRole}
 */
// public
QVariant
TreeModel::
data(const QModelIndex& index, const int role) const // override
{
//    if ( ! index.isValid()) {
//        return QVariant();
//    }
//    else if (Qt::ItemDataRole::DisplayRole == role) {
//        TreeItem* const item = static_cast<TreeItem*>(index.internalPointer());
//        return item->m_text;
//    }
    // Ref: https://forum.qt.io/topic/68802/custom-role-for-a-custom-model
//    else if (Qt::ItemDataRole::ForegroundRole == role) {
//        return QBrush{Qt::GlobalColor::darkGreen};
//    }
//    else {
        return QVariant();
//    }
}

// public
void
TreeModel::
setIndexWidgets(QAbstractItemView* widget)
const
{
    const int size = m_rootVec.size();
    for (int rowIndex = 0; rowIndex < size; ++rowIndex) {
        TreeItem* const item = m_rootVec[rowIndex];
        Private::setIndexWidget(*this, widget, /*nullptr, */item, rowIndex);
    }
}

}  // namespace SDV
