//
// Created by kca on 29/6/2020.
//

#include "TextViewDecorator.h"
#include <QLabel>
#include <QScrollBar>
#include <QKeyEvent>
#include <QDebug>
#include "TextView.h"
#include "TextViewJsonTree.h"
#include "TreeNodeExpander.h"
#include "TextViewJsonNode.h"
#include "Algorithm.h"
#include "TextViewPosition.h"
#include "TextViewDocumentView.h"
#include "TextViewDocument.h"
#include "QKeyEvents.h"
#include "TextViewTextCursor.h"

namespace SDV {

// public static
const QColor TextViewDecorator::kTextColor{Qt::GlobalColor::darkGray};

enum class StopEventPropagation { Yes, No };

struct TextViewDecorator::Private
{
    static void
    slotHorizontalScrollBarValueChanged(TextViewDecorator& self, const int value)
    {
        update(self);
    }

    static void
    update(TextViewDecorator& self)
    {
        assert(nullptr != self.m_jsonTree);
        // TODO: Do not always call.  The algo should be more selective.
        // Example: "Check" each visible node.  After first incorrect one is found, hide and all after.
        // This will reduce event "noise" of hide, then reshow same widget(s)!
        hideAllUsedTreeNodes(self);
        const QFontMetricsF& fontMetricsF = QFontMetricsF{self.m_textView.font()};
        const qreal lineSpacing = fontMetricsF.lineSpacing();
        const int firstVisibleLineIndex = self.m_textView.firstVisibleLineIndex();
        const int lastVisibleLineIndex = self.m_textView.lastVisibleLineIndex();
        const std::vector<QString>& lineVec = self.m_textView.docView().doc().lineVec();

        const TextViewDocumentView::const_iterator firstIter = self.m_textView.docView().findOrAssert(firstVisibleLineIndex);
        const TextViewDocumentView::const_iterator lastIter = self.m_textView.docView().findOrAssert(lastVisibleLineIndex);
        // If hbar->value() is positive, then shift left.
        const qreal x = -1.0 * self.m_textView.horizontalScrollBar()->value();

        // Note: operator<= on random access iterator.  This will not work if we switch to unordered_set later.  :(
        for (auto lineIndexIter = firstIter; lineIndexIter <= lastIter; ++lineIndexIter)
        {
            const int lineIndex = *lineIndexIter;
            const std::vector<std::shared_ptr<TextViewJsonNode>>& nodeVec = self.m_jsonTree->lineIndex_To_NodeVec()[lineIndex];
            // TODO: Only check last?
            for (auto jsonNodeIter = nodeVec.rbegin(); jsonNodeIter != nodeVec.rend(); ++jsonNodeIter)
            {
                const std::shared_ptr<TextViewJsonNode>& jsonNode = *jsonNodeIter;
                if (false == canExpand(jsonNode)) {
                    continue;
                }
                const QString& line = lineVec[lineIndex];
                const int indexOfExpanderPosition = Private::indexOfExpanderPosition(line);
                const JsonTreeNode& node = getFreeNodeConstRef(self, lineSpacing, jsonNode, lineIndex);
                const qreal y = self.m_textView.heightForVisibleLineIndex(lineIndex);
                {
                    const qreal width = fontMetricsF.horizontalAdvance(line, indexOfExpanderPosition);
                    node.expander->move(QPointF{x + width, y}.toPoint());
                }
                {
                    const qreal width = fontMetricsF.horizontalAdvance(line);
                    node.sizeLabel->move(QPointF{x + width, y}.toPoint());
                }
                break;
            }
        }
    }

    static void
    hideAllUsedTreeNodes(TextViewDecorator& self)
    {
        if (self.m_lineIndex_To_JsonTreeNode_Map.empty()) {
            return;
        }
        self.m_freeTreeNodeVec.reserve(self.m_freeTreeNodeVec.size() + self.m_lineIndex_To_JsonTreeNode_Map.size());

        for (auto iter = self.m_lineIndex_To_JsonTreeNode_Map.begin()
            ; self.m_lineIndex_To_JsonTreeNode_Map.end() != iter
            ; ++iter)
        {
            JsonTreeNode& node = iter->second;
            node.jsonNode = nullptr;
            node.expander->setVisible(false);
            if (node.expanderConnection) {
                QObject::disconnect(node.expanderConnection);
            }
            node.sizeLabel->setVisible(false);

            self.m_freeTreeNodeVec.push_back(node);
        }
        self.m_lineIndex_To_JsonTreeNode_Map.clear();
    }

    static bool
    canExpand(const std::shared_ptr<TextViewJsonNode>& jsonNode)
    {
        // (1) Do not allow root to be collapsed.
        const bool x = (nullptr != jsonNode->nullableParent()
                        && (JsonNodeType::ObjectBegin == jsonNode->type() || JsonNodeType::ArrayBegin == jsonNode->type())
                        // Remember: Every object and array has at least one child: '}' or ']'
                        && jsonNode->childVec().size() > 1);
        return x;
    }

    static bool
    isExpanded(const TextViewDecorator& self,
               const std::shared_ptr<TextViewJsonNode>& jsonNode)
    {
        const bool x = Algorithm::Map::getOrDefault(self.m_jsonNode_To_IsExpanded_Map, jsonNode, true);
        return x;
    }

    // TODO: THIS IS DUMB.  JUST PLACE THE WIDGET THERE YOU WANT IT!  LEFT EDGE MINUS (EXPANDER WIDGET WIDTH + MARGIN)
    static int
    indexOfExpanderPosition(const QString& text)
    {
        const size_t i = Algorithm::findFirstIndexIf(text, [](const QChar& ch) { return false == ch.isSpace(); });
        assert(i >= 2);
        // Left-edge to left-edge distance of two leading spaces seems to look good.
        // Why two?  Most fonts are taller than wider, but less than 2.0 ratio.
        return i - 2;
    }

    // Intentional: "Loud" method name to match return type.
    static const JsonTreeNode&
    getFreeNodeConstRef(TextViewDecorator& self,
                        const qreal lineSpacing,
                        const std::shared_ptr<TextViewJsonNode>& jsonNode,
                        const int lineIndex)
    {
        JsonTreeNode& node = getFreeNodeRef(self, lineSpacing, jsonNode, lineIndex);

        // Intentional: Set expanded before signal-slot connect.
        const bool isExpanded = Private::isExpanded(self, jsonNode);
        node.expander->slotSetExpanded(isExpanded);

        node.expanderConnection =
            QObject::connect(node.expander, &TreeNodeExpander::signalExpandedChanged,
                             [&self, node](const bool isExpanded) { slotSetExpanded(self, node, isExpanded); });
        return node;
    }

    // Intentional: "Loud" method name to match return type.
    static JsonTreeNode&
    getFreeNodeRef(TextViewDecorator& self,
                   const qreal lineSpacing,
                   const std::shared_ptr<TextViewJsonNode>& jsonNode,
                   const int lineIndex)
    {
        JsonTreeNode nodeValue = getFreeNodeValue(self, lineSpacing, jsonNode);
        const auto pair = Algorithm::Map::insertNewOrAssert(self.m_lineIndex_To_JsonTreeNode_Map, lineIndex, nodeValue);
        auto iter = pair.first;
        // Very important: We modify *value* 'expanderConnection' in the caller.  This *must* be a reference!
        JsonTreeNode& nodeRef = iter->second;
        return nodeRef;
    }

    // Intentional: "Loud" method name to match return type.
    static JsonTreeNode
    getFreeNodeValue(TextViewDecorator& self,
                     const qreal lineSpacing,
                     const std::shared_ptr<TextViewJsonNode>& jsonNode)
    {
        if (self.m_freeTreeNodeVec.empty())
        {
            JsonTreeNode x =
                    JsonTreeNode{
                        .jsonNode = jsonNode,
                        .expander = newTreeNodeExpander(self, lineSpacing),
                        .sizeLabel = newQLabel(self, jsonNode)
                    };
            return x;
        }
        else {
            JsonTreeNode node = self.m_freeTreeNodeVec.back();
            self.m_freeTreeNodeVec.pop_back();
            node.jsonNode = jsonNode;
            setQLabelText(self, node.sizeLabel, jsonNode);
            node.expander->show();
            node.sizeLabel->show();
            return node;
        }
    }

    static TreeNodeExpander*
    newTreeNodeExpander(const TextViewDecorator& self,
                        const qreal lineSpacing)
    {
        QWidget* const parent = getParent(self);
        TreeNodeExpander* const w = new TreeNodeExpander{parent};
        w->setFixedSize(QSizeF{lineSpacing, lineSpacing}.toSize());
        w->show();
        return w;
    }

    static QLabel*
    newQLabel(const TextViewDecorator& self,
              const std::shared_ptr<TextViewJsonNode>& jsonNode)
    {
        QWidget* const parent = getParent(self);
        QLabel* const w = new QLabel{parent};
        setQLabelText(self, w, jsonNode);
        w->setTextFormat(Qt::TextFormat::PlainText);
        // Disable bizarre default behaviour where QLabel drag (mouse press+move) will move the whole window.
        // Ref: https://stackoverflow.com/a/18981898/257299
        w->setAttribute(Qt::WidgetAttribute::WA_TransparentForMouseEvents);
        w->setFont(self.m_textView.font());
        QPalette palette = w->palette();
        palette.setColor(w->foregroundRole(), self.m_textColor);
        w->setPalette(palette);
        w->show();
        return w;
    }

    static void
    setQLabelText(const TextViewDecorator& self,
                  QLabel* const label,
                  const std::shared_ptr<TextViewJsonNode>& jsonNode)
    {
        assert(jsonNode->childVec().empty() == false);
        // -1?  Always exclude final "}" or "]".
        const int childCount = jsonNode->childVec().size() - 1;

        if (isExpanded(self, jsonNode))
        {
            const QString& text = QString{"  // size:%1"}.arg(childCount);
            label->setText(text);
        }
        else {
            // Either "}" or "]"
            const std::shared_ptr<TextViewJsonNode>& jsonNodeClose = jsonNode->childVec().back();
            const QString& text = QString{"...%1  // size:%2"}.arg(jsonNodeClose->text()).arg(childCount);
            label->setText(text);
        }
    }

    static QWidget*
    getParent(const TextViewDecorator& self)
    {
        // Important: Parent is viewport, not self.  Why?  We are using absolute positioning for widgets that float
        // above the viewport, not self.  There is a subtle difference in coordinate systems.
        QWidget* const x = self.m_textView.viewport();
        return x;
    }

    static void
    slotSetExpanded(TextViewDecorator& self, const JsonTreeNode& node, const bool isExpanded)
    {
        setExpanded(self, node, isExpanded);
        if (isExpanded) {
            expand(self, node);
        }
        else {
            collapse(self, node);
        }
        updateViewport(self, node);
    }

    static void
    setExpanded(TextViewDecorator& self, const JsonTreeNode& node, const bool isExpanded)
    {
        const bool prevIsExpanded = Private::isExpanded(self, node.jsonNode);
        assert(prevIsExpanded != isExpanded);
        assert(canExpand(node.jsonNode));
        self.m_jsonNode_To_IsExpanded_Map[node.jsonNode] = isExpanded;
    }

    static void
    expand(const TextViewDecorator& self, const JsonTreeNode& node)
    {
        const std::shared_ptr<TextViewJsonNode>& firstChild = node.jsonNode->childVec().front();
        const TextViewPosition& firstPos = firstChild->pos();
        const std::shared_ptr<TextViewJsonNode>& lastChild = node.jsonNode->childVec().back();
        const TextViewPosition& lastPos = lastChild->pos();
        assert(firstPos.lineIndex <= lastPos.lineIndex);

        for (int lineIndex = firstPos.lineIndex;
             lineIndex <= lastPos.lineIndex;
             ++lineIndex)
        {
            const std::vector<std::shared_ptr<TextViewJsonNode>>& lineNodeVec = self.m_jsonTree->lineIndex_To_NodeVec()[lineIndex];
            assert(lineNodeVec.empty() == false);
            // It is most likely to be last, so perform reverse search.
            std::vector<std::shared_ptr<TextViewJsonNode>>::const_reverse_iterator found =
                std::find_if(lineNodeVec.rbegin(), lineNodeVec.rend(), &Private::canExpand);

            if (lineNodeVec.rend() == found)
            {
                // JsonNode on this line is not expandable.  Just show it.  Is this a useless branch???
                self.m_textView.docView().showLine(lineIndex);
            }
            else {
                const std::shared_ptr<TextViewJsonNode>& jsonNode = *found;
                const std::shared_ptr<TextViewJsonNode>& lastChild2 = jsonNode->childVec().back();
                const TextViewPosition& lastPos2 = lastChild2->pos();

                if (Private::isExpanded(self, jsonNode))
                {
                    // Show root and all children
                    self.m_textView.docView().showLineRange(lineIndex, lastPos2.lineIndex);
                }
                else {
                    // Only show root
                    self.m_textView.docView().showLine(lineIndex);
                }
                lineIndex = lastPos2.lineIndex;
            }
        }
    }

    static void
    collapse(const TextViewDecorator& self, const JsonTreeNode& node)
    {
        const std::shared_ptr<TextViewJsonNode>& firstChild = node.jsonNode->childVec().front();
        const TextViewPosition& firstPos = firstChild->pos();
        const std::shared_ptr<TextViewJsonNode>& lastChild = node.jsonNode->childVec().back();
        const TextViewPosition& lastPos = lastChild->pos();
        self.m_textView.docView().hideLineRange(firstPos.lineIndex, lastPos.lineIndex);
    }

    static void
    updateViewport(const TextViewDecorator& self, const JsonTreeNode& node)
    {
        // How much to update?  If we expand or collapse, the viewport will only change *below* line of node.jsonNode.
        // However, it is non-trivial to calculate the first visible line *after* node.jsonNode.  Solution?  Repaint *one* extra line.
        const TextViewPosition& pos = node.jsonNode->pos();
        const qreal y = self.m_textView.heightForVisibleLineIndex(pos.lineIndex);
        QRect viewportRect = self.m_textView.viewport()->rect();
        viewportRect.setTop(y);

        // Intentional: Expand or collapse will always show or hide text lines.  Force update on viewport of parent TextView...
        // which will trigger signalVisibleLinesChanged()... which will trigger Private::update()!  *Phew* :)
        self.m_textView.viewport()->update(viewportRect);
    }

    static StopEventPropagation
    keyPressEvent(const TextViewDecorator& self, QKeyEvent* const event)
    {
        // Assume event will match.  If not, flip to No at very bottom.
        StopEventPropagation stopEventPropagation = StopEventPropagation::Yes;

        // (1) This is probably from the numpad.
        if (QKeyEvents::matches(event, (Qt::Modifier::CTRL | Qt::Key::Key_Plus))
            // (2) Most keyboards require Shift+= to input '+'.
            || QKeyEvents::matches(event, (Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_Plus)))
        {
            keyPressEvent_trySetExpanded(self, true);
        }
        else if (QKeyEvents::matches(event, (Qt::Modifier::CTRL | Qt::Key::Key_Minus)))
        {
            keyPressEvent_trySetExpanded(self, false);
        }
        else {
            stopEventPropagation = StopEventPropagation::No;
        }
        return stopEventPropagation;
    }

    static void
    keyPressEvent_trySetExpanded(const TextViewDecorator& self, const bool isExpanded)
    {
        const TextViewGraphemePosition& pos = self.m_textView.textCursor().position();
        const auto iter = self.m_lineIndex_To_JsonTreeNode_Map.find(pos.pos.lineIndex);
        if (self.m_lineIndex_To_JsonTreeNode_Map.end() != iter)
        {
            const JsonTreeNode& node = iter->second;
            node.expander->slotSetExpanded(isExpanded);
        }
    }
};

// public
TextViewDecorator::
TextViewDecorator(TextView& parent)
    : Base{&parent}, m_textView{parent}, m_textColor{kTextColor}
{
    QObject::connect(parent.horizontalScrollBar(), &QScrollBar::valueChanged,
                     [this](const int value) {
                         Private::slotHorizontalScrollBarValueChanged(*this, value);
                     });
    QObject::connect(&parent, &TextView::signalVisibleLinesChanged,
                     [this]() { Private::update(*this); });

    m_textView.installEventFilter(this);
}

// public
void
TextViewDecorator::
setJsonTree(const std::shared_ptr<TextViewJsonTree>& jsonTree)
{
    assert(nullptr != jsonTree);
    m_jsonTree = jsonTree;
    Private::hideAllUsedTreeNodes(*this);
    // Intentional: Do not call Private::update(...).  Wait for TextView::signalVisibleLinesChanged().
}

// public
void
TextViewDecorator::
setTextColor(const QColor& color)
{
    if (color != m_textColor)
    {
        m_textColor = color;
        Private::update(*this);
    }
}

/**
 * @return true to stop event propagation
 */
// public
bool
TextViewDecorator::
eventFilter(QObject* const watched, QEvent* const event)  // override
{
    // Intentional: Base impl always returns false.
//    return QObject::eventFilter(watched, event);

    const QEvent::Type type = event->type();
    switch (type)
    {
        case QEvent::Type::KeyPress: {
            const StopEventPropagation x = Private::keyPressEvent(*this, static_cast<QKeyEvent*>(event));
            return (StopEventPropagation::Yes == x);
        }
        default: {
            // @DebugBreakpoint
            int dummy = 1;
        }
    }
    // Intentional: Never block processing by intended target.
    return false;
}

}  // namespace SDV
