//
// Created by kca on 28/7/2020.
//

#ifndef SDV_BASESTATUSBAR_H
#define SDV_BASESTATUSBAR_H

#include <QStatusBar>
class QSizeGrip;

namespace SDV {

/**
 * This is a simple, separate base class to (1) remove the layout (and all managed widgets), and (2) add a QSizeGrip where we have full
 * control.  And why is base class QStatusBar required?  See: QMainWindow::setStatusBar(QStatusBar*).  Huff!  Why not use QWidget!?  :(
 *
 * Ref: https://stackoverflow.com/questions/63102012/how-to-align-two-widgets-in-a-qhboxlayout-where-one-is-aligned-far-left-and-one
 */
class BaseStatusBar : public QStatusBar
{
Q_OBJECT

public:
    using Base = QStatusBar;

    explicit BaseStatusBar(QWidget* parent = nullptr);
    ~BaseStatusBar() override = default;

    QSizeGrip* sizeGrip() const { return m_sizeGrip; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QSizeGrip* m_sizeGrip;
};

}  // namespace SDV

#endif //SDV_BASESTATUSBAR_H
