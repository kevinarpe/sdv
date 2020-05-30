//
// Created by kca on 3/4/2020.
//

#ifndef SDV_MAINWINDOWMANAGER_H
#define SDV_MAINWINDOWMANAGER_H

#include <unordered_set>
#include <vector>
#include <QString>

namespace SDV {

class MainWindowManagerToken;
class MainWindow;

class MainWindowManager {

public:
    explicit MainWindowManager() = default;

    MainWindowManagerToken
    add(MainWindow& mainWindow);

    std::unordered_set<MainWindow*>::iterator
    begin() { return m_mainWindowSet.begin(); }

    std::unordered_set<MainWindow*>::iterator
    end() { return m_mainWindowSet.end(); }

    std::size_t size() const { return m_mainWindowSet.size(); }

    void tryAddFileOpenRecent(const QString& absFilePath);
    void afterFileOpen(const QString& absFilePath);
    void afterFileClose(const QString& absFilePath);

    const std::vector<QString>& fileOpenRecentAbsFilePathVec() { return m_fileOpenRecentAbsFilePathVec; }
    const std::vector<QString>& openAbsFilePathVec() { return m_openAbsFilePathVec; }

private:
    struct Private;
    std::unordered_set<MainWindow*> m_mainWindowSet;
    std::vector<QString> m_fileOpenRecentAbsFilePathVec;
    std::vector<QString> m_openAbsFilePathVec;

    void remove_(MainWindow& mainWindow);
    friend class MainWindowManagerToken;
};

}  // namespace SDV

#endif //SDV_MAINWINDOWMANAGER_H
