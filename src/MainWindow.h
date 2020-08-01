//
// Created by kca on 3/4/2020.
//

#ifndef SDV_MAINWINDOW_H
#define SDV_MAINWINDOW_H

#include <unordered_map>
#include <QMainWindow>
#include "MainWindowManagerToken.h"
#include "Constants.h"
#include "TextFormat.h"
#include "MainWindowInput.h"
class QThread;

namespace SDV {

class MainWindowManager;
class MainWindowStatusBar;
class TextWidget;
class TextView;
class TextViewLineNumberArea;
class TextViewDecorator;
class MainWindowThreadWorker;
class TextViewTextStatsService;
class TextViewJsonTree;
class TextViewJsonNodePositionService;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const QString kWindowTitle;

    using Base = QMainWindow;
    MainWindow(MainWindowManager& mainWindowManager,
               const std::unordered_map<JsonNodeType, TextFormat>& formatMap,
               MainWindowThreadWorker* threadWorker,
               const MainWindowInput& input,
               QWidget* parent = nullptr,
               Qt::WindowFlags flags = Qt::WindowFlags());
    ~MainWindow() override;

    void adjustGeometry(MainWindow* other = nullptr);
    void addRecentFileOpen(int number, const MainWindowInput& input);
    void addOpenInput(const MainWindowInput& input);
    void removeOpenInput(const MainWindowInput& input);

protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    struct Private;
    const std::unordered_map<JsonNodeType, TextFormat>& m_formatMap;
    MainWindowInput m_input;
    MainWindowStatusBar* m_statusBar;
    /** Ex: "7,245 lines, 13,845 Unicode chars, 14,252 UTF-8 bytes" */
    QString m_statusBarTextViewLabelBaseText;
    TextView* m_textView;
    TextViewLineNumberArea* m_textViewLineNumberArea;
    TextViewDecorator* m_textViewDecorator;
    std::unique_ptr<MainWindowManagerToken> m_mainWindowManagerToken;
    QAction* m_fileCloseAction;
    QMenu* m_fileOpenRecentMenu;
    QMenu* m_windowMenu;
    bool m_isClosing;
    MainWindowThreadWorker* m_threadWorker;
    // Enable only if necessary.
//    std::shared_ptr<TextViewJsonTree> m_jsonTree;
    std::unique_ptr<TextViewJsonNodePositionService> m_jsonNodePositionService;
    std::unordered_map<QAction*, MainWindowInput> m_windowMenuAction_To_Input_Map;
    std::vector<QMetaObject::Connection> m_qObjectConnectionVec;

    struct
    {
        std::shared_ptr<TextViewTextStatsService> service;
        int serviceId;
        struct
        {
            std::vector<int> slotOpenInputStreamVec;
            std::vector<int> slotOpenClipboardTextVec;
            std::vector<int> slotCalcTextSelectionStatsVec;

        } requestIds;

    } m_textStats;
};

}  // namespace SDV

#endif //SDV_MAINWINDOW_H
