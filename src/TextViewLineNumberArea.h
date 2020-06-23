//
// Created by kca on 21/6/2020.
//

#ifndef SDV_TEXTVIEWLINENUMBERAREA_H
#define SDV_TEXTVIEWLINENUMBERAREA_H

#include <QWidget>
#include <QPen>
#include "TextView.h"

namespace SDV {

class TextViewLineNumberArea : public QWidget
{
public:
    static const qreal kDefaultLeftMarginCharWidthRatio;  // = 0.5;
    static const qreal kDefaultRightMarginCharWidthRatio;  // = 1.0;
    static const QPen kPen;  // = Pen{QColor{127, 129, 126}}

    using Base = QWidget;
    explicit TextViewLineNumberArea(TextView& textView,
                                    QWidget* parent = nullptr,
                                    Qt::WindowFlags f = Qt::WindowFlags()
                                    );
    ~TextViewLineNumberArea() override = default;

    QSize sizeHint() const override;

    /** Default: kDefaultLeftMarginCharWidthRatio */
    qreal leftMarginCharWidthRatio() { return m_leftMarginCharWidthRatio; }

    /**
     * Default: kDefaultLeftMarginCharWidthRatio
     *
     * @param leftMarginCharWidthRatio
     *        number of 'char widths' to use as left margin
     *        <br>if char width is 12, then 0.5 will give 6 pixels left margin.
     */
    void setLeftMarginCharWidthRatio(qreal leftMarginCharWidthRatio);

    /** Default: kDefaultRightMarginCharWidthRatio */
    qreal rightMarginCharWidthRatio() { return m_rightMarginCharWidthRatio; }

    /**
     * Default: kDefaultRightMarginCharWidthRatio
     *
     * @param rightMarginCharWidthRatio
     *        number of 'char widths' to use as right margin
     *        <br>if char width is 12, then 1.0 will give 12 pixels right margin.
     */
    void setRightMarginCharWidthRatio(qreal rightMarginCharWidthRatio);

    /**
     * Default: kPen
     */
    const QPen& pen() const { return m_pen; }

    /**
     * Default: kPen
     *
     * @param pen
     *        foreground color used to draw line numbers
     */
    void setPen(const QPen& pen);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct Private;
    TextView& m_textView;
    qreal m_leftMarginCharWidthRatio;
    qreal m_rightMarginCharWidthRatio;
    int m_width;
    QPen m_pen;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWLINENUMBERAREA_H
