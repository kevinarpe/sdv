//
// Created by kca on 10/6/2020.
//

#ifndef SDV_TEXTVIEWTEXTCURSOR_H
#define SDV_TEXTVIEWTEXTCURSOR_H

#include <memory>
#include <QObject>
#include <QBasicTimer>
#include <QRect>
class QKeyEvent;
class QPaintEvent;
#include "TextViewSelection.h"
#include "TextSegmentFontWidth.h"

namespace SDV {

class TextView;
class TextViewDocumentView;
class TextViewGraphemeCursor;
class TextViewGraphemePosition;

class TextViewTextCursor : public QObject
{
    Q_OBJECT

public:
    /** Used for end of line cursor */
    static const QChar SPACE_CHAR;  // QChar{QLatin1Char{' '}}
    /** Used for end of line cursor */
    static const QString SPACE_GRAPHEME;  // QString{SPACE_CHAR}

    using Base = QObject;
    TextViewTextCursor(TextView& textView, const std::shared_ptr<TextViewDocumentView>& docView);

//    enum class DisplayMode { Blinking, Solid };
    void reset();
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
     * This value alternates during each display phase: visible vs invisible.
     *
     * @return true if the text cursor is visible (solid rectangle)
     */
    bool isVisible() const { return m_isVisible; }
    const TextViewGraphemePosition& pos() const;
    const TextViewSelection& selection() const { return m_selection; }
    bool isUpdate() const { return m_isUpdate; }
    bool hasMoved() const { return m_hasMoved; }
    void afterPaintEvent();
    bool eventFilter(QObject* watched, QEvent* event) override;

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
    struct Private;
    TextView& m_textView;
    std::shared_ptr<TextViewDocumentView> m_docView;
    bool m_isBlinking;
    int m_blinkMillis;
    QBasicTimer m_timer;
    bool m_isVisible;
    // Ref: https://stackoverflow.com/a/6089065/257299
//    std::unique_ptr<TextViewGraphemeCursor> m_graphemeCursor;
    std::shared_ptr<TextViewGraphemeCursor> m_graphemeCursor;
    TextSegmentFontWidth m_fontWidth;
    TextViewSelection m_selection;
    bool m_isUpdate;
    bool m_hasMoved;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWTEXTCURSOR_H
