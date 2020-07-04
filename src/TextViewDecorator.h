//
// Created by kca on 29/6/2020.
//

#ifndef SDV_TEXTVIEWDECORATOR_H
#define SDV_TEXTVIEWDECORATOR_H

#include <memory>
#include <QObject>
#include <QColor>
class QLabel;

namespace SDV {

class TextView;
class JsonTree;
class JsonNode;
class TreeNodeExpander;

class TextViewDecorator : public QObject
{
    Q_OBJECT

public:
    static const QColor kTextColor;  // {Qt::GlobalColor::darkGray}

    using Base = QObject;
    TextViewDecorator(TextView& parent);
    ~TextViewDecorator() override = default;

    void setJsonTree(const std::shared_ptr<JsonTree>& jsonTree);

    /** Default: {@link #kTextColor} */
    const QColor& textColor() { return m_textColor; }
    /**
     * @param color
     *        text color of size hints that appear at open and close of objects and arrays
     */
    void setTextColor(const QColor& color);

private:
    struct Private;
    TextView& m_textView;
    QColor m_textColor;
    std::shared_ptr<JsonTree> m_jsonTree;
    struct JsonTreeNode
    {
        JsonNode* jsonNode;
        TreeNodeExpander* expander;
        QMetaObject::Connection expanderConnection;
        QLabel* sizeLabel;
    };
    std::vector<JsonTreeNode> m_freeTreeNodeVec;
    std::vector<JsonTreeNode> m_usedTreeNodeVec;
    std::unordered_map<JsonNode*, bool> m_jsonNode_To_IsExpanded_Map;
};

}  // namespace SDV

#endif //SDV_TEXTVIEWDECORATOR_H
