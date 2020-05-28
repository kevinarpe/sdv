//
// Created by kca on 4/4/2020.
//

#ifndef SDV_TEXTWIDGET_H
#define SDV_TEXTWIDGET_H

#include <QWidget>
#include <QTextLayout>
#include <QThread>
#include "FindThreadWorker.h"
#include "FindLineEdit.h"

namespace SDV {

class CircleWidget;
class FindWidget;
class GoToWidget;
class PlainTextEdit;

class TextWidget : public QWidget
{
    Q_OBJECT

public:
    static const QFont kDefaultFont; // {"Deja Vu Sans Mono", 12}
    using Base = QWidget;

    explicit TextWidget(QWidget* parent = nullptr,
                        Qt::WindowFlags flags = Qt::WindowFlags());
    ~TextWidget() override = default;

    PlainTextEdit* plainTextEdit() const { return m_plainTextEdit; }

protected:
    void showEvent(QShowEvent* event) override;
//    void hideEvent(QHideEvent* event) override;

public slots:
    void slotSetPlainText(const QString& plainText,
                          const QVector<QTextLayout::FormatRange>& formatRangeVec);
    void slotFind();
    void slotGoTo();

signals:
    // FindThreadWorker::slotFind
    void signalFindChanged(const QString& findText, bool isCaseSensitive, bool isRegex);

private:
    // Ref: https://stackoverflow.com/a/28734794/257299
    struct Private;
    CircleWidget* m_circleWidget;
    FindWidget* m_findWidget;
    GoToWidget* m_goToWidget;
    PlainTextEdit* m_plainTextEdit;
    QThread* m_findTextThread;
    // @Nullable
    FindThreadWorker* m_nullableFindTextThreadWorker;
    FindThreadWorker::Result m_lastFindResult;
    int m_lastFindResultIndex;
    // Intentional: Do not store full copy of plainText.  It may be very large.
    int m_plainTextLength;
};

}  // namespace SDV

#endif //SDV_TEXTWIDGET_H
