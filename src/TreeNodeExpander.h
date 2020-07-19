//
// Created by kca on 25/5/2020.
//

#ifndef SDV_TREENODEEXPANDER_H
#define SDV_TREENODEEXPANDER_H

#include <QWidget>

namespace SDV {

class TreeNodeExpander : public QWidget
{
    Q_OBJECT

public:
    using Base = QWidget;

    explicit TreeNodeExpander(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~TreeNodeExpander() override = default;

    bool isExpanded() const { return m_isExpanded; }

public slots:
    /**
     * A signal is *only* emitted after a change -- expand or collapse.
     * <p>
     * Don't forget: You can call {@link QObject#blockSignals(bool)} before and after this slot to avoid unnecessary signals.
     */
    void slotSetExpanded(bool isExpanded);

signals:
    void signalExpandedChanged(bool isExpanded);

protected:
    void paintEvent(QPaintEvent* event) override;
    bool event(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    bool m_isExpanded;
    bool m_isMouseOver;
};

}  // namespace SDV

#endif //SDV_TREENODEEXPANDER_H
