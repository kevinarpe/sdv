//
// Created by kca on 19/4/2020.
//

#ifndef SDV_GOTOWIDGET_H
#define SDV_GOTOWIDGET_H

#include "SmoothShowHideWidget.h"
#include "FindLineEdit.h"
class QComboBox;
class QLabel;

namespace SDV {

class GoToWidget : public SmoothShowHideWidget
{
    Q_OBJECT

public:
    using Base = SmoothShowHideWidget;
    explicit GoToWidget(QWidget* parent = nullptr);
    ~GoToWidget() override = default;
    FindLineEdit* findLineEdit() const { return m_findLineEdit; }

signals:
    void signalSpecialKeyPressed(FindLineEdit::KeySequence keySequence);
    /**
     * @param lineNumber
     *        one-based, e.g., 1, 2, 3...
     *        <br>-1 if missing
     *
     * @param columnNumber
     *        one-based, e.g., 1, 2, 3...
     *        <br>-1 if missing
     */
    void signalGoToLineColumn(int lineNumber, int columnNumber);
    /**
     * @param offset
     *        zero-based, e.g., 0, 1, 2...
     *        <br>-1 if missing
     */
    void signalGoToOffset(int offset);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool event(QEvent* event) override;

private:
    struct Private;
    QComboBox* m_comboBox;
    FindLineEdit* m_findLineEdit;
    QLabel* m_rangeLabel;
};

}  // namespace SDV

#endif //SDV_GOTOWIDGET_H
