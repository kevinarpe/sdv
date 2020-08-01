//
// Created by kca on 24/7/2020.
//

#ifndef SDV_JSONTREERESULT_H
#define SDV_JSONTREERESULT_H

#include <memory>
#include <vector>
#include <QString>

namespace SDV {

class TextViewJsonTree;

/** Separate values-to-be-moved from other values.  */
struct JsonTreeResult
{
    std::vector<QString> jsonTextLineVec;
    std::shared_ptr<TextViewJsonTree> jsonTree;
};

}  // namespace SDV

#endif //SDV_JSONTREERESULT_H
