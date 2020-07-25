cmake_minimum_required(VERSION 3.15)
project(sdv)

# I tried these two env vars, but it did not work for me.
# export CXX=/usr/bin/g++-10
# export CC=/usr/bin/gcc-10
# Ref: https://stackoverflow.com/a/17275650/257299
#set(CMAKE_C_COMPILER /usr/bin/gcc-10 CACHE PATH "" FORCE)
#set(CMAKE_CXX_COMPILER /usr/bin/g++-10 CACHE PATH "" FORCE)

# Ref: https://bytefreaks.net/programming-2/make-building-with-cmake-verbose
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Ref: https://unix.stackexchange.com/a/218253/29414
IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb3")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

#include_directories(lib/qtbase-opensource-src/include)
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
        src/TreeItem.cpp
        src/TreeItem.h
        src/TextFormat.h
        src/Bit.h
        src/GoToLineEdit.cpp
        src/GoToLineEdit.h
        src/SmoothShowHideWidget.cpp
        src/SmoothShowHideWidget.h
        src/TreeNodeExpander.cpp
        src/TreeNodeExpander.h
        src/JsonNode.cpp
        src/JsonNode.h
        src/JsonTree.cpp
        src/JsonTree.h
        src/PlainTextEditDecorator.cpp
        src/PlainTextEditDecorator.h
        src/Algorithm.h
        src/TextViewDocument.cpp
        src/TextViewDocument.h
        src/TextView.cpp
        src/TextView.h
        src/TextViewDocumentView.cpp
        src/TextViewDocumentView.h
        src/TextViewTextCursor.cpp
        src/TextViewTextCursor.h
        src/TextViewGraphemePosition.cpp
        src/TextViewGraphemePosition.h
        src/GraphemeFinder.cpp
        src/GraphemeFinder.h
        src/TextBoundaryFinder.cpp
        src/TextBoundaryFinder.h
        src/TextViewGraphemeCursor.cpp
        src/TextViewGraphemeCursor.h
        src/TextSegmentFontWidth.cpp
        src/TextSegmentFontWidth.h
        src/TextViewSelection.cpp
        src/TextViewSelection.h
        src/TextViewPosition.cpp
        src/TextViewPosition.h
        src/TextViewLineNumberArea.cpp
        src/TextViewLineNumberArea.h
        src/PaintBackgroundFunctor.cpp
        src/PaintBackgroundFunctor.h
        src/PaintForegroundFunctor.cpp
        src/PaintForegroundFunctor.h
        src/LineSegment.cpp
        src/LineSegment.h
        src/PaintContext.cpp
        src/PaintContext.h
        src/PaintForegroundContextImp.cpp
        src/PaintForegroundContextImp.h
        src/LineFormat.cpp
        src/LineFormat.h
        src/TextViewDecorator.cpp
        src/TextViewDecorator.h
        src/CountingIterator.h
        src/TextViewLineSelection.cpp
        src/TextViewLineSelection.h
        src/QKeyEvents.cpp
        src/QKeyEvents.h
        src/QStrings.cpp
        src/QStrings.h
        src/TextViewTextStatsService.cpp
        src/TextViewTextStatsService.h
        src/QFileInfos.cpp
        src/QFileInfos.h
        src/MainWindowInputStream.cpp
        src/MainWindowInputStream.h
        src/SmartPointers.h
        src/MainWindowThreadWorker.cpp
        src/MainWindowThreadWorker.h
        src/SharedPointerMap.h
        src/ThreadSafeSharedPointerMap.h
        src/MainWindowInput.cpp
        src/MainWindowInput.h
        src/NextIdService.cpp
        src/NextIdService.h
        src/JsonTreeResult.h
        src/JsonTreeResult.cpp)

target_link_libraries(sdv Qt5::Widgets)
