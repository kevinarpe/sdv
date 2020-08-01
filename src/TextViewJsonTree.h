//
// Created by kca on 25/7/2020.
//

#ifndef SDV_TEXTVIEWJSONTREE_H
#define SDV_TEXTVIEWJSONTREE_H

#include <memory>
#include <vector>

namespace SDV {

class TextViewJsonNode;

class TextViewJsonTree
{
public:
    TextViewJsonTree(std::shared_ptr<TextViewJsonNode>&& rootJsonNode,
                     std::vector<std::vector<std::shared_ptr<TextViewJsonNode>>>&& lineIndex_To_NodeVec)
        : m_rootJsonNode{std::move(rootJsonNode)},
          m_lineIndex_To_NodeVec{std::move(lineIndex_To_NodeVec)}
    {}

    const TextViewJsonNode& rootJsonNode() const { return *m_rootJsonNode; }
    const std::shared_ptr<TextViewJsonNode>& rootJsonNodePtr() const { return m_rootJsonNode; }
    const std::vector<std::vector<std::shared_ptr<TextViewJsonNode>>>& lineIndex_To_NodeVec() const { return m_lineIndex_To_NodeVec; }

private:
    std::shared_ptr<TextViewJsonNode> m_rootJsonNode;
    /** All nodes are owned indirectly by {@link #m_rootJsonNode}. */
    std::vector<std::vector<std::shared_ptr<TextViewJsonNode>>> m_lineIndex_To_NodeVec;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWJSONTREE_H
