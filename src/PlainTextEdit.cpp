//
// Created by kca on 5/4/2020.
//

#include "PlainTextEdit.h"
#include <cmath>
#include <QMenu>
#include <QDebug>
#include <QPainter>
#include <QShortcut>
#include <QLabel>
#include <QScrollBar>
#include "LineNumberAreaWidget.h"
#include "PlainTextEditDecorator.h"

// TODO: DELETE ME???

namespace SDV {

static const QPoint INVALID_POINT{-1, -1};

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

    static void
    setClipRect(PlainTextEdit& self, QPaintEvent* event, QPainter* painter)
    {
        QRect rect = event->rect();
        const int adjEventBottom = getAdjEventBottom(self, event);
        if (rect.bottom() > adjEventBottom) {
            rect.setBottom(adjEventBottom);
            painter->setClipRect(rect);
        }
    }

    static int
    getAdjEventBottom(PlainTextEdit& self, QPaintEvent* event)
    {
        int eventBottom = event->rect().bottom();
        if (self.horizontalScrollBar()->isVisible()) {
            const QPoint& topLeft = QPoint{0, 0};
            const QPoint& hbarPosG = self.horizontalScrollBar()->mapToGlobal(topLeft);
            // Important: Use m_lineNumberAreaWidget, not this, b/c event is relative to m_lineNumberAreaWidget.
            const QPoint& hbarPosG2 = self.m_lineNumberAreaWidget->mapFromGlobal(hbarPosG);
            if (eventBottom >= hbarPosG2.y()) {
                // Remember: hbarPosG2.y() is the *first* vertical pixel of hbar.  We want to exclude this vertical pixel.
                eventBottom = hbarPosG2.y() - 1;
            }
        }
        return eventBottom;
    }

    static void
    slotToggleLineWrap(PlainTextEdit& self, const bool isChecked)
    {
        self.setLineWrapMode(isChecked ? Base::LineWrapMode::WidgetWidth : Base::LineWrapMode::NoWrap);
    }

    static void
    slotUpdateRequest(PlainTextEdit& self, const QRect &rect, const int dy)
    {
        if (0 != dy) {
            self.m_lineNumberAreaWidget->scroll(0, dy);
        }
        else {
            self.m_lineNumberAreaWidget->update(0, rect.y(), self.m_lineNumberAreaWidget->width(), rect.height());
        }
    }

    static void
    hoverMoveEvent(PlainTextEdit& self, QHoverEvent* event)
    {
        mouseOverTextBlock(self, event->pos());
        drag(self, event->pos());
    }

    static void
    mouseOverTextBlock(PlainTextEdit& self, const QPoint& mousePos)
    {
        // This *still* looks a little bit weird.  Text colour changes on mouse-over.  Bizarre.
        const QTextCursor& textCursor = self.cursorForPosition(mousePos);
        const QTextBlock& textBlock = textCursor.block();
        const int blockIndex = textBlock.blockNumber();
//        const int firstLineNumber = textBlock.firstLineNumber();
//        const int lineCount = textBlock.lineCount();
        if (blockIndex == self.m_lastMouseOverBlockIndex) {
            return;
        }
//            qDebug() << QString("mouse-over.blockIndex(): %1 -> %2").arg(m_lastMouseOverBlockIndex).arg(blockIndex);
//            if (m_lastMouseOverBlockIndex >= 0) {
//                const QTextBlock& lastTextBlock = document()->findBlockByNumber(m_lastMouseOverBlockIndex);
//                const int lastFirstLineNumber = lastTextBlock.firstLineNumber();
//                const int lastLineCount = lastTextBlock.lineCount();
//                QTextLayout* lastTextLayout = lastTextBlock.layout();
//                QVector<QTextLayout::FormatRange> lastFormatVec = lastTextLayout->formats();
//                assert(lastFormatVec.size() > 0);
//                lastFormatVec.removeFirst();
//                lastTextLayout->setFormats(lastFormatVec);
//                document()->markContentsDirty(lastTextBlock.position(), lastTextBlock.length());
//            }
        self.m_lastMouseOverBlockIndex = blockIndex;
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
        QList<QTextEdit::ExtraSelection> extraList = self.extraSelections();
        extraList.clear();
        extraList.append(extra);
        // This is RANDOM.  Notice that I only add a *single* dummy "extra" selection.  For unknown reasons, this
        // causes the full-width (mouse-over) background to paint first, not last.  <shruggy>!
        // Ref: https://stackoverflow.com/questions/28257022/qsyntaxhighlighter-text-selection-overrides-style
        if (formatVec.size() > 0)
        {
            const QTextLayout::FormatRange& fr = formatVec[0];
            QTextCursor tc{self.document()};
            tc.setPosition(textBlock.position(), QTextCursor::MoveMode::MoveAnchor);
            tc.setPosition(textBlock.position() + textBlock.length(), QTextCursor::MoveMode::KeepAnchor);
            QTextEdit::ExtraSelection extra0{};
            extra0.cursor = tc;
            extra0.format = fr.format;
            extraList.append(extra0);
        }
        self.setExtraSelections(extraList);
    }

    static void
    drag(PlainTextEdit& self, const QPoint& mousePos)
    {
        if (false == isDragging(self)) {
            return;
        }
        if (Qt::CursorShape::IBeamCursor == self.m_cursorShape) {
            return;
        }
        // First visible line
        const QTextCursor& textCursor0 = self.cursorForPosition(QPoint{0, 0});
        const QTextBlock& textBlock0 = textCursor0.block();
        const QString& text0 = textBlock0.text();
        // Ex: 18, 20, or 24
        // In practice, due to slight line height variations due to latin and non-latin (CJK) chars,
        // this line height is not uniform.  Usually, CJK chars are slightly tallers than latin chars.
        const qreal height0 = textBlock0.layout()->boundingRect().height();
        // Coordinate system origin (0, 0) is top left.
        const int dXPixel = self.m_lastMouseMovePoint.x() - mousePos.x();
        const int dYPixel = self.m_lastMouseMovePoint.y() - mousePos.y();
        // Intentional: Integer truncate: 17/18 -> 0!
        const int dLine = dYPixel / ((int) height0);
        // For QPlainTextEdit, horizonal scrolling is smooth -- per pixel.  Thus, horizonal bar range is pixels.
        // However, vertical scrolling is not smooth -- per line.  Thus, vertical bar range is lines.
        // If first visible line height is 18 pixels
        const int hbarPixel = self.horizontalScrollBar()->value();
        const int vbarLine = self.verticalScrollBar()->value();
//        qDebug() << QString("x[%1,%2]: %3 + %4 = %5, vbarLine[%6,%7]: %8 + %9 = %10, dy2: %11")
//            .arg(horizontalScrollBar()->minimum()).arg(horizontalScrollBar()->maximum())
//            .arg(hbarPixel).arg(dXPixel).arg(hbarPixel + dXPixel)
//            .arg(verticalScrollBar()->minimum()).arg(verticalScrollBar()->maximum())
//            .arg(vbarLine).arg(dYPixel).arg(vbarLine + dYPixel)
//            .arg(dLine);
        self.horizontalScrollBar()->setValue(hbarPixel + dXPixel);
        // Always update horizontal mouse move point, as this is pixel-based, not line-based.
        self.m_lastMouseMovePoint.rx() -= dXPixel;
        // Only update vertical mouse move point if at least one line of vertical distance moved.
        if (0 != dLine) {
            self.verticalScrollBar()->setValue(vbarLine + dLine);
            self.m_lastMouseMovePoint.ry() -= dYPixel;
        }
    }

    static bool
    isDragging(const PlainTextEdit& self)
    {
        const bool x = (INVALID_POINT != self.m_lastMouseMovePoint);
        return x;
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
      m_lastMouseMovePoint{INVALID_POINT},
      m_lastMouseOverBlockIndex{-1},
      m_decorator{new PlainTextEditDecorator{*this}}
{
    // Enable QEvent::Type::HoverMove
    setAttribute(Qt::WidgetAttribute::WA_Hover, true);

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
        QShortcut* topLeftShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Home), this);
        QObject::connect(topLeftShortcut, &QShortcut::activated, this, &PlainTextEdit::slotScrollHorizontalHome);
    }
    {
        QShortcut* topLeftShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_End), this);
        QObject::connect(topLeftShortcut, &QShortcut::activated, this, &PlainTextEdit::slotScrollHorizontalEnd);
    }
    {
        QShortcut* pageLeftShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_PageUp), this);
        QObject::connect(pageLeftShortcut, &QShortcut::activated, this, &PlainTextEdit::slotScrollPageLeft);
    }
    {
        QShortcut* pageRightShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_PageDown), this);
        QObject::connect(pageRightShortcut, &QShortcut::activated, this, &PlainTextEdit::slotScrollPageRight);
    }
    QObject::connect(this, &PlainTextEdit::updateRequest,
        [this](const QRect& rect, int dy) { Private::slotUpdateRequest(*this, rect, dy); });
}

// public
int
PlainTextEdit::
lineNumberAreaWidth()
const  // override
{
    // +1?  Remember: log10 of 150 is 2, not 3. :)
    const qreal digitCount = 1 + std::log10(blockCount());
    const QFontMetricsF fontMetricsF{font()};
    // +1?  Trailing space that is one digit wide.
    const qreal y = (1.0 + digitCount) * fontMetricsF.horizontalAdvance(QLatin1Char{'9'});
    // +0.5?  Truncate correctly: 1.1 -> 1, 1.4 -> 1, 1.5 -> 2, 1.6 -> 2
    const int x = (0.5 + y);
    return x;
}

// public
void
PlainTextEdit::
lineNumberAreaPaintEvent(QPaintEvent* event)  // override
{
    QPainter painter{m_lineNumberAreaWidget};
    const QPalette& palette = this->palette();
    const QColor& bgColor = palette.color(QPalette::ColorRole::Window);
    painter.fillRect(event->rect(), bgColor);
    // TODO: Replace with a QPalette color role
    painter.setPen(QColor{127, 129, 126});
    painter.setFont(font());
    QTextBlock textBlock = firstVisibleBlock();
    int blockIndex = textBlock.blockNumber();

    const QRectF& bbg = blockBoundingGeometry(textBlock);
    const QPointF& co = contentOffset();
    const QRectF& bbgt = bbg.translated(co);
    qreal top = viewport()->y() + bbgt.top();
    qreal height = bbg.height();
    qreal nextTop = top + height;
    const QFontMetricsF fontMetricsF{font()};
    const qreal fontHeight = fontMetricsF.height();
    const qreal width = m_lineNumberAreaWidget->width();
    const qreal charWidth = fontMetricsF.horizontalAdvance(QLatin1Char{'9'});
    Private::setClipRect(*this, event, &painter);

    while (textBlock.isValid() && top <= event->rect().bottom())
    {
        if (textBlock.isVisible()
            // This condition makes no sense to me!  How can it ever be false?
            && nextTop >= event->rect().top()/*
            && nextTop <= adjEventBottom*/) {

            const QString& number = QString::number(blockIndex + 1);
            const double topOffset = (height - fontHeight) / 2.0;
            painter.drawText(QRectF{0, top + topOffset, width - charWidth, fontHeight},
                Qt::AlignmentFlag::AlignRight, number);
        }
        textBlock = textBlock.next();
        top = nextTop;
        const QRectF& bbr = blockBoundingRect(textBlock);
        height = bbr.height();
        nextTop = top + height;
        ++blockIndex;
    }
}

/**
 * @return result may be invalid when block count is zero
 *
 * @see QTextBlock::isValid()
 */
// public
QTextBlock
PlainTextEdit::
tryGetFirstVisibleBlock()
const
{
    const QTextCursor& tc = cursorForPosition(QPoint{0, 0});
    const QTextBlock& x = tc.block();
    return x;
}

/**
 * @return result may be invalid when block count is zero
 *         <br>If the result is valid, it is not guaranteed to be fully visible.  It may be clipped.
 *
 * @see QTextBlock#isValid()
 * @see #tryGetLastFullyVisibleBlock()
 */
// public
QTextBlock
PlainTextEdit::
tryGetLastVisibleBlock()
const
{
    const int blockCount = this->blockCount();
    if (0 == blockCount) {
        const QTextBlock& x = document()->firstBlock();
        return x;
    }
    const QPoint point{0, viewport()->height() - 1};
    const QTextCursor& tc = cursorForPosition(point);
    const QTextBlock& tb = tc.block();
    if (tb.isValid()) {
        // Ex: tb.text(): "                \"description\": \"元野球部マネージャー❤︎…最高の夏をありがとう…❤︎\","
        return tb;
    }
    else {
        const QTextBlock& x = document()->findBlockByNumber(blockCount - 1);
        return x;
    }
}

/**
 * @return result may be invalid when block count is zero
 *         <br>If the result is valid, it is guaranteed to be fully visible.  It is never clipped.
 *
 * @see QTextBlock#isValid()
 * @see #tryGetLastVisibleBlock()
 */
// public
QTextBlock
PlainTextEdit::
tryGetLastFullyVisibleBlock()
const
{
    QTextBlock tb = tryGetLastVisibleBlock();
    if (false == tb.isValid()) {
        return tb;
    }
    const QRectF& bbg = blockBoundingGeometry(tb);
    // Necessary?  Unsure.
    const QPointF& co = contentOffset();
    if (0 != co.x() && 0 != co.y()) {
        // @DebugBreakpoint
        int dummy = 1;
    }
    const QRectF& bbgt = bbg.translated(co);
    const qreal b1 = bbgt.bottom();
    const int b2 = viewport()->height();
    if (b1 <= b2) {
        return tb;
    }
    else {
        const QTextBlock& x = tb.previous();
        return x;
    }
}

// public
void
PlainTextEdit::
setResult(const JsonTree& result)
{
    m_result = result;
    setPlainText(result.jsonText);
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
slotScrollHorizontalHome()
{
    horizontalScrollBar()->setValue(0);
}

// public slot
void
PlainTextEdit::
slotScrollHorizontalEnd()
{
    horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
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
contextMenuEvent(QContextMenuEvent* e)  // override
{
    // Ref: https://doc.qt.io/qt-5/qplaintextedit.html#contextMenuEvent
    // Ref: https://stackoverflow.com/a/43820412/257299
    QMenu* menu = createStandardContextMenu();
    menu->addSeparator();
    QAction* action = menu->addAction("Word Wrap");
    action->setCheckable(true);
    action->setChecked(Base::LineWrapMode::WidgetWidth == lineWrapMode());
    QObject::connect(action, &QAction::toggled,
        [this](bool isChecked) { Private::slotToggleLineWrap(*this, isChecked); });
    // @Blocking
    menu->exec(e->globalPos());
    delete menu;
}

// protected
void
PlainTextEdit::
keyPressEvent(QKeyEvent* e)  // override
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
keyReleaseEvent(QKeyEvent* e)  // override
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
mousePressEvent(QMouseEvent* event)  // override
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
mouseReleaseEvent(QMouseEvent* event)  // override
{
    if (Qt::KeyboardModifier::ControlModifier == event->modifiers()
        // Careful: Singular button() is the button that caused the event.  Plural buttons() will never include
        // button(), as it has been released.
        && Qt::MouseButton::LeftButton == event->button()) {

        Private::setCursor(*this, Qt::CursorShape::OpenHandCursor);
        m_lastMouseMovePoint = INVALID_POINT;
    }
    Base::mouseReleaseEvent(event);
}

// protected
void
PlainTextEdit::
mouseMoveEvent(QMouseEvent* event)  // override
{
    // (1) Mouse tracking is *not* enabled.  Thus, this method is only called when a mouse button is pressed.
    // (2) Only process the event if IBeamCursor for text selection.
    // (3) Ignore this event for other cursor shapes to prevent text selection during drag.
    if (Qt::CursorShape::IBeamCursor == m_cursorShape) {
        Base::mouseMoveEvent(event);
    }
}

// protected
void
PlainTextEdit::
focusOutEvent(QFocusEvent* e)  // override
{
    // Triggered when: Ctrl pressed, then Ctrl+F pressed -> focus transfered to find text lineedit.
    Private::setCursor(*this, Qt::CursorShape::IBeamCursor);
    Base::focusOutEvent(e);
}

// protected
void
PlainTextEdit::
leaveEvent(QEvent* event)  // override
{
    QList<QTextEdit::ExtraSelection> extraList = extraSelections();
    extraList.clear();
    setExtraSelections(extraList);
    QWidget::leaveEvent(event);
}

// protected
void
PlainTextEdit::
resizeEvent(QResizeEvent* e)  // override
{
    Base::resizeEvent(e);
    m_lineNumberAreaWidget->setMinimumWidth(lineNumberAreaWidth());
}

// protected
bool
PlainTextEdit::
event(QEvent* event)  // override
{
    const QEvent::Type type = event->type();
    switch (type) {
        case QEvent::Type::HoverMove: {
            Private::hoverMoveEvent(*this, static_cast<QHoverEvent*>(event));
            break;
        }
    }
    const bool x = Base::event(event);
    return x;
}

// protected
void
PlainTextEdit::
showEvent(QShowEvent* event)  // override
{
    Base::showEvent(event);
    emit signalShow();
}

}  // namespace SDV
