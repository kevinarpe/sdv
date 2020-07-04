//
// Created by kca on 5/6/2020.
//

#include "TextViewDocument.h"

namespace SDV {

static const QString kEmptyLine{};

struct TextViewDocument::Private
{
    // Why not use std::make_shared()?  It does not work with private ctors. :(
    // Ref: https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
    static std::shared_ptr<TextViewDocument>
    make_shared()
    {
        std::shared_ptr<TextViewDocument> p{new TextViewDocument{}};
        return p;
    }
};

// private static
std::shared_ptr<TextViewDocument> TextViewDocument::s_empty = Private::make_shared();

//const QString&
//TextViewDocument::
//line(const int lineIndex)
//const
//{
//    if (m_lineVec.empty() && 0 == lineIndex) {
//        return kEmptyLine;
//    }
//    else {
//        const QString& x = m_lineVec[lineIndex];
//        return x;
//    }
//}

}  // namespace SDV
