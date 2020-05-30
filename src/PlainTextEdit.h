//
// Created by kca on 5/4/2020.
//

#ifndef SDV_PLAINTEXTEDIT_H
#define SDV_PLAINTEXTEDIT_H

#include <memory>
#include <QPlainTextEdit>
#include <QTextBlock>
#include "IWidgetWithLineNumberArea.h"
class QLabel;

namespace SDV {

class LineNumberAreaWidget;
class TreeNodeExpanderWidget;

class PlainTextEdit : public QPlainTextEdit, public IWidgetWithLineNumberArea
{
    Q_OBJECT

public:
    using Base = QPlainTextEdit;
    explicit PlainTextEdit(QWidget* parent = nullptr);
    ~PlainTextEdit() override = default;

    using QPlainTextEdit::blockBoundingGeometry;
    using QPlainTextEdit::contentOffset;

    int lineNumberAreaWidth() const override;
    void lineNumberAreaPaintEvent(QPaintEvent* event) override;
    LineNumberAreaWidget* lineNumberAreaWidget() const { return m_lineNumberAreaWidget; }
    QTextBlock tryGetFirstVisibleBlock() const;
    QTextBlock tryGetLastFullyVisibleBlock() const;
    QTextBlock tryGetLastVisibleBlock() const;

public slots:
    void slotScrollTopLeft();
    void slotScrollBottomRight();
    void slotScrollPageLeft();
    void slotScrollPageRight();

protected:
    void contextMenuEvent(QContextMenuEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void focusOutEvent(QFocusEvent* e) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* e) override;
    bool event(QEvent* event) override;

private:
    struct Private;
    const std::unordered_map<Qt::CursorShape, QCursor>& kCursorMap;
    Qt::CursorShape m_cursorShape;
    QPoint m_lastMouseMovePoint;
    int m_lastMouseOverBlockIndex;
    LineNumberAreaWidget* m_lineNumberAreaWidget;
    TreeNodeExpanderWidget* m_treeNodeExpanderWidget;
    QLabel* m_label;
};

}  // namespace SDV

#endif //SDV_PLAINTEXTEDIT_H
