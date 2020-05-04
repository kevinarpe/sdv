// Ref: https://code.qt.io/cgit/qt/qtbase.git/tree/src/widgets/util/qsystemtrayicon_p.h?h=5.12.5
/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SDV_BALLOONTIP_H
#define SDV_BALLOONTIP_H

#include <QWidget>

namespace SDV {

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
