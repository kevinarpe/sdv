//
// Created by kca on 7/4/2020.
//

#include "FindThreadWorker.h"
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>

namespace SDV {

struct FindThreadWorker::Private
{
    static const int m_static_qRegisterMetaType;

    static std::vector<int>
    createLineOffsetVec(const QString& plainText) {
        const QChar newline{(char) '\n'};
        std::vector<int> v = {0};
        int from = 0;
        while (true) {
            const int i = plainText.indexOf(newline, from);
            if (-1 == i) {
                break;
            }
            // Intentional: We match '\n', so start of next line is 1+i.
            v.push_back(1 + i);
            from = 1 + i;
        }
        return v;
    }

    static void
    findText(FindThreadWorker& self, const QString& text, const bool isCaseSensitive)
    {
        qDebug() << QString("%1: FindThreadWorker::findText_: [%2]").arg(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs)).arg(text);
        assert(text.length() > 0);
        self.m_atomicHasWaitingSignal = false;
        const Qt::CaseSensitivity isCaseSensitive2 =
            isCaseSensitive ? Qt::CaseSensitivity::CaseSensitive : Qt::CaseSensitivity::CaseInsensitive;
        Result result;
        int lineIndex = 0;
        int from = 0;
        while (true) {
            if (self.m_atomicHasWaitingSignal) {
                qDebug() << "m_atomicHasWaitingSignal";
                self.m_atomicHasWaitingSignal = false;
                return;
            }
            int charIndex = self.m_plainText.indexOf(text, from, isCaseSensitive2);
            if (-1 == charIndex) {
                break;
            }
            from = charIndex + text.length();
            updateIndices(self, &lineIndex, &charIndex);
            result.tupleVec.emplace_back(lineIndex, charIndex, text.length());
        }
        emit self.signalFindComplete(result);
    }

    static void
    findRegex(FindThreadWorker& self, const QString& text, const bool isCaseSensitive)
    {
        qDebug() << QString("%1: FindThreadWorker::findRegex_: [%2]").arg(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODateWithMs)).arg(text);
        assert(text.length() > 0);
        self.m_atomicHasWaitingSignal = false;
        Result result;
        const QRegularExpression::PatternOption po =
            isCaseSensitive
            ? QRegularExpression::PatternOption::NoPatternOption
            : QRegularExpression::PatternOption::CaseInsensitiveOption;
        QRegularExpression regex{text, po};
//    assert(regex.isValid());
        if (regex.isValid()) {
            QRegularExpressionMatchIterator iter = regex.globalMatch(self.m_plainText);
            int lineIndex = 0;
            while (true) {
                if (self.m_atomicHasWaitingSignal) {
                    qDebug() << "m_atomicHasWaitingSignal";
                    self.m_atomicHasWaitingSignal = false;
                    return;
                }
                if ( ! iter.hasNext()) {
                    break;
                }
                QRegularExpressionMatch m = iter.next();
                const QString& match = m.captured(0);
                int charIndex = m.capturedStart(0);
                updateIndices(self, &lineIndex, &charIndex);
                result.tupleVec.emplace_back(lineIndex, charIndex, match.length());
            }
        }
        emit self.signalFindComplete(result);
    }

    static void
    updateIndices(FindThreadWorker& self, int* lineIndex, int* charIndex)
    {
        int nextLineOffset = self.m_lineOffsetVec[*lineIndex];
        const int size = self.m_lineOffsetVec.size();
        for ( ; *lineIndex + 1 < size; ++*lineIndex) {
            const int lineOffset = nextLineOffset;
            nextLineOffset = self.m_lineOffsetVec[1 + *lineIndex];
            if (*charIndex < nextLineOffset) {
                *charIndex -= lineOffset;
                return;
            }
        }
        *charIndex -= nextLineOffset;
    }
};

// Ref: https://stackoverflow.com/a/26364094/257299
// Ref: https://stackoverflow.com/a/26777364/257299
// private static
const int
FindThreadWorker::Private::
m_static_qRegisterMetaType = qRegisterMetaType<FindThreadWorker::Result>("Result");

// public explicit
FindThreadWorker::
FindThreadWorker(const QString& plainText)
    : m_atomicHasWaitingSignal{false},
      m_plainText{plainText},
      m_lineOffsetVec{Private::createLineOffsetVec(plainText)}
{}

// public slot
void
FindThreadWorker::
slotFind(const QString& text, const bool isCaseSensitive, const bool isRegex)
{
    if (isRegex) {
        Private::findRegex(*this, text, isCaseSensitive);
    }
    else {
        Private::findText(*this, text, isCaseSensitive);
    }
}

}  // namespace SDV
