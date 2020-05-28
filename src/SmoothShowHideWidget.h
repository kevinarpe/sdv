//
// Created by kca on 24/5/2020.
//

#ifndef SDV_SMOOTHSHOWHIDEWIDGET_H
#define SDV_SMOOTHSHOWHIDEWIDGET_H

#include <memory>
#include <QWidget>
#include <QPropertyAnimation>

namespace SDV {

class SmoothShowHideWidget : public QWidget
{
    Q_OBJECT

public:
    using Base = QWidget;
    explicit SmoothShowHideWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~SmoothShowHideWidget() override = default;

public slots:
    void setVisible(bool visible) override;
    virtual void animatedHide();

signals:
    void signalHidden();

private:
    struct Private;
    int m_height;
    std::unique_ptr<QPropertyAnimation> m_nullableShowAnimation;
    std::unique_ptr<QPropertyAnimation> m_nullableHideAnimation;
};

}  // namespace SDV

#endif //SDV_SMOOTHSHOWHIDEWIDGET_H
