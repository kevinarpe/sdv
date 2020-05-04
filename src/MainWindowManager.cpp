//
// Created by kca on 3/4/2020.
//

#include "MainWindowManager.h"
#include "MainWindowManagerToken.h"
#include "MainWindow.h"
#include <cassert>

namespace SDV {

// public
MainWindowManagerToken
MainWindowManager::
add(MainWindow& mainWindow)
{
    std::pair<std::unordered_set<MainWindow*>::iterator, bool> result = m_mainWindowSet.insert(&mainWindow);
    assert(result.second);
    return MainWindowManagerToken{*this, mainWindow};
}

// public
void
MainWindowManager::
tryAddFileOpenRecent(const QString& absFilePath)
{
    if (m_fileOpenRecentAbsFilePathVec.end() !=
            std::find(m_fileOpenRecentAbsFilePathVec.begin(), m_fileOpenRecentAbsFilePathVec.end(), absFilePath)) {
        return;
    }
    m_fileOpenRecentAbsFilePathVec.push_back(absFilePath);

    for (MainWindow* mw : m_mainWindowSet) {
        mw->addFileOpenRecent(m_fileOpenRecentAbsFilePathVec.size(), absFilePath);
    }
}

// public
void
MainWindowManager::
afterFileOpen(const QString& absFilePath)
{
    assert( ! absFilePath.isEmpty());
    assert(m_openAbsFilePathVec.end() == std::find(m_openAbsFilePathVec.begin(), m_openAbsFilePathVec.end(), absFilePath));
    m_openAbsFilePathVec.push_back(absFilePath);

    for (MainWindow* mw : m_mainWindowSet) {
        mw->addOpenFile(absFilePath);
    }
}

// public
void
MainWindowManager::
afterFileClose(const QString& absFilePath)
{
    assert( ! absFilePath.isEmpty());
    const std::vector<QString>::iterator& iter =
        std::find(m_openAbsFilePathVec.begin(), m_openAbsFilePathVec.end(), absFilePath);

    assert(m_openAbsFilePathVec.end() != iter);
    m_openAbsFilePathVec.erase(iter);

    for (MainWindow* mw : m_mainWindowSet) {
        mw->removeOpenFile(absFilePath);
    }
}

// private
void
MainWindowManager::
remove_(MainWindow& mainWindow)
{
    assert(m_mainWindowSet.erase(&mainWindow));
}

}  // namespace SDV