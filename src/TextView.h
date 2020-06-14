//
// Created by kca on 6/6/2020.
//

#ifndef SDV_TEXTVIEW_H
#define SDV_TEXTVIEW_H

#include <QAbstractScrollArea>
#include <QBasicTimer>
#include <vector>
#include <memory>

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
    int firstVisibleLineIndex() { return m_firstVisibleLineIndex; }
    int lastFullyVisibleLineIndex() { return m_lastFullyVisibleLineIndex; }
    int lastVisibleLineIndex() { return m_lastVisibleLineIndex; }
    const QRect& textCursorRect() { return m_textCursorRect; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Private;
    std::unique_ptr<TextViewDocumentView> m_docView;
    std::unique_ptr<TextViewTextCursor> m_textCursor;
    bool m_isAfterSetDoc;
    QRect m_textCursorRect;
    QRectF m_textCursorRectF;
    int m_firstVisibleLineIndex;
    int m_lastFullyVisibleLineIndex;
    int m_lastVisibleLineIndex;
};

}  // namespace SDV

#endif //SDV_TEXTVIEW_H
