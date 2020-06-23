//
// Created by kca on 6/6/2020.
//

#ifndef SDV_TEXTVIEW_H
#define SDV_TEXTVIEW_H

#include <memory>
#include <QAbstractScrollArea>
#include "GraphemeFinder.h"

namespace SDV {

class TextViewDocument;
class TextViewDocumentView;
class TextViewTextCursor;

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

    int fullyVisibleLineCount() const { return m_fullyVisibleLineCount; }
    int visibleLineCount() const { return m_visibleLineCount; }
    int firstVisibleLineIndex() const { return m_firstVisibleLineIndex; }
    int lastFullyVisibleLineIndex() const { return m_lastFullyVisibleLineIndex; }
    int lastVisibleLineIndex() const { return m_lastVisibleLineIndex; }

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
    int m_fullyVisibleLineCount;
    int m_visibleLineCount;
    int m_firstVisibleLineIndex;
    int m_lastFullyVisibleLineIndex;
    int m_lastVisibleLineIndex;
};

}  // namespace SDV

#endif //SDV_TEXTVIEW_H
