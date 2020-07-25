//
// Created by kca on 24/7/2020.
//

#ifndef SDV_JSONTREERESULT_H
#define SDV_JSONTREERESULT_H

#include <memory>
#include <vector>
#include <QString>

namespace SDV {

class JsonTree;

/** Separate values-to-be-moved from other values.  */
struct JsonTreeResult
{
    JsonTreeResult();

    std::vector<QString> jsonTextLineVec;
    std::shared_ptr<JsonTree> jsonTree;
};

}  // namespace SDV

#endif //SDV_JSONTREERESULT_H
