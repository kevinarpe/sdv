//
// Created by kca on 21/4/2020.
//

#ifndef SDV_CIRCLEWIDGET_H
#define SDV_CIRCLEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QDebug>

namespace SDV {

class CircleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CircleWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
        : QWidget{parent, f}
    {}
//
//    QPaintEngine* paintEngine() const override
//    {
//        return QWidget::paintEngine();
//    }

protected:
//    void resizeEvent(QResizeEvent* event) override
//    {
//        qDebug() << "resizeEvent()";
//        QWidget::resizeEvent(event);
//    }
//
//    LAST: FINISH THE DONUT!
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::RenderHint::Antialiasing);
        const int w = width();
        const int h = height();
//        painter.translate(w / 2, h / 2);
        const int penWidth = 10;
//        const int alpha = 256 / 3;
        const int alpha = 255;
        painter.setPen(QPen{QBrush{QColor{137, 207, 240, alpha}}, penWidth});
//        painter.setPen(QPen{QBrush{QColor{0, 0, 255, 64}}, penWidth});
//        painter.setBrush(QBrush{QColor{137, 207, 240, 64}});
//        painter.drawEllipse(QRect{-((w - penWidth)/ 2), -((h - penWidth) / 2), w - penWidth, h - penWidth});

        painter.drawEllipse(QRect{penWidth / 2, penWidth / 2, w - penWidth, h - penWidth});
//        QColor{Qt::GlobalColor::darkBlue};
        painter.setPen(QPen{QBrush{QColor{0, 0, 255, alpha / 2}}, 1});
        painter.drawEllipse(QRect{0, 0, w - 1, h - 1});
        painter.drawEllipse(QRect{penWidth, penWidth, w - 2 * penWidth, h - 2 * penWidth});

//        painter.drawEllipse(QRect{0, 0, w, h});
//        QPainterPath pp{};
//        pp.addEllipse(QRect{-width() / 2, -height() / 2, width(), height()});
//        pp.addEllipse(QRect{-width() / 4, -height() / 4, width(), height()});
//        painter.drawPath(pp);
    }
};

}  // namespace SDV

#endif //SDV_CIRCLEWIDGET_H
