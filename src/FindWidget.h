//
// Created by kca on 11/4/2020.
//

#ifndef SDV_FINDWIDGET_H
#define SDV_FINDWIDGET_H

#include "SmoothShowHideWidget.h"
#include <QRegularExpression>
#include "FindLineEdit.h"
class QLabel;
class QToolButton;
class QCheckBox;

namespace SDV {

class FindWidget : public SmoothShowHideWidget
{
    Q_OBJECT

public:
    using Base = SmoothShowHideWidget;

    explicit FindWidget(QWidget* parent = nullptr);
    ~FindWidget() override = default;

    FindLineEdit* findLineEdit() const { return m_findLineEdit; }
    QLabel* matchCountLabel() const { return m_matchCountLabel; }

signals:
    void signalQueryChanged(const QString& text, bool isCaseSensitive, bool isRegex);
    void signalNextMatch();
    void signalPrevMatch();
    void signalSpecialKeyPressed(FindLineEdit::KeySequence keySequence);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    bool event(QEvent* event) override;

private:
    struct Private;
    FindLineEdit* m_findLineEdit;
    QLabel* m_matchCountLabel;
    QToolButton* m_nextMatchToolButton;
    QToolButton* m_prevMatchToolButton;
    QCheckBox* m_matchCaseCheckBox;
    QCheckBox* m_regexCheckBox;
    QRegularExpression m_regex;
};

}  // namespace SDV

#endif //SDV_FINDWIDGET_H
