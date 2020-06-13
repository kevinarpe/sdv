//
// Created by kca on 31/5/2020.
//

#include "PlainTextEditDecorator.h"
#include <QLabel>
#include <QScrollBar>
#include <QDebug>
#include "PlainTextEdit.h"
#include "TreeNodeExpanderWidget.h"
#include "JsonNode.h"
#include "Algorithm.h"

namespace SDV {

struct PlainTextEditDecorator::Private
{
    static void
    slotScrollBarValueChanged(PlainTextEditDecorator& self, const Qt::Orientation ori, const int value)
    {
        update(self);
    }

    static void
    slotTextChanged(PlainTextEditDecorator& self)
    {
        self.m_textBlockMinHeight = getTextBlockMinHeight(self);
        if (self.m_textBlockMinHeight <= 0.0) {
            int dummy = 1;
            return;
        }
        update(self);
    }

    static qreal
    getTextBlockMinHeight(const PlainTextEditDecorator& self)
    {
        qreal minHeight = std::numeric_limits<qreal>::max();
        for (QTextBlock textBlock = self.m_parent.document()->firstBlock(); textBlock.isValid(); textBlock = textBlock.next())
        {
            assert(textBlock.isVisible());
            // TODO: When we hide blocks when expanders are collapsed... this code needs to be enabled!
//            if (textBlock.isVisible()) {
            const QRectF& r = self.m_parent.blockBoundingRect(textBlock);
            minHeight = qMin(minHeight, r.height());
//            }
        }
        if (std::numeric_limits<qreal>::max() == minHeight) {
            return -1.0;
        }
        else {
            return minHeight;
        }
    }

    static void
    slotParentShow(PlainTextEditDecorator& self)
    {
        if (self.m_updateBeforeShow)
        {
            self.m_updateBeforeShow = false;
            update(self);
        }
    }

    static void
    update(PlainTextEditDecorator& self)
    {
        // Bizarre: Wait for first show, before updating floating widgets (expander and size label).
        if ( ! self.m_parent.isVisible()) {
            self.m_updateBeforeShow = true;
            return;
        }
        hideAllUsedTreeNodes(self);
        const QFontMetricsF& fm = QFontMetricsF{self.m_parent.font()};
        const QTextBlock& firstVisibleBlock = self.m_parent.tryGetFirstVisibleBlock();
        const QString& firstText = firstVisibleBlock.text();
        const QTextBlock& lastVisibleBlock = self.m_parent.tryGetLastVisibleBlock();
        const QString& lastText = lastVisibleBlock.text();
        const PrettyWriterResult& result = self.m_parent.result();

        for (QTextBlock textBlock = firstVisibleBlock;
             textBlock.isValid() && textBlock.blockNumber() <= lastVisibleBlock.blockNumber();
             textBlock = textBlock.next())
        {
            if (false == textBlock.isVisible()) {
                continue;
            }
            const int lineIndex = textBlock.blockNumber();
            const QVector<JsonNode*>& nodeVec = result.m_lineIndex_To_NodeVec[lineIndex];
            for (JsonNode* const jsonNode : nodeVec)
            {
                const QRectF& z = self.m_parent.blockBoundingGeometry(textBlock).translated(self.m_parent.contentOffset());
                const QString& text = textBlock.text();

                if ( ! isExpandable(jsonNode)) { continue; }
                const int indexOfExpanderPosition = Private::indexOfExpanderPosition(text);
                // Important: Check this before malloc below.
                if (-1 == indexOfExpanderPosition) {
                    int dummy = 1;
                    continue;
                }
                TreeNode& treeNode = getFreeTreeNode(self, self.m_textBlockMinHeight, jsonNode);
                const QRectF& bbg = self.m_parent.blockBoundingGeometry(textBlock);
                const QPointF& co = self.m_parent.contentOffset();
                const QRectF& bbgt = bbg.translated(co);
                {
                    const qreal xOffset = fm.horizontalAdvance(text, indexOfExpanderPosition);
                    const qreal yOffset = (bbgt.height() - treeNode.m_expander->height()) / 2;
                    treeNode.m_expander->move(QPointF{bbgt.x() + xOffset, bbgt.y() + yOffset}.toPoint());
                }
                {
                    const qreal xOffset = fm.horizontalAdvance(text) + fm.horizontalAdvance(QLatin1String{"  "});
                    treeNode.m_sizeLabel->move(QPointF{bbgt.x() + xOffset, bbgt.y()}.toPoint());
                }
                break;
            }
        }
    }

    static void
    hideAllUsedTreeNodes(PlainTextEditDecorator& self)
    {
        if (self.m_usedTreeNodeVec.empty()) {
            return;
        }
        for (TreeNode& treeNode : self.m_usedTreeNodeVec)
        {
            treeNode.m_expander->setVisible(false);
            treeNode.m_sizeLabel->setVisible(false);
        }
        self.m_freeTreeNodeVec.insert(
            self.m_freeTreeNodeVec.end(), self.m_usedTreeNodeVec.begin(), self.m_usedTreeNodeVec.end());

        self.m_usedTreeNodeVec.clear();
    }

    static bool
    isExpandable(JsonNode* const jsonNode)
    {
        const bool x = ((JsonNodeType::ObjectBegin == jsonNode->type() || JsonNodeType::ArrayBegin == jsonNode->type())
                        // Remember: Every object and array has at least one child: '}' or ']'
                        && jsonNode->childVec().size() > 1);
        return x;
    }

    static bool
    isExpanded(const PlainTextEditDecorator& self, JsonNode* const jsonNode)
    {
        const bool x = Algorithm::Map::getOrDefault(self.m_jsonNodeToIsExpandedMap, jsonNode, true);
//        if (jsonNode->optionalParent()) {
//            qDebug() << "isExpanded:" << self.m_parent.result().m_nodeToPosMap[jsonNode].lineIndex << ":" << jsonNode->optionalParent().value()->text() << "." << jsonNode->text() << ":" << x;
//        }
        return x;
    }

    static int
    indexOfExpanderPosition(const QString& text)
    {
        const size_t i = Algorithm::findFirstIndexIf(text, [](const QChar& ch) { return !ch.isSpace(); });
        if (i >= 2) {
            return i - 2;
        }
        else {
            return -1;
        }
    }

    static TreeNode&
    getFreeTreeNode(PlainTextEditDecorator& self, const qreal textBlockMinHeight, JsonNode* jsonNode)
    {
        TreeNode& treeNode = getFreeTreeNode0(self, textBlockMinHeight, jsonNode);
        const bool isExpanded = Private::isExpanded(self, jsonNode);
        treeNode.m_expander->slotSetExpanded(isExpanded);
        if (treeNode.m_expanderConnection) {
            QObject::disconnect(treeNode.m_expanderConnection);
        }
        treeNode.m_expanderConnection =
            QObject::connect(treeNode.m_expander, &TreeNodeExpanderWidget::signalIsExpanded,
                             [&self, jsonNode](const bool isExpanded) { slotSetExpanded(self, jsonNode, isExpanded); });
        return treeNode;
    }

    static TreeNode&
    getFreeTreeNode0(PlainTextEditDecorator& self, const qreal textBlockMinHeight, JsonNode* jsonNode)
    {
        if (self.m_freeTreeNodeVec.empty()) {//<<newline
            TreeNode& x =
                self.m_usedTreeNodeVec.emplace_back(
                    TreeNode{
                        .m_jsonNode = jsonNode,
                        .m_expander = newTreeNodeExpanderWidget(self, textBlockMinHeight),
                        .m_sizeLabel = newQLabel(self, jsonNode)
                    });
            return x;
        }
        else {
            TreeNode& x = self.m_usedTreeNodeVec.emplace_back(self.m_freeTreeNodeVec.back());
            self.m_freeTreeNodeVec.pop_back();
            setQLabelText(*(x.m_sizeLabel), jsonNode);
            x.m_expander->show();
            x.m_sizeLabel->show();
            return x;
        }
    }

    static TreeNodeExpanderWidget*
    newTreeNodeExpanderWidget(PlainTextEditDecorator& self, const qreal textBlockMinHeight)
    {
        TreeNodeExpanderWidget* const w = new TreeNodeExpanderWidget{getParent(self)};
        w->setFixedSize(QSizeF{textBlockMinHeight, textBlockMinHeight}.toSize());
        w->show();
        return w;
    }

    static void
    slotSetExpanded(PlainTextEditDecorator& self, JsonNode* const jsonNode, const bool isExpanded)
    {
        const bool prevIsExpanded = Private::isExpanded(self, jsonNode);
        assert(prevIsExpanded != isExpanded);
        self.m_jsonNodeToIsExpandedMap[jsonNode] = isExpanded;
        const PrettyWriterResult& result = self.m_parent.result();
        if (isExpanded) {
            JsonNode* const firstChild = jsonNode->childVec().first();
            const PrettyWriterResult::Pos& firstPos = result.m_nodeToPosMap[firstChild];
            JsonNode* const lastChild = jsonNode->childVec().last();
            const PrettyWriterResult::Pos& lastPos = result.m_nodeToPosMap[lastChild];
            assert(firstPos.lineIndex <= lastPos.lineIndex);
            const QTextBlock& textBlock0 = self.m_parent.document()->findBlockByLineNumber(firstPos.lineIndex);
            QTextBlock textBlock = textBlock0;
            for (int lineIndex = firstPos.lineIndex;
                 lineIndex <= lastPos.lineIndex;
                 ++lineIndex, textBlock = textBlock.next())
            {
                assert(textBlock.isValid());
                const QVector<JsonNode*>& lineNodeVec = result.m_lineIndex_To_NodeVec[lineIndex];
                assert( ! lineNodeVec.isEmpty());
                QVector<JsonNode*>::const_iterator found = std::find_if(lineNodeVec.begin(), lineNodeVec.end(), &Private::isExpandable);
                if (lineNodeVec.end() == found) {
                    textBlock.setVisible(true);
                }
                else {
                    JsonNode* const expandableLineNode = *found;
                    if (Private::isExpanded(self, expandableLineNode)) {
                        textBlock.setVisible(true);
                    }
                    else {
                        JsonNode* const lastChild2 = expandableLineNode->childVec().last();
                        const PrettyWriterResult::Pos& lastPos2 = result.m_nodeToPosMap[lastChild2];
                        lineIndex = lastPos2.lineIndex - 1;
                        textBlock = self.m_parent.document()->findBlockByLineNumber(lineIndex);
                    }
                }
            }
        }
        else {
            JsonNode* const firstChild = jsonNode->childVec().first();
            const PrettyWriterResult::Pos& firstPos = result.m_nodeToPosMap[firstChild];
            JsonNode* const lastChild = jsonNode->childVec().last();
            const PrettyWriterResult::Pos& lastPos = result.m_nodeToPosMap[lastChild];
            assert(firstPos.lineIndex <= lastPos.lineIndex);
            const QTextBlock& textBlock0 = self.m_parent.document()->findBlockByLineNumber(firstPos.lineIndex);
            QTextBlock textBlock = textBlock0;
            for (int lineIndex = firstPos.lineIndex;
                 lineIndex <= lastPos.lineIndex;
                 ++lineIndex, textBlock = textBlock.next())
            {
                assert(textBlock.isValid());
                textBlock.setVisible(false);
            }
        }
//        self.m_parent.document()->markContentsDirty(0, self.m_parent.document()->end().position());
//        self.m_parent.update();
        self.m_parent.show();
    }

    static QLabel*
    newQLabel(PlainTextEditDecorator& self, JsonNode* const jsonNode)
    {
        QLabel* const w = new QLabel{getParent(self)};
        setQLabelText(*w, jsonNode);
        w->setTextFormat(Qt::TextFormat::PlainText);
        // Disable bizarre default behaviour where QLabel drag (mouse press+move) will move the whole window.
        // Ref: https://stackoverflow.com/a/18981898/257299
        w->setAttribute(Qt::WidgetAttribute::WA_TransparentForMouseEvents);
        w->setFont(self.m_parent.font());
        QPalette palette = w->palette();
        palette.setColor(w->foregroundRole(), Qt::GlobalColor::darkGray);
        w->setPalette(palette);
        w->show();
        return w;
    }

    static void
    setQLabelText(QLabel& w, JsonNode* jsonNode)
    {
        assert( ! jsonNode->childVec().empty());
        const QString& text = QString{"// size:%1"}.arg(jsonNode->childVec().size());
        w.setText(text);
    }

    static QWidget*
    getParent(PlainTextEditDecorator& self)
    {
        // Important: Parent is viewport, not self.  Why?  We are using absolute positioning for widgets that float
        // above the viewport, not self.  There is a subtle difference in coordinate systems.
        QWidget* const x = self.m_parent.viewport();
        return x;
    }
};

// public
PlainTextEditDecorator::
PlainTextEditDecorator(PlainTextEdit& parent)
    : Base{&parent}, m_parent{parent}, m_updateBeforeShow{false}, m_textBlockMinHeight{-1.0}
{
    parent.installEventFilter(this);
    QObject::connect(&parent, &QPlainTextEdit::textChanged, [this]() { Private::slotTextChanged(*this); });

    // Move with arrows or mouse drag of scrollbar will trigger signal valueChanged.
    // Signal sliderMoved will only trigger for mouse drag.
    // Weirdly, for me, only signal actionTriggered is triggered for any scroll bar action.
    // However, shortcuts that update the scrollbar, e.g., Home or End or Ctrl+Home or Ctrl+End
    // will trigger signal valueChanged.
    parent.verticalScrollBar()->setTracking(true);

    // horizontalScrollBar
    QObject::connect(parent.horizontalScrollBar(), &QScrollBar::valueChanged,
                     [this](const int value) { Private::slotScrollBarValueChanged(*this, Qt::Orientation::Horizontal, value); });
//
//    QObject::connect(parent.horizontalScrollBar(), &QScrollBar::sliderMoved,
//                     [this](const int value) { Private::slotScrollBarValueChanged(*this, Qt::Orientation::Horizontal, value); });

    QObject::connect(parent.horizontalScrollBar(), &QScrollBar::actionTriggered,
                     [this](const int value) { Private::slotScrollBarValueChanged(*this, Qt::Orientation::Horizontal, value); });

    // verticalScrollBar
    QObject::connect(parent.verticalScrollBar(), &QScrollBar::valueChanged,
                     [this](const int value) { qDebug() << "valueChanged";  Private::slotScrollBarValueChanged(*this, Qt::Orientation::Vertical, value); });

//    QObject::connect(parent.verticalScrollBar(), &QScrollBar::sliderMoved,
//                     [this](const int value) { Private::slotScrollBarValueChanged(*this, Qt::Orientation::Vertical, value); });

    QObject::connect(parent.verticalScrollBar(), &QScrollBar::actionTriggered,
                     [this](const int action) { Private::slotScrollBarValueChanged(*this, Qt::Orientation::Vertical, action); });

    QObject::connect(&parent, &PlainTextEdit::signalShow, [this]() { Private::slotParentShow(*this); });
}

// public
bool
PlainTextEditDecorator::
eventFilter(QObject* watched, QEvent* event)  // override
{
    static QEvent::Type lastEvent = static_cast<QEvent::Type>(-1);
//    static int count = 0;
//    ++count;
//    qDebug() << count << ":" << event->type();
    // This is X Windows System order of events.  Microsoft Windows and Mac OS X may be different.
    if (lastEvent == QEvent::Type::Resize && QEvent::Type::Paint == event->type()) {
        Private::update(*this);
    }
    lastEvent = event->type();
    const bool x = Base::eventFilter(watched, event);
    return x;
}

}  // namespace SDV
