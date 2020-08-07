//
// Created by kca on 6/8/2020.
//

#ifndef SDV_MAINWINDOWSTATUSBARNODEPATHLABEL_H
#define SDV_MAINWINDOWSTATUSBARNODEPATHLABEL_H

#include <QLabel>
#include <QClipboard>

namespace SDV {

class TextViewJsonTree;
class TextViewJsonNodePositionService;
class TextView;
class MainWindowStatusBar;
class TextViewJsonNode;

class MainWindowStatusBarNodePathLabel : public QLabel
{
    Q_OBJECT

public:
    using Base = QLabel;

    MainWindowStatusBarNodePathLabel(TextView* textView,
                                     MainWindowStatusBar* statusBar,
                                     QWidget *parent = nullptr,
                                     Qt::WindowFlags flags = Qt::WindowFlags());

    ~MainWindowStatusBarNodePathLabel() override;  // = default

    void setJsonTree(const TextViewJsonTree& jsonTree);

public slots:
    void setRichAndPlainText(const QString& richText, const QString& plainText);
    void slotCopyTextToClipboard(QClipboard::Mode mode = QClipboard::Mode::Clipboard) const;

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    struct Private;
    TextView* m_textView;
    MainWindowStatusBar* m_statusBar;
    QString m_plainText;
    std::unique_ptr<TextViewJsonNodePositionService> m_jsonNodePositionService;
    /**
     * Keys are always integers that count from zero, e.g., {@code "0"}.  The deepest nodes in a node path (from current node to root)
     * have the lowest values, and the node just before root always has hyperlink {@code "0"}.
     */
    std::unordered_map<QString, TextViewJsonNode*> m_hyperlinkToNodeMap;
};

}  // namespace SDV

#endif //SDV_MAINWINDOWSTATUSBARNODEPATHLABEL_H
