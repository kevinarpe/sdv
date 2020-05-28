cmake_minimum_required(VERSION 3.15)
project(sdv)

# Ref: https://bytefreaks.net/programming-2/make-building-with-cmake-verbose
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Ref: https://unix.stackexchange.com/a/218253/29414
IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb3")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

include_directories(lib/qtbase-opensource-src-5.12.5+dfsg/include)
find_package(Qt5Widgets REQUIRED)
find_package(RapidJSON REQUIRED)

add_executable(sdv
        main.cpp
        src/MainWindow.cpp
        src/MainWindow.h
        src/MainWindowManagerToken.cpp
        src/MainWindowManagerToken.h
        src/MainWindowManager.cpp
        src/MainWindowManager.h
        src/TabWidget.cpp
        src/TabWidget.h
        src/TextWidget.cpp
        src/TextWidget.h
        src/PrettyWriter2.h
        src/PlainTextEdit.cpp
        src/PlainTextEdit.h
        src/FindThreadWorker.cpp
        src/FindThreadWorker.h
        src/FindLineEdit.cpp
        src/FindLineEdit.h
        src/FindWidget.cpp
        src/FindWidget.h
        src/BalloonTip.cpp
        src/BalloonTip.h
        src/LineNumberAreaWidget.cpp
        src/LineNumberAreaWidget.h
        src/IWidgetWithLineNumberArea.h
        src/StatusBar.cpp
        src/StatusBar.h
        src/QTextBoundaryFinders.cpp
        src/QTextBoundaryFinders.h
        src/GoToWidget.cpp
        src/GoToWidget.h
        src/CircleWidget.cpp
        src/CircleWidget.h
        src/QtHash.h
        src/Constants.cpp
        src/Constants.h
        src/rapidjson/document.h
        src/TreeModel.cpp
        src/TreeModel.h
        src/TreeItem.cpp
        src/TreeItem.h
        src/TextFormat.h
        src/Bit.h
        src/GoToLineEdit.cpp
        src/GoToLineEdit.h
        src/SmoothShowHideWidget.cpp
        src/SmoothShowHideWidget.h
        src/TreeNodeExpanderWidget.cpp
        src/TreeNodeExpanderWidget.h)

target_link_libraries(sdv Qt5::Widgets)
