//
// Created by kca on 3/4/2020.
//

#ifndef SDV_MAINWINDOW_H
#define SDV_MAINWINDOW_H

#include <QMainWindow>
#include <QTextCharFormat>
#include "MainWindowManagerToken.h"
#include "Constants.h"
#include "TextFormat.h"
class QTreeView;

namespace SDV {

class MainWindowManager;
class StatusBar;
//class TabWidget;
class TextWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const QString WINDOW_TITLE;
    using Base = QMainWindow;
    MainWindow(MainWindowManager& mainWindowManager,
               const std::unordered_map<JsonNodeType, TextFormat>& formatMap,
               QString absFilePath = QString(),
               QWidget* parent = nullptr,
               Qt::WindowFlags flags = Qt::WindowFlags());

    void setGeometry(MainWindow* other = nullptr);
    const QString& absFilePath() const { return m_absFilePath; }
    void addFileOpenRecent(int number, const QString& absFilePath);
    void addOpenFile(const QString& absFilePath);
    void removeOpenFile(const QString& absFilePath);

protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    struct Private;
    static const QString CLIPBOARD_;  // = "<clipboard>";
    static const QString STDIN_;  // = "<stdin>";
    const std::unordered_map<JsonNodeType, TextFormat>& m_formatMap;
    // Ex: "" or "/home/kca/saveme/qt5/structured-data-viewer/cmake-build-debug/CMakeCache.txt"
    QString m_absFilePath;
    StatusBar* m_statusBar;
    /** Ex: "7,245 lines, 13,845 Unicode chars, 14,252 UTF-8 bytes" */
    QString m_statusBarTextViewLabelBaseText;
//    TabWidget* m_tabWidget;
    TextWidget* m_textWidget;
    // RAII guarantees when '*this' is destroyed, mainWindowManagerToken will also be destroyed.
    MainWindowManagerToken m_mainWindowManagerToken;
    QAction* m_fileCloseAction;
    QMenu* m_fileOpenRecentMenu;
    QMenu* m_windowMenu;
    bool m_isClosing;
};

}  // namespace SDV

#endif //SDV_MAINWINDOW_H
