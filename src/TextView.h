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

//    enum class TextCursorAction {
//        Left = 1,Right,Up,Down,
//        PageUp,PageDown,
//        Ctrl_PageUp,Ctrl+PageDown,Alt+PageUp,Alt+PageDown
//    };

    // Left,Right,Up,Down
    // PageUp,PageDown,Ctrl+PageUp,Ctrl+PageDown,Alt+PageUp,Alt+PageDown
    // Home,End,Ctrl+Home,Ctrl+End
//    enum class ScrollAction {
//        /** Ctrl+Home */
//        TopLeft,
//        /** Ctrl+End */
//        BottomRight,
//        /** Home */
//        VerticalHome,
//        VerticalEnd,
//        HorizontalHome,
//        HorizontalEnd,
//        PageLeft,
//        PageRight,
//    };

    TextViewTextCursor& textCursor() { return *m_textCursor; }
    int firstVisibleLineIndex() { return m_firstVisibleLineIndex; }
    int lastFullyVisibleLineIndex() { return m_lastFullyVisibleLineIndex; }
    int lastVisibleLineIndex() { return m_lastVisibleLineIndex; }
//    const QRectF& textCursorRectF() { return m_textCursorRectF; }
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
    QRectF m_textCursorPrevRectF;
    int m_firstVisibleLineIndex;
    int m_lastFullyVisibleLineIndex;
    int m_lastVisibleLineIndex;
};

}  // namespace SDV

#endif //SDV_TEXTVIEW_H
