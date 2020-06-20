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
    using Base = QAbstractScrollArea;
    explicit TextView(QWidget* parent = nullptr);
    ~TextView() override; // = default

    void setDoc(const std::shared_ptr<TextViewDocument>& doc);

    TextViewTextCursor& textCursor() { return *m_textCursor; }
    const TextViewTextCursor& textCursor() const { return *m_textCursor; }

    int firstVisibleLineIndex() const { return m_firstVisibleLineIndex; }
    int lastFullyVisibleLineIndex() const { return m_lastFullyVisibleLineIndex; }
    int lastVisibleLineIndex() const { return m_lastVisibleLineIndex; }

    const QRect& textCursorRect() const { return m_textCursorRect; }

    int lineIndexForHeight(qreal viewportYCoord) const;

    // TODO: Add: QRectF rectForPosition(const TextViewPosition& pos) const;

    struct Position {
        int lineIndex;
        GraphemeFinder::Result grapheme;
    };
    Position positionForPoint(const QPointF& viewportPointF, GraphemeFinder::IncludeTextCursor includeTextCursor) const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Private;
    std::shared_ptr<TextViewDocumentView> m_docView;
    std::unique_ptr<TextViewTextCursor> m_textCursor;
    std::unique_ptr<GraphemeFinder> m_graphemeFinder;
    bool m_isAfterSetDoc;
    QRect m_textCursorRect;
    QRectF m_textCursorRectF;
    int m_firstVisibleLineIndex;
    int m_lastFullyVisibleLineIndex;
    int m_lastVisibleLineIndex;
};

}  // namespace SDV

#endif //SDV_TEXTVIEW_H
