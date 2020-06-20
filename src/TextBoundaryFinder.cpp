//
// Created by kca on 16/6/2020.
//

#include "TextBoundaryFinder.h"

namespace SDV {

void
TextBoundaryFinder::
reset(QTextBoundaryFinder::BoundaryType boundaryType, const QString& text)
{
    m_text = text;
    m_buffer.resize(text.length() + 1);
    m_finder = QTextBoundaryFinder{boundaryType, text.data(), text.length(),
                                   m_buffer.data(), static_cast<int>(m_buffer.size())};
}

// Ref: https://bugreports.qt.io/browse/QTBUG-85158
// Note: Qt5 docs say: "The range is from 0 (the beginning of the string) to the length of the string inclusive."
// This is not true.  After toNextBoundary() returns -1, position() will also be -1.  Surprise!
int
TextBoundaryFinder::
toNextBoundary()
{
    if (m_text.length() == m_finder.position()) {
        return -1;
    }
    const int x = m_finder.toNextBoundary();
    return x;
}

int
TextBoundaryFinder::
toPreviousBoundary()
{
    if (0 == m_finder.position()) {
        return -1;
    }
    const int x = m_finder.toPreviousBoundary();
    return x;
}

}  // namespace SDV
