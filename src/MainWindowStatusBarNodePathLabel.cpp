//
// Created by kca on 6/8/2020.
//

#include "MainWindowStatusBarNodePathLabel.h"
#include <QMenu>
#include <QGuiApplication>
#include <QContextMenuEvent>
#include "TextViewJsonNodePositionService.h"
#include "MainWindowStatusBar.h"
#include "Algorithm.h"
#include "TextView.h"
#include "TextViewTextCursor.h"
#include "TextViewPosition.h"
#include "TextViewJsonNode.h"
#include "TextViewGraphemePosition.h"

namespace SDV {

struct MainWindowStatusBarNodePathLabel::Private
{
    static void
    slotTextCursorPositionChanged(MainWindowStatusBarNodePathLabel& self)
    {
        // TODO: Move off-thread for speed?  Not sure...
        const TextViewGraphemePosition& pos = self.m_textView->textCursor().position();
        self.m_statusBar->slotSetTextCursorPosition(pos.pos.lineIndex, pos.pos.charIndex);
        // Also: Update status bar to show text cursor position.
        // @Nullable
        const std::shared_ptr<TextViewJsonNode>& jsonNode = self.m_jsonNodePositionService->tryFind(pos.pos);
        self.m_hyperlinkToNodeMap.clear();
        int index = 0;
        QString richText{};
        QString plainText{};
        for (TextViewJsonNode* n = jsonNode.get();
             nullptr != n;
             n = n->nullableParent(), ++index)
        {
            const QString& indexStr = QString::number(index);
            Algorithm::Map::insertNewOrAssert(self.m_hyperlinkToNodeMap, indexStr, n);
            const int arrayIndex = n->arrayIndex();
            if (arrayIndex >= 0)
            {
                richText.prepend(QString{QLatin1String{"[<a href='%1'>%2</a>]"}}.arg(indexStr).arg(arrayIndex));
                plainText.prepend(QString{QLatin1String{"[%1]"}}.arg(arrayIndex));
            }
            else {
                const JsonNodeType type = n->type();
                if (JsonNodeType::Key == type)
                {
                    richText.prepend(QString{QLatin1String{"[<a href='%1'>%2</a>]"}}.arg(indexStr).arg(n->text()));
                    plainText.prepend(QString{QLatin1String{"[%1]"}}.arg(n->text()));
                }
            }
        }
        richText.prepend(QLatin1Char{'$'});
        plainText.prepend(QLatin1Char{'$'});
        // TODO: What font to use?
        self.setRichAndPlainText(richText, plainText);
    }

    static void
    slotNodePathLabelLinkActivated(MainWindowStatusBarNodePathLabel& self, const QString& href)
    {
        TextViewJsonNode* const n = Algorithm::Map::getOrAssert(self.m_hyperlinkToNodeMap, href);
        TextViewTextCursor& textCursor = self.m_textView->textCursor();
        const TextViewPosition& pos = n->pos();
        textCursor.setPosition(pos);
    }

    static QAction*
    createCopyAction(const MainWindowStatusBarNodePathLabel& self, QMenu* const menu)
    {
        const QIcon& icon = QIcon::fromTheme(Constants::IconThemeName::kEditCopy);
        // @Debug
        const bool n = icon.isNull();
        QAction* const action = new QAction{icon, "&Copy", menu};
        QObject::connect(action, &QAction::triggered, [&self](){ self.slotCopyTextToClipboard(); });
        return action;
    }
};

// public explicit
MainWindowStatusBarNodePathLabel::
MainWindowStatusBarNodePathLabel(TextView* textView,
                                 MainWindowStatusBar* statusBar,
                                 QWidget *parent /*= nullptr*/,
                                 Qt::WindowFlags flags /*= Qt::WindowFlags()*/)
    : Base{parent, flags},
      m_textView{textView}, m_statusBar{statusBar},
      m_jsonNodePositionService{std::make_unique<TextViewJsonNodePositionService>()}
{
    assert(nullptr != m_textView);
    assert(nullptr != m_statusBar);

    QObject::connect(&(m_textView->textCursor()), &TextViewTextCursor::signalPositionChanged,
                     // Optional: Provide context QObject to help with disconnect (during dtor).
                     this,
                     [this]() { Private::slotTextCursorPositionChanged(*this); });

    QObject::connect(this, &QLabel::linkActivated,
                     // Optional: Provide context QObject to help with disconnect (during dtor).
                     this,
                     [this](const QString& href) { Private::slotNodePathLabelLinkActivated(*this, href); });
}

// public
MainWindowStatusBarNodePathLabel::
~MainWindowStatusBarNodePathLabel() = default;  // override

// public
void
MainWindowStatusBarNodePathLabel::
setJsonTree(const TextViewJsonTree& jsonTree)
{
    m_jsonNodePositionService = std::make_unique<TextViewJsonNodePositionService>(jsonTree);
    Private::slotTextCursorPositionChanged(*this);
}

// public slot
void
MainWindowStatusBarNodePathLabel::
setRichAndPlainText(const QString& richText, const QString& plainText)
{
    setText(richText);
    m_plainText = plainText;
}

// public slot
void
MainWindowStatusBarNodePathLabel::
slotCopyTextToClipboard(QClipboard::Mode mode /*= QClipboard::Mode::Clipboard*/)
const
{
    if (m_plainText.isEmpty()) {
        return;
    }
    QClipboard* const clipboard = QGuiApplication::clipboard();
    clipboard->setText(m_plainText, mode);
}

// protected
void
MainWindowStatusBarNodePathLabel::
contextMenuEvent(QContextMenuEvent* const event)  // override
{
    // Intentional: Do not use default context menu.
//    Base::contextMenuEvent(event);

    event->accept();
    QMenu* const menu = new QMenu{this};
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction* const copyAction = Private::createCopyAction(*this, menu);
    menu->addAction(copyAction);
    const QPoint& globalPos = event->globalPos();
    // Ref: https://stackoverflow.com/questions/63303201/when-to-call-qmenupopup-vs-qmenuexec
    // @Blocking
    menu->exec(globalPos);
    // Replaced by: menu->setAttribute(Qt::WA_DeleteOnClose);
//    delete menu;
}

}  // namespace SDV
