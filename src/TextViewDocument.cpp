//
// Created by kca on 5/6/2020.
//

#include "TextViewDocument.h"

namespace SDV {

// private static
std::shared_ptr<TextViewDocument> TextViewDocument::s_empty = std::make_shared<TextViewDocument>(std::vector<QString>{{QString{}}});

}  // namespace SDV
