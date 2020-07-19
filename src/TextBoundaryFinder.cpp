//
// Created by kca on 16/6/2020.
//

#include "TextBoundaryFinder.h"

namespace SDV {

struct TextBoundaryFinder::Private
{
    static int
    countRange(TextBoundaryFinder& self, const int offset, const int length)
    {
        if (0 == length) {
            return 0;
        }
        self.m_finder.setPosition(offset);
        assert(self.m_finder.isAtBoundary());
        int count = 0;
        while (true)
        {
            ++count;
            const int pos = self.m_finder.toNextBoundary();
            // This is bad.  We want to assert(false) if this happens.
            assert(-1 != pos);
            if (pos >= (offset + length)) {
                break;
            }
        }
        return count;
    }
};

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

int
TextBoundaryFinder::
countAll()
{
    const int x = Private::countRange(*this, 0, m_text.length());
    return x;
}

int
TextBoundaryFinder::
countRange(const int offset, const int length)
{
    assert(offset >= 0 && (offset + length) <= m_text.length());
    const int x = Private::countRange(*this, offset, length);
    return x;
}

}  // namespace SDV
