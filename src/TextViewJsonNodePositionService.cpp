//
// Created by kca on 26/7/2020.
//

#include <QDebug>
#include "TextViewJsonNodePositionService.h"
#include "TextViewJsonTree.h"
#include "TextViewJsonNode.h"

namespace SDV {

static void 
visit(const std::shared_ptr<TextViewJsonNode>& jsonNode,
      std::vector<std::shared_ptr<TextViewJsonNode>>& jsonNodeVec)
{
    const std::vector<std::shared_ptr<TextViewJsonNode>>& childVec = jsonNode->childVec();
    jsonNodeVec.reserve(1 + childVec.size());
    jsonNodeVec.push_back(jsonNode);

    for (const std::shared_ptr<TextViewJsonNode>& child : childVec)
    {
        visit(child, jsonNodeVec);
    }
}

// public explicit
TextViewJsonNodePositionService::
TextViewJsonNodePositionService(const TextViewJsonTree& jsonTree)
{
    const std::shared_ptr<TextViewJsonNode>& rootJsonNode = jsonTree.rootJsonNodePtr();
    visit(rootJsonNode, m_jsonNodeVec);
}

struct Compare
{
    bool operator()(const std::shared_ptr<TextViewJsonNode>& jsonNode, 
                    const TextViewPosition& pos)
    {
        const int c = jsonNode->compare(pos);
        const bool x = (c < 0);
        return x;
    }
};

// public
// @Nullable
std::shared_ptr<TextViewJsonNode>
TextViewJsonNodePositionService::
tryFind(const TextViewPosition& pos)
const
{
    auto iter = std::lower_bound(m_jsonNodeVec.begin(), m_jsonNodeVec.end(), pos, Compare{});
    if (m_jsonNodeVec.end() != iter)
    {
        const std::shared_ptr<TextViewJsonNode>& jsonNode = *iter;
        const int c = jsonNode->compare(pos);
        if (0 == c) {
//            qDebug() << "match:" << jsonNode->text();
            return jsonNode;
        }
//        else {
//            qDebug() << "non-match:" << jsonNode->text();
//        }
    }
//    else {
//        qDebug() << "iter == end";
//    }
    return std::shared_ptr<TextViewJsonNode>{};
}

}  // namespace SDV
