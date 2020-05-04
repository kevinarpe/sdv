//
// Created by kca on 4/4/2020.
//

#include "TextWidget.h"
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <QLineEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDebug>
#include <QDateTime>
#include <QPainter>
#include "FindWidget.h"
#include "GoToWidget.h"
#include "LineNumberAreaWidget.h"
#include "PlainTextEdit.h"
#include "CircleWidget.h"

namespace SDV {

// Ref: https://stackoverflow.com/a/28734794/257299
struct TextWidget::Private
{
    static void
    reset(TextWidget& self)
    {
        self.m_findTextThreadWorker = nullptr;
        self.m_lastFindResultIndex = -1;
        self.m_plainTextLength = -1;
    }

    static void
    removeLastFindResultFormatting(TextWidget& self)
    {
        if (self.m_lastFindResult.tupleVec.empty()) {
            return;
        }
        QTextDocument* const doc = self.m_plainTextEdit->document();
        for (const FindThreadWorker::Result::Tuple& tuple : self.m_lastFindResult.tupleVec) {
            const QTextBlock& textBlock = doc->findBlockByLineNumber(tuple.lineIndex);
//            const QTextBlock& textBlock2 = doc->findBlockByNumber(tuple.lineIndex);
//            assert(textBlock == textBlock2);
            QTextLayout* const textLayout = textBlock.layout();
            QVector<QTextLayout::FormatRange> formatRangeVec = textLayout->formats();
            formatRangeVec.removeLast();
            textLayout->setFormats(formatRangeVec);
        }
    }

    static void
    applyLastFindResultFormatting(TextWidget& self, int* nullableLastFindResultIndex)
    {
        if (nullptr != nullableLastFindResultIndex) {
            *nullableLastFindResultIndex = -1;
        }
        if (self.m_lastFindResult.tupleVec.empty()) {
            return;
        }
        const int firstVisibleBlockNumber = getFirstVisibleBlockNumber(self);
        // Intentional: -1: Assume last visible line may only be *partially* visible.
        const int lastVisibleBlockNumber = getLastVisibleBlockNumber(self) - 1;
        bool foundFirstVisibleMatch = false;

        const int tupleCount = self.m_lastFindResult.tupleVec.size();
        for (int i = 0; i < tupleCount; ++i) {
            const FindThreadWorker::Result::Tuple& tuple = self.m_lastFindResult.tupleVec[i];
            const QTextBlock& textBlock = self.m_plainTextEdit->document()->findBlockByLineNumber(tuple.lineIndex);
//            const QTextBlock& textBlock2 = doc->findBlockByNumber(tuple.lineIndex);
//            assert(textBlock == textBlock2);
            QTextLayout* const textLayout = textBlock.layout();
            QTextLayout::FormatRange formatRange;
            formatRange.start = tuple.charIndex;
            formatRange.length = tuple.length;
            if (!foundFirstVisibleMatch
                && tuple.lineIndex >= firstVisibleBlockNumber
                && tuple.lineIndex <= lastVisibleBlockNumber) {

                foundFirstVisibleMatch = true;
                QTextCursor currTextCursor = self.m_plainTextEdit->textCursor();
                const int endPos = textBlock.position() + tuple.charIndex + tuple.length;
                currTextCursor.setPosition(endPos, QTextCursor::MoveMode::MoveAnchor);
                self.m_plainTextEdit->setTextCursor(currTextCursor);
                self.m_plainTextEdit->ensureCursorVisible();
                formatRange.format.setBackground(kBrushOrange);
                if (nullptr != nullableLastFindResultIndex) {
                    *nullableLastFindResultIndex = i;
                }
            }
            else {
                formatRange.format.setBackground(kBrushYellow);
            }
            QVector<QTextLayout::FormatRange> formatRangeVec = textLayout->formats();
            formatRangeVec.append(std::move(formatRange));
            textLayout->setFormats(formatRangeVec);
        }
        if ( ! foundFirstVisibleMatch && tupleCount > 0) {
            const FindThreadWorker::Result::Tuple& tuple0 = self.m_lastFindResult.tupleVec[0];
            // Intentional: -1 for some "UI breathing room".  :-)
            self.m_plainTextEdit->verticalScrollBar()->setValue(tuple0.lineIndex - 1);

            const QTextBlock& textBlock = self.m_plainTextEdit->document()->findBlockByLineNumber(tuple0.lineIndex);
            QTextLayout* const textLayout = textBlock.layout();
            QVector<QTextLayout::FormatRange> formatRangeVec = textLayout->formats();
            // If there are multiple matches (and formats) on the first matching line, then use first.
            formatRangeVec[0].format.setBackground(kBrushOrange);
            textLayout->setFormats(formatRangeVec);
            if (nullptr != nullableLastFindResultIndex) {
                *nullableLastFindResultIndex = 0;
            }

            QTextCursor textCursor{self.m_plainTextEdit->document()};
            const int endPos = textBlock.position() + tuple0.charIndex + tuple0.length;
            textCursor.setPosition(endPos, QTextCursor::MoveMode::MoveAnchor);
            self.m_plainTextEdit->setTextCursor(textCursor);
            self.m_plainTextEdit->ensureCursorVisible();
        }
    }

    static void
    afterUpdateDocumentFormats(TextWidget& self)
    {
        self.m_plainTextEdit->document()->markContentsDirty(0, self.m_plainTextLength);
    }

    static void
    updateMatchCountLabel(TextWidget& self)
    {
        if (self.m_findWidget->findLineEdit()->text().isEmpty()) {
            self.m_findWidget->matchCountLabel()->setText(QString{});
        }
        else {
            const QString& text =
                QString("%1 / %2").arg(1 + self.m_lastFindResultIndex).arg(self.m_lastFindResult.tupleVec.size());
            self.m_findWidget->matchCountLabel()->setText(text);
        }
    }

    static void
    nextMatch(TextWidget& self, const int change)
    {
        if (self.m_lastFindResultIndex < 0) {
            return;
        }
        if (-1 == change && 0 == self.m_lastFindResultIndex) {
            // ??
        }
        else if (+1 == change && self.m_lastFindResult.tupleVec.size() - 1 == self.m_lastFindResultIndex) {
            // ??
        }
        Private::setBackground(self, self.m_lastFindResultIndex, kBrushYellow);
        const int nextIndex = nextFindResultIndex(self, change);
        Private::setBackground(self, nextIndex, kBrushOrange);
        self.m_lastFindResultIndex = nextIndex;
        Private::updateMatchCountLabel(self);
        const int lineIndex = self.m_lastFindResult.tupleVec[nextIndex].lineIndex;
        QScrollBar* const vbar = self.m_plainTextEdit->verticalScrollBar();
        // Intentional: +1 for "breathing room"
        if (lineIndex < vbar->value() + 1) {
            vbar->setValue(lineIndex - 1);
        }
        else {
            // Intentional: -1: Assume last visible line may only be *partially* visible.
            const int lastVisibleBlockNumber = getLastVisibleBlockNumber(self) - 1;
            if (lineIndex > lastVisibleBlockNumber) {
                const int firstVisibleBlockNumber = getFirstVisibleBlockNumber(self);
                const int visibleBlockCount = lastVisibleBlockNumber - firstVisibleBlockNumber + 1;
                // Recall: VBar value is *first* visible line.
                vbar->setValue(lineIndex + 1 - visibleBlockCount);
            }
        }
    }

    // Ref: https://forum.qt.io/topic/74327/qtextedit-how-to-find-the-visible-part
    static int
    getFirstVisibleBlockNumber(const TextWidget& self)
    {
        const int x = self.m_plainTextEdit->cursorForPosition(QPoint{0, 0}).block().blockNumber();
        return x;
    }

    static int
    getLastVisibleBlockNumber(const TextWidget& self)
    {
        const QPoint point{0, self.m_plainTextEdit->viewport()->height() - 1};
        const QTextBlock& textBlock = self.m_plainTextEdit->cursorForPosition(point).block();
//        qDebug() << QString("last visible text: [%1]").arg(textBlock.text());
        const int x = textBlock.blockNumber();
        return x;
    }

    static int
    nextFindResultIndex(const TextWidget& self, const int change)
    {
        int x = self.m_lastFindResultIndex + change;
        if (x < 0) {
            return self.m_lastFindResult.tupleVec.size() - 1;
        }
        else if (x >= self.m_lastFindResult.tupleVec.size()) {
            return 0;
        }
        else {
            return x;
        }
    }

    static void
    setBackground(TextWidget& self, const int findResultIndex, const QBrush& brush)
    {
        const FindThreadWorker::Result::Tuple& tuple = self.m_lastFindResult.tupleVec[findResultIndex];
        const QTextBlock& textBlock = self.m_plainTextEdit->document()->findBlockByLineNumber(tuple.lineIndex);
        QTextLayout* const textLayout = textBlock.layout();
        QVector<QTextLayout::FormatRange> formatRangeVec = textLayout->formats();
        QTextLayout::FormatRange* formatRange =
            std::find_if(formatRangeVec.begin(), formatRangeVec.end(),
                         [&tuple](const QTextLayout::FormatRange& formatRange) -> bool {
                             return (formatRange.start == tuple.charIndex &&
                                     formatRange.length == tuple.length);
                         });
        assert(formatRange != formatRangeVec.end());
        formatRange->format.setBackground(brush);
        textLayout->setFormats(formatRangeVec);
        self.m_plainTextEdit->document()->
            markContentsDirty(textBlock.position() + formatRange->start, formatRange->length);
    }

    static void
    slotGoToLineColumn(TextWidget& self, const int lineNumber, const int columnNumber)
    {
        qDebug() << "slotGoToLineColumn()";
    }

    static void
    slotGoToOffset(TextWidget& self, const int offset)
    {
        qDebug() << "slotGoToOffset()";
    }

    static void
    slotGoToWidgetHidden(TextWidget& self)
    {
        qDebug() << "slotGoToWidgetHidden()";
    }

    // FindWidget::signalQueryChanged
    static void
    slotFindQueryChanged(TextWidget& self, const QString& findText, const bool isCaseSensitive, const bool isRegex)
    {
        qDebug() << QString("%1: TextWidget::slotFindQueryChanged_: [%2]").arg(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs)).arg(findText);
        if (findText.isEmpty()) {
            removeLastFindResultFormatting(self);
            afterUpdateDocumentFormats(self);
            self.m_lastFindResult = FindThreadWorker::Result{};
            self.m_findWidget->matchCountLabel()->setText(QString{});
        }
        else {
            if ( ! self.m_findTextThread->isRunning()) {
                self.m_findTextThread->start();
            }
            self.m_findTextThreadWorker->beforeEmitSignal();
            emit self.signalFindChanged(findText, isCaseSensitive, isRegex);
        }
    }

    // FindThreadWorker::signalFindComplete
    static void
    slotFindComplete(TextWidget& self, const FindThreadWorker::Result& result)
    {
        qDebug() << QString("%1: TextWidget::slotFindComplete_").arg(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs));
        Private::removeLastFindResultFormatting(self);
        self.m_lastFindResult = result;
        Private::applyLastFindResultFormatting(self, &self.m_lastFindResultIndex);
        Private::afterUpdateDocumentFormats(self);
        Private::updateMatchCountLabel(self);
    }

    static void
    slotFindOrGoToWidgetSpecialKeyPressed(TextWidget& self, const FindLineEdit::KeySequence keySequence)
    {
        QScrollBar* const vbar = self.m_plainTextEdit->verticalScrollBar();
        if (FindLineEdit::KeySequence::PageUp == keySequence) {
            vbar->setValue(vbar->value() - vbar->pageStep());
        }
        else if (FindLineEdit::KeySequence::PageDown == keySequence) {
            vbar->setValue(vbar->value() + vbar->pageStep());
        }
        else if (FindLineEdit::KeySequence::CtrlHome == keySequence) {
            self.m_plainTextEdit->slotScrollTopLeft();
        }
        else if (FindLineEdit::KeySequence::CtrlEnd == keySequence) {
            self.m_plainTextEdit->slotScrollBottomRight();
        }
        else if (FindLineEdit::KeySequence::AltPageUp == keySequence) {
            self.m_plainTextEdit->slotScrollPageLeft();
        }
        else if (FindLineEdit::KeySequence::AltPageDown == keySequence) {
            self.m_plainTextEdit->slotScrollPageRight();
        }
        else {
            assert(false);
        }
    }

    static void
    slotNextMatch(TextWidget& self)
    {
        nextMatch(self, +1);
    }

    static void
    slotPrevMatch(TextWidget& self)
    {
        nextMatch(self, -1);
    }

    static void
    slotFindWidgetHidden(TextWidget& self)
    {
        removeLastFindResultFormatting(self);
        afterUpdateDocumentFormats(self);
    }

//    static const QFont kDefaultFont; // {"Deja Vu Sans Mono", 12}
    static const QBrush kBrushYellow; // {QColor{255, 255, 0}};
    static const QBrush kBrushOrange; // {QColor{255, 192, 0}};
};

// public static
const QBrush
TextWidget::Private::
//    const QBrush brushYellow = QBrush{QColor{255, 255, 128}};
kBrushYellow{QColor{255, 255, 0}};

// public static
const QBrush
TextWidget::Private::
kBrushOrange{QColor{255, 192, 0}};

// public static
const QFont
TextWidget::
kDefaultFont{"Deja Vu Sans Mono", 12};

// public explicit
TextWidget::
TextWidget(QWidget* parent /*= nullptr*/,
           Qt::WindowFlags flags /*= Qt::WindowFlags()*/)
    : QWidget(parent, flags)
{
    Private::reset(*this);

    m_circleWidget = new CircleWidget{this};
    m_circleWidget->setGeometry(QRect{320, 240, 50, 50});

    m_findWidget = new FindWidget{};
    m_findWidget->setVisible(false);
    m_findWidget->findLineEdit()->setFont(kDefaultFont);
    QObject::connect(m_findWidget, &FindWidget::signalQueryChanged,
                     [this](const QString& findText, bool isCaseSensitive, bool isRegex)
                     { Private::slotFindQueryChanged(*this, findText, isCaseSensitive, isRegex); });
    QObject::connect(m_findWidget, &FindWidget::signalSpecialKeyPressed,
                     [this](FindLineEdit::KeySequence keySequence)
                     { Private::slotFindOrGoToWidgetSpecialKeyPressed(*this, keySequence); });
    QObject::connect(m_findWidget, &FindWidget::signalNextMatch,
                     [this]() { Private::slotNextMatch(*this); });
    QObject::connect(m_findWidget, &FindWidget::signalPrevMatch,
                     [this]() { Private::slotPrevMatch(*this); });
    QObject::connect(m_findWidget, &FindWidget::signalHidden,
                     [this]() { Private::slotFindWidgetHidden(*this); });

    m_goToWidget = new GoToWidget{};
    m_goToWidget->setVisible(false);
    m_goToWidget->findLineEdit()->setFont(kDefaultFont);
    QObject::connect(m_goToWidget, &GoToWidget::signalSpecialKeyPressed,
                     [this](FindLineEdit::KeySequence keySequence)
                     { Private::slotFindOrGoToWidgetSpecialKeyPressed(*this, keySequence); });
    QObject::connect(m_goToWidget, &GoToWidget::signalGoToLineColumn,
                     [this](int lineNumber, int columnNumber)
                     { Private::slotGoToLineColumn(*this, lineNumber, columnNumber); });
    QObject::connect(m_goToWidget, &GoToWidget::signalGoToOffset,
                     [this](int offset) { Private::slotGoToOffset(*this, offset); });
    QObject::connect(m_goToWidget, &GoToWidget::signalHidden,
                     [this]() { Private::slotGoToWidgetHidden(*this); });

    m_plainTextEdit = new PlainTextEdit{};
//    const Qt::TextInteractionFlags& f1 = m_plainTextEdit->textInteractionFlags();
    m_plainTextEdit->setReadOnly(true);
//    const Qt::TextInteractionFlags& f2 = m_plainTextEdit->textInteractionFlags();
//    m_plainTextEdit->setTextInteractionFlags(m_plainTextEdit->textInteractionFlags() | Qt::TextInteractionFlag::TextSelectableByKeyboard);
//    const Qt::TextInteractionFlags& f3 = m_plainTextEdit->textInteractionFlags();
    m_plainTextEdit->setLineWrapMode(QPlainTextEdit::LineWrapMode::NoWrap);
//    m_plainTextEdit->setFont(QFont{"Deja Vu Sans Mono", 12});
    m_plainTextEdit->setFont(kDefaultFont);
//    m_plainTextEdit->lineNumberAreaWidget()->setFont(kDefaultFont);

    m_findTextThread = new QThread{this};

    QHBoxLayout* hboxLayout = new QHBoxLayout{};
    hboxLayout->setContentsMargins(0, 0, 0, 0);
    hboxLayout->setSpacing(0);
    hboxLayout->addWidget(m_plainTextEdit->lineNumberAreaWidget());
    hboxLayout->addWidget(m_plainTextEdit);

    QVBoxLayout* vboxLayout = new QVBoxLayout{};
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    vboxLayout->setSpacing(0);
    vboxLayout->addWidget(m_findWidget);
    vboxLayout->addWidget(m_goToWidget);
    vboxLayout->addLayout(hboxLayout);
    setLayout(vboxLayout);
}

// protected virtual
void
TextWidget::
showEvent(QShowEvent* event) // override
{
    m_plainTextEdit->setFocus();
//    m_circleWidget->raise();
    Base::showEvent(event);
}

// public slot
void
TextWidget::
slotSetPlainText(const QString& plainText,
                 const QVector<QTextLayout::FormatRange>& formatRangeVec)
{
    // Intentional: This method may only be called once!
    assert(m_plainTextLength < 0);
    m_plainTextLength = plainText.length();
    m_plainTextEdit->setPlainText(plainText);
    QTextDocument* const doc = m_plainTextEdit->document();
    const int formatRangeCount = formatRangeVec.size();
    // Two is the maximum number of expected formats in a single line.  When/Why?  A key-value pair.
    for (int i = 0; i < formatRangeCount; i += 0) {
        const QTextLayout::FormatRange& formatRange0 = formatRangeVec[i];
        const QTextBlock& textBlock = doc->findBlock(formatRange0.start);
        assert(textBlock.isValid());
        const int position = textBlock.position();
        const int length = textBlock.length();
        QTextLayout* const textLayout = textBlock.layout();
        // Always empty, but remember QVector uses implicit sharing.
        QVector<QTextLayout::FormatRange> textBlockFormatRangeVec = textLayout->formats();
        for (int j = i; j < formatRangeCount; ++j) {
            const QTextLayout::FormatRange& formatRange = formatRangeVec[j];
            if (formatRange.start < position || formatRange.start >= position + length) {
                break;
            }
            QTextLayout::FormatRange formatRangeCopy{formatRange};
            formatRangeCopy.start -= position;
            textBlockFormatRangeVec.append(std::move(formatRangeCopy));
            ++i;
        }
        textLayout->setFormats(textBlockFormatRangeVec);
    }
    Private::afterUpdateDocumentFormats(*this);

    m_findTextThreadWorker = new FindThreadWorker(plainText);
    // Ref: https://doc.qt.io/qt-5/qthread.html#details
    m_findTextThreadWorker->moveToThread(m_findTextThread);
    QObject::connect(m_findTextThread, &QThread::finished, m_findTextThreadWorker, &QObject::deleteLater);
    QObject::connect(this, &TextWidget::signalFindChanged, m_findTextThreadWorker, &FindThreadWorker::slotFind);
    QObject::connect(
        m_findTextThreadWorker, &FindThreadWorker::signalFindComplete,
        [this](const FindThreadWorker::Result& result) { Private::slotFindComplete(*this, result); });
}

// public slot
void
TextWidget::
slotFind()
{
    if (m_findWidget->isVisible()) {
        m_findWidget->findLineEdit()->setFocus();
    }
    else {
        m_findWidget->setVisible(true);
        Private::applyLastFindResultFormatting(*this, nullptr);
        Private::afterUpdateDocumentFormats(*this);
    }
}

// public slot
void
TextWidget::
slotGoTo()
{
    if (m_goToWidget->isVisible()) {
        m_goToWidget->findLineEdit()->setFocus();
    }
    else {
        m_goToWidget->setVisible(true);
//        Private::applyLastFindResultFormatting(*this, nullptr);
//        Private::afterUpdateDocumentFormats(*this);
    }
}

// protected
void
TextWidget::
hideEvent(QHideEvent* event) // override
{
    QWidget::hideEvent(event);
    Private::reset(*this);
}

}  // namespace SDV
