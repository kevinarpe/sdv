//
// Created by kca on 19/4/2020.
//

#ifndef SDV_GOTOWIDGET_H
#define SDV_GOTOWIDGET_H

#include <QWidget>
#include "FindLineEdit.h"
class QComboBox;
class QLabel;

namespace SDV {

class GoToWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GoToWidget(QWidget* parent = nullptr);
    FindLineEdit* findLineEdit() const { return m_findLineEdit; }

signals:
    void signalSpecialKeyPressed(FindLineEdit::KeySequence keySequence);
    void signalGoToLineColumn(int lineNumber, int columnNumber);
    void signalGoToOffset(int offset);
    void signalHidden();

private:
    struct Private;
    QComboBox* m_comboBox;
    FindLineEdit* m_findLineEdit;
    QLabel* m_rangeLabel;
};

}  // namespace SDV

#endif //SDV_GOTOWIDGET_H
