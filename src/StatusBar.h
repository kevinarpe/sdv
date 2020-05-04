//
// Created by kca on 19/4/2020.
//

#ifndef SDV_STATUSBAR_H
#define SDV_STATUSBAR_H

#include <QStatusBar>
class QLabel;

namespace SDV {

class StatusBar : public QStatusBar
{
    Q_OBJECT

public:
    explicit StatusBar(QWidget* parent = nullptr);
    QLabel* sharedLabel() const { return m_sharedLabel; }
    QLabel* textViewLabel() const { return m_textViewLabel; }

private:
    using Base = QStatusBar;
    QLabel* m_sharedLabel;
    QLabel* m_textViewLabel;
};

}  // namespace SDV

#endif //SDV_STATUSBAR_H
