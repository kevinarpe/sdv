//
// Created by kca on 26/7/2020.
//

#ifndef SDV_TEXTVIEWJSONNODEPOSITIONSERVICE_H
#define SDV_TEXTVIEWJSONNODEPOSITIONSERVICE_H

#include <vector>
#include <memory>

namespace SDV {

class TextViewJsonTree;
class TextViewJsonNode;
class TextViewPosition;

class TextViewJsonNodePositionService final
{
public:
    TextViewJsonNodePositionService() {}
    explicit TextViewJsonNodePositionService(const TextViewJsonTree& jsonTree);

    // @Nullable
    std::shared_ptr<TextViewJsonNode>
    tryFind(const TextViewPosition& pos) const;

private:
    std::vector<std::shared_ptr<TextViewJsonNode>> m_jsonNodeVec;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWJSONNODEPOSITIONSERVICE_H
