//
// Created by kca on 14/4/2020.
//

#ifndef SDV_BALLOONTIP_H
#define SDV_BALLOONTIP_H

#include <QWidget>

namespace SDV {

// Borrowed from lib/qtbase-opensource-src-5.12.5+dfsg/src/widgets/util/qsystemtrayicon_p.h
class BalloonTip : public QWidget
{
    Q_OBJECT
public:
    static void showBalloon(const QIcon &icon, const QString &title, const QString &msg,
                            const QPoint &pos, int timeoutMillis, bool showArrow = true);
    static void hideBalloon();
    static bool isBalloonVisible();
    static void updateBalloonPosition(const QPoint& pos);

private:
    BalloonTip(const QIcon &icon, const QString &title, const QString &msg);
    ~BalloonTip();
    void balloon(const QPoint&, int, bool);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void timerEvent(QTimerEvent *e) override;

private:
    QPixmap pixmap;
    int timerId;
    bool showArrow;
};

}  // namespace SDV

#endif //SDV_BALLOONTIP_H
