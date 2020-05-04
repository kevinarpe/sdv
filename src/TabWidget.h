//
// Created by kca on 4/4/2020.
//

#ifndef SDV_TABWIDGET_H
#define SDV_TABWIDGET_H

#include <QtWidgets>

namespace SDV {

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QWidget* parent = nullptr);

private:
    struct Private;
};

}  // namespace SDV

#endif //SDV_TABWIDGET_H
