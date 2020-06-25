//
// Created by kca on 6/6/2020.
//

#ifndef SDV_TEXTVIEW_H
#define SDV_TEXTVIEW_H

#include <unordered_map>
#include <set>
#include <memory>
#include <QAbstractScrollArea>
#include "GraphemeFinder.h"
#include "TextViewLineTextFormat.h"

namespace SDV {

class TextViewDocument;
class TextViewDocumentView;
class TextViewTextCursor;
class PaintEventContext;

class TextView : public QAbstractScrollArea
{
    Q_OBJECT

public:
    /** Baby blue borrowed from IntelliJ! :) */
    static const QBrush kSelectedTextBackgroundBrush;  // {QBrush{QColor{166, 210, 255}}};
    /** Light yellow borrowed from IntelliJ! :) */
    static const QBrush kTextCursorLineBackgroundBrush;  // {QBrush{QColor{252, 250, 237}}};

    using Base = QAbstractScrollArea;
    explicit TextView(QWidget* parent = nullptr);
    ~TextView() override; // = default

    void setDoc(const std::shared_ptr<TextViewDocument>& doc);
    const TextViewDocumentView& docView() const { return *m_docView; }

    TextViewTextCursor& textCursor() { return *m_textCursor; }
    const TextViewTextCursor& textCursor() const { return *m_textCursor; }

    /** Default: kSelectedTextBackgroundBrush */
    const QBrush& selectedTextBackgroundBrush() const { return m_selectedTextBackgroundBrush; }

    /**
     * Default: kSelectedTextBackgroundBrush
     *
     * @param b
     *        background brush used for selected text
     */
    void setSelectedTextBackgroundBrush(const QBrush& b);

    /** Default: kTextCursorLineBackgroundBrush */
    const QBrush& textCursorLineBackgroundBrush() const { return m_textCursorLineBackgroundBrush; }

    /**
     * Default: kTextCursorLineBackgroundBrush
     *
     * @param b
     *        background brush used for line of text cursor
     */
    void setTextCursorLineBackgroundBrush(const QBrush& b);

    /**
     * Viewport height divided by font line spacing.  If the last visible line is partially visible, it is *included*.
     *
     * This is adjusted after a resize event.
     *
     * @see viewportFullyVisibleLineCount()
     */
    int viewportVisibleLineCount() const { return m_viewportVisibleLineCount; }

    /**
     * Viewport height divided by font line spacing.  If the last visible line is partially visible, it is *excluded*.
     *
     * This is adjusted after a resize event.
     *
     * @see viewportVisibleLineCount()
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

    // @Nullable
    PaintEventContext* paintEventContext() const { return m_paintEventContext.get(); }

    void setPaintEventContext(const std::shared_ptr<PaintEventContext>& paintEventContext)
        { m_paintEventContext = paintEventContext; }

    using TextFormatSet = std::set<TextViewLineTextFormat, TextViewLineTextFormat::NonOverlapCompare>;

    std::unordered_map<int, TextFormatSet>& lineIndex_To_TextFormatSet_Map() { return m_lineIndex_To_TextFormatSet_Map; }

    // TODO: Also measure visibleLineCount()?

    const QRect& textCursorRect() const { return m_textCursorRect; }

    int lineIndexForHeight(qreal viewportYCoord) const;
    // Optionally clip for viewport?
    // TODO: Add?: QRectF rectForLine(int visibleLineIndex) const;
    // TODO: Add?: QRectF rectForPosition(const TextViewPosition& pos) const;

    struct Position
    {
        int lineIndex;
        GraphemeFinder::Result grapheme;
    };
    Position positionForPoint(const QPointF& viewportPointF,
                              GraphemeFinder::IncludeTextCursor includeTextCursor) const;

signals:
    void signalVisibleLineIndicesChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Private;
    std::shared_ptr<TextViewDocumentView> m_docView;
    std::unique_ptr<TextViewTextCursor> m_textCursor;
    std::unique_ptr<GraphemeFinder> m_graphemeFinder;
    QBrush m_selectedTextBackgroundBrush;
    QBrush m_textCursorLineBackgroundBrush;
    bool m_isAfterSetDoc;
    /**
     * Intentional: Store two copies of text cursor rect.  Why?  QWidget::update(QRect) only accept QRect.
     * <br>However, when we actually paint, we use QRectF for sub-pixel accuracy.  :)
     */
    QRect m_textCursorRect;
    QRectF m_textCursorRectF;
    int m_viewportFullyVisibleLineCount;
    int m_viewportVisibleLineCount;
    int m_firstVisibleLineIndex;
    int m_lastFullyVisibleLineIndex;
    int m_lastVisibleLineIndex;
    std::shared_ptr<PaintEventContext> m_paintEventContext;
    std::unordered_map<int, TextFormatSet> m_lineIndex_To_TextFormatSet_Map;
};

}  // namespace SDV

#endif //SDV_TEXTVIEW_H
