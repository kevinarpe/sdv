//
// Created by kca on 3/4/2020.
//

#ifndef SDV_MAINWINDOWMANAGERTOKEN_H
#define SDV_MAINWINDOWMANAGERTOKEN_H

namespace SDV {

class MainWindowManager;
class MainWindow;

class MainWindowManagerToken {

public:
    MainWindowManagerToken(MainWindowManager& mainWindowManager, MainWindow& mainWindow)
        : mainWindowManager(mainWindowManager), mainWindow(mainWindow)
    {}

    ~MainWindowManagerToken();

    MainWindowManager& getMainWindowManager() { return mainWindowManager; }

private:
    MainWindowManager& mainWindowManager;
    MainWindow& mainWindow;
};

}  // namespace SDV

#endif //SDV_MAINWINDOWMANAGERTOKEN_H
