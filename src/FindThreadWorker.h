//
// Created by kca on 7/4/2020.
//

#ifndef SDV_FINDTHREADWORKER_H
#define SDV_FINDTHREADWORKER_H

#include <QObject>
#include <atomic>
#include <vector>

namespace SDV {

class FindThreadWorker : public QObject
{
    Q_OBJECT

public:
    explicit FindThreadWorker(const QString& plainText);

    struct Result {
        // See: QTextLayout::FormatRange::start,length
        struct Tuple {
            int lineIndex;
            /** QChar index for {@link #lineIndex} */
            int charIndex;
            int length;
            Tuple(int lineIndex_, int charIndex_, int length_)
                : lineIndex(lineIndex_), charIndex(charIndex_), length(length_)
            {}
        };
        std::vector<Tuple> tupleVec;
    };

    void beforeEmitSignal() {
        m_atomicHasWaitingSignal = true;
    }

signals:
    // Ref: https://stackoverflow.com/a/10405871/257299
    // Ref: https://www.embeddeduse.com/2013/06/29/copied-or-not-copied-arguments-signals-slots/
    void signalFindComplete(const Result& result);

public slots:
    void slotFind(const QString& text, bool isCaseSensitive, bool isRegex);

private:
    struct Private;
    std::atomic<bool> m_atomicHasWaitingSignal;
    const QString m_plainText;
    const std::vector<int> m_lineOffsetVec;
};

}  // namespace SDV

#endif //SDV_FINDTHREADWORKER_H
