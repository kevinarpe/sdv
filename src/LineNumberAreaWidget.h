//
// Created by kca on 15/4/2020.
//

#ifndef SDV_LINENUMBERAREAWIDGET_H
#define SDV_LINENUMBERAREAWIDGET_H

#include <QWidget>

namespace SDV {

class IWidgetWithLineNumberArea;

// Ref: https://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html
class LineNumberAreaWidget : public QWidget
{
public:
    using Base = QWidget;
    explicit LineNumberAreaWidget(IWidgetWithLineNumberArea& widget, QWidget* parent = nullptr)
        : Base{parent}, m_widget{widget}
    {}
    ~LineNumberAreaWidget() override = default;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    IWidgetWithLineNumberArea& m_widget;
};

}  // namespace SDV

#endif //SDV_LINENUMBERAREAWIDGET_H
