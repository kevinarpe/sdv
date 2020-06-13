//
// Created by kca on 10/6/2020.
//

#ifndef SDV_TEXTVIEWTEXTCURSOR_H
#define SDV_TEXTVIEWTEXTCURSOR_H

#include <QObject>
#include <QBasicTimer>
#include <QRect>
class QKeyEvent;
class QPaintEvent;

namespace SDV {

class TextView;
class TextViewDocumentView;

class TextViewTextCursor : public QObject
{
    Q_OBJECT

public:
    using Base = QObject;
    TextViewTextCursor(TextView& textView, const TextViewDocumentView& docView);

//    enum class DisplayMode { Blinking, Solid };

    /**
     * The default is true.
     *
     * @return true if text cursor blinks when widget has focus
     *
     * @see slotSetBlinking(bool)
     */
    bool isBlinking() const { return m_isBlinking; }
    /**
     * The default is QApplication::cursorFlashTime().
     *
     * @return number of milliseconds for each display phase: visible vs invisible
     */
    int blinkMillis() const { return m_blinkMillis; }
    /**
     * @return true if the text cursor is visible (solid rectangle)
     */
    bool isVisible() const { return m_isVisible; }
    int lineIndex() const { return m_lineIndex; }
    int columnIndex() const { return m_columnIndex; }
    QChar ch() const;
    QChar prevCh() const;
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool isUpdate() const { return m_isUpdate; }
    bool hasMoved() const { return m_hasMoved; }
    void afterPaintEvent();

public slots:
    /**
     * The default is true.
     *
     * @param isBlinking
     *        if true, the text cursor will blink when widget has focus
     *
     * @see isBlinking()
     */
    void slotSetBlinking(bool isBlinking);
    /**
     * The default is QApplication::cursorFlashTime().
     *
     * @param millis
     *        number of milliseconds for each display phase: visible vs invisible
     *
     * @see blinkMillis()
     */
    void slotSetBlinkMillis(int millis);

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    // TODO: Is it useful to store cursor location relative to TextView?  Ex: VISIBLE, ABOVE, BELOW
    struct Private;
    TextView& m_textView;
    const TextViewDocumentView& m_docView;
    bool m_isBlinking;
    int m_blinkMillis;
    QBasicTimer m_timer;
    bool m_isVisible;
    int m_lineIndex;
    int m_columnIndex;
    bool m_isUpdate;
    bool m_hasMoved;
    int m_prevLineIndex;
    int m_prevColumnIndex;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWTEXTCURSOR_H
