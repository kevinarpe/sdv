//
// Created by kca on 19/4/2020.
//

#ifndef SDV_MAINWINDOWSTATUSBAR_H
#define SDV_MAINWINDOWSTATUSBAR_H

#include "BaseStatusBar.h"
class QLabel;

namespace SDV {

class MainWindowStatusBar : public BaseStatusBar
{
    Q_OBJECT

public:
    using Base = BaseStatusBar;

    explicit MainWindowStatusBar(QWidget* parent = nullptr);
    ~MainWindowStatusBar() override = default;

    QLabel* nodePathLabel() const { return m_nodePathLabel; }
    QLabel* textStatsLabel() const { return m_textStatsLabel; }

public slots:
    void slotSetTextCursorPosition(int lineIndex, int columnIndex);

private:
    QLabel* m_nodePathLabel;
    QLabel* m_textCursorPositionLabel;
    QLabel* m_textStatsLabel;
};

}  // namespace SDV

#endif //SDV_MAINWINDOWSTATUSBAR_H
