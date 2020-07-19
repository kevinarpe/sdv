//
// Created by kca on 3/4/2020.
//

#ifndef SDV_MAINWINDOWMANAGER_H
#define SDV_MAINWINDOWMANAGER_H

#include <unordered_set>
#include <vector>
#include <memory>
#include <QString>
#include "MainWindowInput.h"

namespace SDV {

class MainWindowManagerToken;
class MainWindow;

class MainWindowManager {

public:
    MainWindowManager() = default;

    std::unique_ptr<MainWindowManagerToken>
    add(MainWindow& mainWindow);

    std::unordered_set<MainWindow*>::iterator
    begin() { return m_mainWindowSet.begin(); }

    std::unordered_set<MainWindow*>::iterator
    end() { return m_mainWindowSet.end(); }

    std::size_t size() const { return m_mainWindowSet.size(); }

    bool tryAddRecentFileOpen(const MainWindowInput& input);
    void afterOpenInput(const MainWindowInput& input);
    void afterCloseInput(const MainWindowInput& input);

    const std::vector<MainWindowInput>& fileOpenRecentVec() const { return m_fileOpenRecentVec; }
    const std::vector<MainWindowInput>& openInputVec() const { return m_openInputVec; }

private:
    struct Private;
    std::unordered_set<MainWindow*> m_mainWindowSet;
    std::vector<MainWindowInput> m_fileOpenRecentVec;
    std::vector<MainWindowInput> m_openInputVec;

    void remove(MainWindow& mainWindow);
    friend class MainWindowManagerToken;
};

}  // namespace SDV

#endif //SDV_MAINWINDOWMANAGER_H
