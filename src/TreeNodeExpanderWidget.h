//
// Created by kca on 25/5/2020.
//

#ifndef SDV_TREENODEEXPANDERWIDGET_H
#define SDV_TREENODEEXPANDERWIDGET_H

#include <QWidget>

namespace SDV {

class TreeNodeExpanderWidget : public QWidget
{
    Q_OBJECT

public:
    using Base = QWidget;
    explicit TreeNodeExpanderWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~TreeNodeExpanderWidget() override = default;

public slots:
//    void slotExpand();
//    void slotCollapse();
    void slotSetExpanded(const bool isExpanded) { m_isExpanded = isExpanded; }

signals:
    void signalIsExpanded(bool isExpanded);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool event(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    bool m_isExpanded;
    bool m_isMouseOver;
};

}  // namespace SDV

#endif //SDV_TREENODEEXPANDERWIDGET_H
