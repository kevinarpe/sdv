//
// Created by kca on 5/6/2020.
//

#include "TextViewDocument.h"

namespace SDV {

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

}  // namespace SDV
