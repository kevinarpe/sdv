//
// Created by kca on 3/4/2020.
//

#include "MainWindowManager.h"
#include <cassert>
#include "MainWindowManagerToken.h"
#include "MainWindow.h"
#include "Algorithm.h"

namespace SDV {

// public
std::unique_ptr<MainWindowManagerToken>
MainWindowManager::
add(MainWindow& mainWindow)
{
    Algorithm::Set::insertNewOrAssert(m_mainWindowSet, &mainWindow);
    std::unique_ptr<MainWindowManagerToken> x = std::make_unique<MainWindowManagerToken>(*this, mainWindow);
    return x;
}

// public
bool
MainWindowManager::
tryAddRecentFileOpen(const MainWindowInput& input)
{
    if (false == input.isFile()) {
        return false;
    }
    // Scenario: Open file, close file, re-open same file: Do not re-add a recent file list
    if (Algorithm::Vector::exists(m_fileOpenRecentVec, input)) {
        return false;
    }
    // TODO: Is there a max:  Or can we remove index zero if too large... then append?
    // TODO: Can we save to a config file and reload?
    Algorithm::Vector::pushBackUniqueOrAssert(m_fileOpenRecentVec, input);

    for (MainWindow* const mw : m_mainWindowSet)
    {
        mw->addRecentFileOpen(m_fileOpenRecentVec.size(), input);
    }
    return true;
}

// public
void
MainWindowManager::
afterOpenInput(const MainWindowInput& input)
{
    Algorithm::Vector::pushBackUniqueOrAssert(m_openInputVec, input);

    for (MainWindow* const mw : m_mainWindowSet)
    {
        mw->addOpenInput(input);
    }
}

// public
void
MainWindowManager::
afterCloseInput(const MainWindowInput& input)
{
    Algorithm::Vector::eraseFirstOrAssert(m_openInputVec, input);

    for (MainWindow* const mw : m_mainWindowSet)
    {
        mw->removeOpenInput(input);
    }
}

// TODO: Can we combine with afterFileClose()?
// private
void
MainWindowManager::
remove(MainWindow& mainWindow)
{
    Algorithm::Set::eraseOrAsset(m_mainWindowSet, &mainWindow);
}

}  // namespace SDV
