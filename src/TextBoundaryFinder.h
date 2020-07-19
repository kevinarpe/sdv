//
// Created by kca on 16/6/2020.
//

#ifndef SDV_TEXTBOUNDARYFINDER_H
#define SDV_TEXTBOUNDARYFINDER_H

#include <QTextBoundaryFinder>

namespace SDV {

class TextView;
class TextViewDocumentView;

/**
 * This is a wrapper around QTextBoundaryFinder that reduces malloc/free.  Also, some minor bugs are fixed.
 */
class TextBoundaryFinder
{
public:
    TextBoundaryFinder() {}

    TextBoundaryFinder(QTextBoundaryFinder::BoundaryType boundaryType, const QString& text) {
        reset(boundaryType, text);
    }

    void reset(QTextBoundaryFinder::BoundaryType boundaryType, const QString& text);
    const QString& text() const { return m_text; }

    /** Unlike underlying QTextBoundaryFinder, this method *never* returns -1 ! */
    int position() const { return m_finder.position(); }
    void toStart() { m_finder.toStart(); }
    void toEnd() { m_finder.toEnd(); }
    int toNextBoundary();
    int toPreviousBoundary();
    int countAll();
    int countRange(int offset, int length);

private:
    struct Private;
    // Ref: https://stackoverflow.com/a/3483026/257299
    std::vector<unsigned char> m_buffer;
    // Intentional: It is unsafe to call QString::data() without storing a copy.
    QString m_text;
    QTextBoundaryFinder m_finder;
};

}  // namespace SDV

#endif //SDV_TEXTBOUNDARYFINDER_H
