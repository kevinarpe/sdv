//
// Created by kca on 6/6/2020.
//

#ifndef SDV_TEXTVIEW_H
#define SDV_TEXTVIEW_H

#include <unordered_map>
#include <set>
#include <memory>
#include <QAbstractScrollArea>
#include <QPen>
#include <QPainter>
#include <QClipboard>
#include "GraphemeFinder.h"
#include "PaintBackgroundFunctor.h"
#include "PaintForegroundFunctor.h"
#include "TextViewGraphemePosition.h"

namespace SDV {

class TextViewDocument;
class TextViewDocumentView;
class TextViewTextCursor;
class PaintContext;

class TextView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    static const QPen kDefaultTextPen;  // {QColor{Qt::GlobalColor::black}}
    /** Baby blue borrowed from IntelliJ! :) */
    static const QBrush kDefaultSelectedTextBackgroundBrush;  // {QColor{166, 210, 255}};
    /** Light yellow borrowed from IntelliJ! :) */
    static const QBrush kDefaultTextCursorLineBackgroundBrush;  // {QColor{252, 250, 237}};
    static const QBrush kDefaultTextCursorBackgroundBrush;  // {QColor{Qt::GlobalColor::black}};
    static const QPen kDefaultTextCursorTextPen;  // {QColor{Qt::GlobalColor::white}};

    using Base = QAbstractScrollArea;

    explicit TextView(QWidget* parent = nullptr);
    ~TextView() override; // = default

    void setDoc(const std::shared_ptr<TextViewDocument>& doc);
    void afterDocUpdate();

    const std::shared_ptr<TextViewDocumentView>& docViewPtr() const { return m_docView; }
    TextViewDocumentView& docView() { return *m_docView; }
    const TextViewDocumentView& docView() const { return *m_docView; }

    TextViewTextCursor& textCursor() { return *m_textCursor; }
    const TextViewTextCursor& textCursor() const { return *m_textCursor; }

    /** Default: {@link #kDefaultTextPen} */
    const QPen& textPen() const { return m_textPen; }

    /**
     * @param pen
     *        foreground color used to paint text
     *        <br>Override this pen using lineIndex_To_TextFormatSet_Map().
     */
    void setTextPen(const QPen& pen);

    /** Default: {@link #kSelectedTextBackgroundBrush} */
    const QBrush& selectedTextBackgroundBrush() const { return m_selectedTextBackgroundBrush; }

    /**
     * @param brush
     *        background brush used for selected text
     */
    void setSelectedTextBackgroundBrush(const QBrush& brush);

    /** Default: {@link #kDefaultTextCursorLineBackgroundBrush} */
    const QBrush& textCursorLineBackgroundBrush() const { return m_textCursorLineBackgroundBrush; }

    /**
     * @param brush
     *        background brush used for line of text cursor
     */
    void setTextCursorLineBackgroundBrush(const QBrush& brush);

    /** Default: {@link #kDefaultTextCursorBackgroundBrush} */
    const QBrush& textCursorBackgroundBrush() { return m_textCursorBackgroundBrush; }

    /**
     * @param brush
     *        background brush used for text cursor
     */
    void setTextCursorBackgroundBrush(const QBrush& brush);

    /** Default: {@link #kDefaultTextCursorTextPen} */
    const QPen& textCursorTextPen() { return m_textCursorTextPen; }

    /**
     * @param pen
     *        foreground color used to paint text cursor
     */
    void setTextCursorTextPen(const QPen& pen);

    /**
     * Viewport height divided by font line spacing.  If the last visible line is partially visible, it is *included*.
     *
     * This is adjusted after a resize event.
     *
     * @see #viewportFullyVisibleLineCount()
     */
    int viewportVisibleLineCount() const { return m_viewportVisibleLineCount; }

    /**
     * Viewport height divided by font line spacing.  If the last visible line is partially visible, it is *excluded*.
     *
     * This is adjusted after a resize event.
     *
     * @see #viewportVisibleLineCount()
     */
    int viewportFullyVisibleLineCount() const { return m_viewportFullyVisibleLineCount; }

    /**
     * Line index of first visible line in viewport.
     *
     * Example: If the first three lines are hidden, return index 3, because indices 0, 1, and 2 are hidden.
     *
     * @return zero if the document is empty
     */
    int firstVisibleLineIndex() const { return m_firstVisibleLineIndex; }

    /**
     * Line index of last visible line in viewport.  This line may be partially visible in the viewport.
     *
     * @return zero if the document is empty
     */
    int lastVisibleLineIndex() const { return m_lastVisibleLineIndex; }

    /**
     * Line index of last visible line in viewport.  This line is always fully visible in the viewport.
     *
     * @return zero if the document is empty
     */
    int lastFullyVisibleLineIndex() const { return m_lastFullyVisibleLineIndex; }

    // TODO: Also measure visibleLineCount()?

    const std::shared_ptr<PaintContext>& paintBackgroundContext() const { return m_paintBackgroundContext; }
    void setPaintBackgroundContext(const std::shared_ptr<PaintContext>& context) { m_paintBackgroundContext = context; }

    const std::shared_ptr<PaintContext>& paintForegroundContext() const { return m_paintForegroundContext; }
    void setPaintForegroundContext(const std::shared_ptr<PaintContext>& context) { m_paintForegroundContext = context; }

    using BackgroundFormatSet = std::set<LineFormatBackground, LineSegment::NonOverlapCompare<LineFormatBackground>>;
    using ForegroundFormatSet = std::set<LineFormatForeground, LineSegment::NonOverlapCompare<LineFormatForeground>>;

    /** Do not forget to call {@link #update()} or {@link #setDoc(...)} after making modifications. */
    std::unordered_map<int, BackgroundFormatSet>& lineIndex_To_BackgroundFormatSet_Map()
        { return m_lineIndex_To_BackgroundFormatSet_Map; }

    const std::unordered_map<int, BackgroundFormatSet>& lineIndex_To_BackgroundFormatSet_Map() const
        { return m_lineIndex_To_BackgroundFormatSet_Map; }

    /** Do not forget to call {@link #update()} or {@link #setDoc(...)} after making modifications. */
    std::unordered_map<int, ForegroundFormatSet>& lineIndex_To_ForegroundFormatSet_Map()
        { return m_lineIndex_To_ForegroundFormatSet_Map; }

    const std::unordered_map<int, ForegroundFormatSet>& lineIndex_To_ForegroundFormatSet_Map() const
        { return m_lineIndex_To_ForegroundFormatSet_Map; }

    const QRect& textCursorRect() const { return m_textCursorRect; }

    int lineIndexForHeight(qreal viewportYCoord) const;
    // Optionally clip for viewport?
    // TODO: Add?: QRectF rectForLine(int visibleLineIndex) const;
    // TODO: Add?: QRectF rectForPosition(const TextViewPosition& pos) const;

    /**
     * @param visibleLineIndex
     *        must be visible in the current viewport position or assert
     *
     * @return number of pixels from top of viewport to top of line
     */
    qreal heightForVisibleLineIndex(int visibleLineIndex);

    TextViewGraphemePosition
    positionForPoint(const QPointF& viewportPointF,
                     GraphemeFinder::IncludeTextCursor includeTextCursor)
    const;

    /**
     * @return copy of selected text with cross-platform new-line appended to each line except the last line
     *         <br>may be empty
     *
     * @see #slotCopySelectedTextToClipboard()
     */
    QString selectedText() const;

public slots:
    /**
     * Calls {@link #selectedText()}.  If not empty, then put text on clipboard.
     */
    void slotCopySelectedTextToClipboard(QClipboard::Mode mode = QClipboard::Mode::Clipboard) const;

signals:
    /**
     * Emitted when any of these change:
     * <br>{@link #firstVisibleLineIndex()}, {@link lastVisibleLineIndex()}, {@link lastFullyVisibleLineIndex()}
     */
    void signalVisibleLinesChanged();
    void signalSelectedTextChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    struct Private;
    std::shared_ptr<TextViewDocumentView> m_docView;
    std::unique_ptr<TextViewTextCursor> m_textCursor;
    std::unique_ptr<GraphemeFinder> m_graphemeFinder;
    QPen m_textPen;
    QBrush m_selectedTextBackgroundBrush;
    QBrush m_textCursorLineBackgroundBrush;
    QBrush m_textCursorBackgroundBrush;
    QPen m_textCursorTextPen;
    bool m_isAfterSetDoc;
    /**
     * Intentional: Store two copies of text cursor rect.  Why?  QWidget::update(QRect) only accept QRect.
     * <br>However, when we actually paint, we use QRectF for sub-pixel accuracy.  :)
     */
    QRect m_textCursorRect;
    QRectF m_textCursorRectF;
    /** Equals m_textCursorRectF but top-left is (0, 0) */
    QRectF m_textCursorPixmapRectF;
    /** Pixmap when text cursor is visible.  Painted by {@link #m_textCursorPainterVisible}. */
    QPixmap m_textCursorPixmapVisible;
    /** Painter for {@link #m_textCursorPixmapVisible} */
    QPainter m_textCursorPainterVisible;
    /** Pixmap when text cursor is invisible.  Painted by {@link #m_textCursorPainterInvisible}. */
    QPixmap m_textCursorPixmapInvisible;
    /** Painter for {@link #m_textCursorPixmapInvisible} */
    QPainter m_textCursorPainterInvisible;

    int m_viewportFullyVisibleLineCount;
    int m_viewportVisibleLineCount;
    int m_firstVisibleLineIndex;
    int m_lastFullyVisibleLineIndex;
    int m_lastVisibleLineIndex;
    std::shared_ptr<PaintContext> m_paintBackgroundContext;
    std::shared_ptr<PaintContext> m_paintForegroundContext;
    std::unordered_map<int, BackgroundFormatSet> m_lineIndex_To_BackgroundFormatSet_Map;
    std::unordered_map<int, ForegroundFormatSet> m_lineIndex_To_ForegroundFormatSet_Map;
};

}  // namespace SDV

#endif //SDV_TEXTVIEW_H
