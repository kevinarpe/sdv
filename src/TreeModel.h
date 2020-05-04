//
// Created by kca on 30/4/2020.
//

#ifndef SDV_TREEMODEL_H
#define SDV_TREEMODEL_H

#include <memory>
#include <QAbstractItemModel>
#include "TreeItem.h"
class QAbstractItemView;

namespace SDV {

//class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    using Base = QAbstractItemModel;
    explicit TreeModel(QVector<TreeItem*> rootVec, QObject* parent = nullptr);
    ~TreeModel();
//
//    QVariant headerData(const int sectionIndex, Qt::Orientation orientation, const int role) const override
//    {
//        if (Qt::Orientation::Horizontal != orientation || Qt::ItemDataRole::DisplayRole != role) {
//            return QVariant{};
//        }
//        switch (sectionIndex)
//        {
//            case 0:
////                qDebug() << "headerData(): " << m_root->m_text;
////                m_root->m_text;
//                qDebug() << "headerData()?";
//            default:
//                return QVariant{};
//        }
//    }

    QModelIndex index(const int rowIndex, const int columnIndex, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override { return 1; }
    QVariant data(const QModelIndex& index, const int role) const override;
    const QVector<TreeItem*>& rootVec() { return m_rootVec; }
    void setIndexWidgets(QAbstractItemView* widget) const;

private:
    struct Private;
    /** If root is object or array, size is two.  Else size is one. */
    const QVector<TreeItem*> m_rootVec;
};

}  // namespace SDV

#endif //SDV_TREEMODEL_H
