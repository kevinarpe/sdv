//
// Created by kca on 5/4/2020.
//

#include "PlainTextEdit.h"
#include <cmath>
#include <QMenu>
#include <QDebug>
#include <QPainter>
#include <QShortcut>
#include "LineNumberAreaWidget.h"
#include "CircleWidget.h"

namespace SDV {

struct PlainTextEdit::Private
{
    // Note: It is not safe to declare static Qt objects, so we need to init on first ctor call, not during static init.
    // "Close enough"... feels like static init, but safe.  :)
    static std::unique_ptr<std::unordered_map<Qt::CursorShape, QCursor>> staticCursorMap;

    static const std::unordered_map<Qt::CursorShape, QCursor>&
    staticGetCursorMap() {
        if (nullptr == staticCursorMap) {
            staticCursorMap = std::make_unique<std::unordered_map<Qt::CursorShape, QCursor>>();
            for (Qt::CursorShape cs : {Qt::CursorShape::IBeamCursor, Qt::CursorShape::OpenHandCursor, Qt::CursorShape::ClosedHandCursor}) {
                (*staticCursorMap)[cs] = QCursor(cs);
            }
        }
        return *staticCursorMap;
    }

    static void
    setCursor(PlainTextEdit& self, Qt::CursorShape cursorShape)
    {
        self.m_cursorShape = cursorShape;
        const QCursor& cursor = self.kCursorMap.at(cursorShape);
        self.viewport()->setCursor(cursor);
    }

    static int
    getAdjEventBottom(PlainTextEdit& self, QPaintEvent* event);

    static void
    slotToggleLineWrap(PlainTextEdit& self, const bool isChecked)
    {
        self.setLineWrapMode(isChecked ? Base::LineWrapMode::WidgetWidth : Base::LineWrapMode::NoWrap);
    }
//
//    static void
//    slotBlockCountChanged(PlainTextEdit& self, int /*newBlockCount*/)
//    {
////        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
//        int dummy = 1;
//    }

    static void
    slotUpdateRequest(PlainTextEdit& self, const QRect &rect, const int dy)
    {
        if (0 != dy) {
            self.m_lineNumberAreaWidget->scroll(0, dy);
        }
        else {
            self.m_lineNumberAreaWidget->update(0, rect.y(), self.m_lineNumberAreaWidget->width(), rect.height());
        }
//        if (rect.contains(self.viewport()->rect())) {
//            slotBlockCountChanged(self, 0);
//        }
    }

    static void
    slotTextChanged(PlainTextEdit& self)
    {
/*
        const QTextBlock& textBlock = self.document()->findBlockByLineNumber(12);
        assert(textBlock.isValid());
        const QString& text = textBlock.text();
        qDebug() << text.mid(0, 20);
        const int hadv0 = self.fontMetrics().horizontalAdvance(text, 19);
        const int hadv1 = self.fontMetrics().horizontalAdvance(text, 20);
        const double char32width = hadv1 - hadv0;
        const QRectF& bbg = self.blockBoundingGeometry(textBlock);
        const QPointF& co = self.contentOffset();
        const QRectF& bbgt = bbg.translated(co);
//        CircleWidget* cw = new CircleWidget{&self};
        CircleWidget* cw = new CircleWidget{self.viewport()};
        const double hw = 50.0f;
        const QRect& r = QRect(//0, 0, hw, hw);
            bbgt.left() + hadv0 + (char32width / 2.0f) - (hw / 2.0f),
            bbgt.top() + (bbgt.height() / 2.0f) - (hw / 2.0f),
            hw, hw);
        cw->setGeometry(r);
        cw->show();
        cw->raise();
*/
    }
};

// private static
std::unique_ptr<std::unordered_map<Qt::CursorShape, QCursor>>
PlainTextEdit::Private::
staticCursorMap{nullptr};

// public explicit
PlainTextEdit::
PlainTextEdit(QWidget* parent /*= nullptr*/)
    : Base{parent},
      kCursorMap{Private::staticGetCursorMap()},
      // Default state: IBeamCursor -> Ctrl & left mouse button not pressed
      m_cursorShape{Qt::CursorShape::IBeamCursor},
      m_lastMouseOverBlockNumber{-1}
{
    setMouseTracking(true);

    m_lineNumberAreaWidget = new LineNumberAreaWidget{*this};
    {
        QShortcut* topLeftShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Home), this);
        QObject::connect(topLeftShortcut, &QShortcut::activated, this, &PlainTextEdit::slotScrollTopLeft);
    }
    {
        QShortcut* bottomRightShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_End), this);
        QObject::connect(bottomRightShortcut, &QShortcut::activated, this, &PlainTextEdit::slotScrollBottomRight);
    }
    {
        QShortcut* pageLeftShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_PageUp), this);
        QObject::connect(pageLeftShortcut, &QShortcut::activated, this, &PlainTextEdit::slotScrollPageLeft);
    }
    {
        QShortcut* pageRightShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_PageDown), this);
        QObject::connect(pageRightShortcut, &QShortcut::activated, this, &PlainTextEdit::slotScrollPageRight);
    }
//    QObject::connect(this, &PlainTextEdit::blockCountChanged,
//        [this](int newBlockCount) { PlainTextEdit::Private::slotBlockCountChanged(*this, newBlockCount); });
    QObject::connect(this, &PlainTextEdit::updateRequest,
        [this](const QRect& rect, int dy) { PlainTextEdit::Private::slotUpdateRequest(*this, rect, dy); });
    QObject::connect(this, &PlainTextEdit::textChanged, [this]() { Private::slotTextChanged(*this); });
//
//    Private::slotBlockCountChanged(*this, 0);
}

// public
int
PlainTextEdit::
lineNumberAreaWidth()
const // override
{
    // +1?  Remember: log10 of 150 is 2, not 3. :)
    const int digitCount = 1 + std::log10(blockCount());
    // +2?  Leading and trailing space that is one digit wide
    const int x = (2 + digitCount) * fontMetrics().horizontalAdvance(QLatin1Char{'0'});
    return x;
}

// public
void
PlainTextEdit::
lineNumberAreaPaintEvent(QPaintEvent* event) // override
{
    QPainter painter{m_lineNumberAreaWidget};
    painter.setFont(font());
    const QPoint& topLeftG2 = m_lineNumberAreaWidget->mapToGlobal(m_lineNumberAreaWidget->pos());
    const QPalette& palette = this->palette();
    const QColor& bgColor = palette.color(QPalette::Background);
    painter.fillRect(event->rect(), bgColor);
    painter.setPen(QColor{127, 129, 126});
    QTextBlock textBlock = firstVisibleBlock();
    int blockIndex = textBlock.blockNumber();
    const QRectF& bbg = blockBoundingGeometry(textBlock);
    const QPoint& topLeftG = mapToGlobal(bbg.topLeft().toPoint());
    const QPointF& co = contentOffset();
    const QPoint& coG = mapToGlobal(co.toPoint());
    const QPoint& co2 = m_lineNumberAreaWidget->mapFromGlobal(coG);
//    LAST: TURN OFF X WINDOWS SCALING (130% -> 100% AND TRY AGAIN!)
    const QRectF& bbgt = bbg.translated(co);
    const QRectF& bbgt2 = bbg.translated(co2);
    int top = qRound(bbgt.top());
    const QRectF& z = this->document()->documentLayout()->blockBoundingRect(textBlock);
//    const QRectF& z = this->document()->documentLayout()->frameBoundingRect(textBlock);
    const QRectF& bbr = blockBoundingRect(textBlock);
    int bottom = top + qRound(bbr.height());  // not 'bottom' but 'next top!'
//    int bottom = top + qRound(bbg.height());
    const int oneDigitWidth = fontMetrics().horizontalAdvance(QLatin1Char{'0'});
    const int eventBottom = PlainTextEdit::Private::getAdjEventBottom(*this, event);

    while (textBlock.isValid() && top <= eventBottom) {
//        qDebug() << "block height: " << (bottom - top);
        if (textBlock.isVisible() && bottom >= event->rect().top() && bottom <= eventBottom) {
            QString lineNumber = QString::number(1 + blockIndex);
            // Vertically center the line number.  In theory, fontMetrics().height() should *always* be less than or
            // equal to (bottom - top), but be safe and use std::max().
            const int fontHeight = painter.fontMetrics().height();
            const int topOffset = std::max(0, (bottom - top) - fontHeight) / 2;
            if (0 != topOffset) {
//                qDebug() << "topOffset: " << topOffset;
            }
            // -oneDigitWidth?  Left one digit width in right margin
            painter.drawText(0, topOffset + top, m_lineNumberAreaWidget->width() - oneDigitWidth, fontHeight,
                             Qt::AlignmentFlag::AlignRight, lineNumber);
        }
        textBlock = textBlock.next();
        top = bottom;
        const QRectF& bbr2 = blockBoundingRect(textBlock);
        bottom = top + qRound(bbr2.height());
        ++blockIndex;
    }
}

int
PlainTextEdit::Private::
getAdjEventBottom(PlainTextEdit& self, QPaintEvent* event)
{
    int eventBottom = event->rect().bottom();
    if (self.horizontalScrollBar()->isVisible()) {
        const QPoint& hbarPosG = self.horizontalScrollBar()->mapToGlobal(self.horizontalScrollBar()->pos());
        // Important: Use m_lineNumberAreaWidget, not this, b/c event is relative to m_lineNumberAreaWidget.
        const QPoint& hbarPosG2 = self.m_lineNumberAreaWidget->mapFromGlobal(hbarPosG);
        if (eventBottom >= hbarPosG2.y()) {
            // Remember: hbarPosG2.y() is the *first* vertical pixel of hbar.  We want to exclude this vertical pixel.
            eventBottom = hbarPosG2.y() - 1;
        }
    }
    return eventBottom;
}

// public slot
void
PlainTextEdit::
slotScrollTopLeft()
{
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);
}

// public slot
void
PlainTextEdit::
slotScrollBottomRight()
{
    horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

// public slot
void
PlainTextEdit::
slotScrollPageLeft()
{
    const int w = contentsRect().width() - (verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0);
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - w);
}

// public slot
void
PlainTextEdit::
slotScrollPageRight()
{
    const int w = contentsRect().width() - (verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0);
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + w);
}

// protected
void
PlainTextEdit::
contextMenuEvent(QContextMenuEvent* e) // override
{
    // Ref: https://doc.qt.io/qt-5/qplaintextedit.html#contextMenuEvent
    // Ref: https://stackoverflow.com/a/43820412/257299
    QMenu* menu = createStandardContextMenu();
    menu->addSeparator();
    QAction* action = menu->addAction("Word Wrap");
    action->setCheckable(true);
    action->setChecked(Base::LineWrapMode::WidgetWidth == lineWrapMode());
    QObject::connect(action, &QAction::toggled,
        [this](bool isChecked) { PlainTextEdit::Private::slotToggleLineWrap(*this, isChecked); });
    // @Blocking
    menu->exec(e->globalPos());
    delete menu;
}

// protected
void
PlainTextEdit::
keyPressEvent(QKeyEvent* e) // override
{
    // If we find a platform where e->modifiers() cannot be trusted, see: QGuiApplication::keyboardModifiers()
    if (Qt::KeyboardModifier::ControlModifier == e->modifiers() && Qt::Key::Key_Control == e->key()) {
        // qDebug() << QString("key-press: modifiers: 0x%1, key: 0x%2").arg(e->modifiers(), 0, 16).arg(e->key(), 0, 16);
        Private::setCursor(*this, Qt::CursorShape::OpenHandCursor);
    }
    else {
        Private::setCursor(*this, Qt::CursorShape::IBeamCursor);
    }
    Base::keyPressEvent(e);
}

// protected
void
PlainTextEdit::
keyReleaseEvent(QKeyEvent* e) // override
{
    if (Qt::Key::Key_Control == e->key()) {
        // qDebug() << QString("key-release: modifiers: 0x%1, key: 0x%2").arg(e->modifiers(), 0, 16).arg(e->key(), 0, 16);
        Private::setCursor(*this, Qt::CursorShape::IBeamCursor);
    }
    Base::keyReleaseEvent(e);
}

// protected
void
PlainTextEdit::
mousePressEvent(QMouseEvent* event) // override
{
    if (Qt::KeyboardModifier::ControlModifier == event->modifiers()
        // Careful: Singular button() is the button that caused the event.  Plural buttons() will include
        // button() and maybe more.
        && Qt::MouseButton::LeftButton == event->button()) {

        Private::setCursor(*this, Qt::CursorShape::ClosedHandCursor);
        m_lastMouseMovePoint = event->pos();
    }
    Base::mousePressEvent(event);
}

// protected
void
PlainTextEdit::
mouseReleaseEvent(QMouseEvent* event) // override
{
    if (Qt::KeyboardModifier::ControlModifier == event->modifiers()
        // Careful: Singular button() is the button that caused the event.  Plural buttons() will never include
        // button(), as it has been released.
        && Qt::MouseButton::LeftButton == event->button()) {

        Private::setCursor(*this, Qt::CursorShape::OpenHandCursor);
    }
    Base::mouseReleaseEvent(event);
}

// protected
void
PlainTextEdit::
mouseMoveEvent(QMouseEvent* event) // override
{
    {
        const QPoint& pos = event->pos();
        const QTextCursor& textCursor = cursorForPosition(pos);
        const QTextBlock& textBlock = textCursor.block();
        const int blockNumber = textBlock.blockNumber();
//        const int firstLineNumber = textBlock.firstLineNumber();
//        const int lineCount = textBlock.lineCount();
        if (blockNumber != m_lastMouseOverBlockNumber) {
//            qDebug() << QString("mouse-over.blockNumber(): %1 -> %2").arg(m_lastMouseOverBlockNumber).arg(blockNumber);
//            if (m_lastMouseOverBlockNumber >= 0) {
//                const QTextBlock& lastTextBlock = document()->findBlockByNumber(m_lastMouseOverBlockNumber);
//                const int lastFirstLineNumber = lastTextBlock.firstLineNumber();
//                const int lastLineCount = lastTextBlock.lineCount();
//                QTextLayout* lastTextLayout = lastTextBlock.layout();
//                QVector<QTextLayout::FormatRange> lastFormatVec = lastTextLayout->formats();
//                assert(lastFormatVec.size() > 0);
//                lastFormatVec.removeFirst();
//                lastTextLayout->setFormats(lastFormatVec);
//                document()->markContentsDirty(lastTextBlock.position(), lastTextBlock.length());
//            }
            m_lastMouseOverBlockNumber = blockNumber;
            QTextLayout* textLayout = textBlock.layout();
            QVector<QTextLayout::FormatRange> formatVec = textLayout->formats();
//            QTextLayout::FormatRange formatRange{};
//            formatRange.start = 0;
////            formatRange.length = textBlock.length();
//            formatRange.length = std::numeric_limits<int>::max();
//            formatRange.format.setProperty(QTextFormat::Property::FullWidthSelection, true);
//            formatRange.format.setBackground(QBrush{QColor{192, 230, 255}});
//            // Intentional: Always paint mouse-over background color *first*, then allow other formats to overwrite.
//            formatVec.insert(0, std::move(formatRange));
//            textLayout->setFormats(formatVec);
//            document()->markContentsDirty(textBlock.position(), textBlock.length());

            QTextEdit::ExtraSelection extra{};
            extra.cursor = textCursor;
            // Ref: https://lists.qt-project.org/pipermail/qt-interest-old/2009-August/011566.html
            extra.format.setProperty(QTextFormat::Property::FullWidthSelection, true);
//            extra.format.setBackground(QBrush{QColor{192, 230, 255}});
//            extra.format.setBackground(QBrush{QColor{192, 230, 255}});
            // Ref: https://www.colorhexa.com/007fff
            extra.format.setBackground(QBrush{QColor{216, 235, 255}});
//            extra.format.setBackground(QBrush{QColor{235, 245, 255}});
            QList<QTextEdit::ExtraSelection> extraList = extraSelections();
            extraList.clear();
            extraList.append(extra);
            // This is RANDOM.  Notice that I only add a *single* dummy "extra" selection.  For unknown reasons, this
            // causes the full-width (mouse-over) background to paint first, not last.  <shruggy>!
            // Ref: https://stackoverflow.com/questions/28257022/qsyntaxhighlighter-text-selection-overrides-style
            if (formatVec.size() > 0) {
                const QTextLayout::FormatRange& fr = formatVec[0];
                QTextCursor tc{document()};
                tc.setPosition(textBlock.position(), QTextCursor::MoveMode::MoveAnchor);
                tc.setPosition(textBlock.position() + textBlock.length(), QTextCursor::MoveMode::KeepAnchor);
                QTextEdit::ExtraSelection extra0{};
                extra0.cursor = tc;
                extra0.format = fr.format;
                extraList.append(extra0);
            }
            setExtraSelections(extraList);
        }
    }
    if (Qt::CursorShape::IBeamCursor == m_cursorShape) {
        Base::mouseMoveEvent(event);
        return;
    }
    // First visible line
    const QTextCursor& textCursor0 = cursorForPosition(QPoint(0, 0));
    const QTextBlock& textBlock0 = textCursor0.block();
    const QString& text0 = textBlock0.text();
    // Ex: 18, 20, or 24
    // In practice, due to slight line height variations due to latin and non-latin (CJK) chars,
    // this line height is not uniform.  Usually, CJK chars are slightly tallers than latin chars.
    const qreal height0 = textBlock0.layout()->boundingRect().height();
    const QPoint pos = event->pos();
    // Coordinate system origin (0, 0) is top left.
    const int dXPixel = m_lastMouseMovePoint.x() - pos.x();
    const int dYPixel = m_lastMouseMovePoint.y() - pos.y();
    // Intentional: Integer truncate: 17/18 -> 0!
    const int dLine = dYPixel / ((int) height0);
    // For QPlainTextEdit, horizonal scrolling is smooth -- per pixel.  Thus, horizonal bar range is pixels.
    // However, vertical scrolling is not smooth -- per line.  Thus, vertical bar range is lines.
    // If first visible line height is 18 pixels
    const int hbarPixel = horizontalScrollBar()->value();
    const int vbarLine = verticalScrollBar()->value();
//        qDebug() << QString("x[%1,%2]: %3 + %4 = %5, vbarLine[%6,%7]: %8 + %9 = %10, dy2: %11")
//            .arg(horizontalScrollBar()->minimum()).arg(horizontalScrollBar()->maximum())
//            .arg(hbarPixel).arg(dXPixel).arg(hbarPixel + dXPixel)
//            .arg(verticalScrollBar()->minimum()).arg(verticalScrollBar()->maximum())
//            .arg(vbarLine).arg(dYPixel).arg(vbarLine + dYPixel)
//            .arg(dLine);
    horizontalScrollBar()->setValue(hbarPixel + dXPixel);
    // Always update horizontal mouse move point, as this is pixel-based, not line-based.
    m_lastMouseMovePoint.rx() -= dXPixel;
    // Only update vertical mouse move point if at least one line of vertical distance moved.
    if (0 != dLine) {
        verticalScrollBar()->setValue(vbarLine + dLine);
        m_lastMouseMovePoint.ry() -= dYPixel;
    }
}

// protected
void
PlainTextEdit::
focusOutEvent(QFocusEvent* e) // override
{
    // Triggered when: Ctrl pressed, then Ctrl+F pressed -> focus transfered to find text lineedit.
    Private::setCursor(*this, Qt::CursorShape::IBeamCursor);
    QPlainTextEdit::focusOutEvent(e);
}

// protected
void
PlainTextEdit::
leaveEvent(QEvent* event) // override
{
    QList<QTextEdit::ExtraSelection> extraList = extraSelections();
    extraList.clear();
    setExtraSelections(extraList);
    QWidget::leaveEvent(event);
}

// protected
void
PlainTextEdit::
resizeEvent(QResizeEvent* e) // override
{
    QPlainTextEdit::resizeEvent(e);

    m_lineNumberAreaWidget->setMinimumWidth(lineNumberAreaWidth());
}

}  // namespace SDV
