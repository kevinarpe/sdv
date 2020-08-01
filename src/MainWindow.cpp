//
// Created by kca on 3/4/2020.
//

#include "MainWindow.h"
#include <set>
#include <rapidjson/error/en.h>
#include <QMessageBox>
#include <QAction>
#include <QLabel>
#include <QFileDialog>
#include <QApplication>
#include <QRegularExpression>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QScreen>
#include <QStyle>
#include <QMimeData>
#include <QCloseEvent>
#include <QDebug>
#include "Algorithm.h"
#include "MainWindowManager.h"
#include "MainWindowStatusBar.h"
#include "TextView.h"
#include "TextViewLineNumberArea.h"
#include "TextViewDocument.h"
#include "TextViewTextCursor.h"
#include "PaintForegroundFunctor.h"
#include "PaintForegroundContextImp.h"
#include "TextViewDecorator.h"
#include "QFileInfos.h"
#include "MainWindowInputStream.h"
#include "MainWindowThreadWorker.h"
#include "TextViewTextStatsService.h"
#include "TextViewJsonTree.h"
#include "TextViewJsonNode.h"
#include "TextViewJsonNodePositionService.h"

namespace SDV {

struct PaintForegroundFunctorImp : public PaintForegroundFunctor
{
    explicit PaintForegroundFunctorImp(const QColor& color)
        : m_pen{QPen{color}}
    {}

    ~PaintForegroundFunctorImp() override = default;

    void beforeDrawText(QPainter& painter, PaintContext* nullableContext) const override
    {
        painter.setPen(m_pen);
    }

private:
    const QPen m_pen;
};

static const QColor kColorDarkBlue = QColor{0, 0, 128};
static const QColor kColorBlue = QColor{0, 0, 255};
static const QColor kColorGreen = QColor{0, 128, 0};
static const QColor kColorPurple = QColor{102, 14, 122};

static const std::shared_ptr<PaintForegroundFunctorImp> kJsonNullOrBoolPainterForegroundFunctor =
    std::make_shared<PaintForegroundFunctorImp>(kColorDarkBlue);

static const std::shared_ptr<PaintForegroundFunctorImp> kJsonNumberPainterForegroundFunctor =
    std::make_shared<PaintForegroundFunctorImp>(kColorBlue);

static const std::shared_ptr<PaintForegroundFunctorImp> kJsonStringPainterForegroundFunctor =
    std::make_shared<PaintForegroundFunctorImp>(kColorGreen);

static const std::shared_ptr<PaintForegroundFunctorImp> kJsonKeyPainterForegroundFunctor =
    std::make_shared<PaintForegroundFunctorImp>(kColorPurple);

static std::unordered_map<JsonNodeType, std::shared_ptr<PaintForegroundFunctor>>
staticCreateFgMap()
{
    std::unordered_map<JsonNodeType, std::shared_ptr<PaintForegroundFunctor>> map{};
    Algorithm::Map::insertNewOrAssert(map, JsonNodeType::Null, kJsonNullOrBoolPainterForegroundFunctor);
    Algorithm::Map::insertNewOrAssert(map, JsonNodeType::Bool, kJsonNullOrBoolPainterForegroundFunctor);
    Algorithm::Map::insertNewOrAssert(map, JsonNodeType::Number, kJsonNumberPainterForegroundFunctor);
    Algorithm::Map::insertNewOrAssert(map, JsonNodeType::String, kJsonStringPainterForegroundFunctor);
    Algorithm::Map::insertNewOrAssert(map, JsonNodeType::Key, kJsonKeyPainterForegroundFunctor);
    return map;
}

static const std::unordered_map<JsonNodeType, std::shared_ptr<PaintForegroundFunctor>> kJsonNodeType_To_PaintFgFunctor = staticCreateFgMap();

static const QLocale kLocale{};

struct MainWindow::Private
{
    static void
    openFilePathList(MainWindow& self, const QStringList& absFilePathList)
    {
        assert(false == absFilePathList.isEmpty());
        // If there are multiple windows to be activated, only activate the *last*.
        std::optional<MainWindow*> optionalWindowToBeActivated{};
        const int count = absFilePathList.size();
        for (int i = 0; i < count; ++i)
        {
            const QString& absFilePath = absFilePathList[i];
            const std::optional<MainWindow*> optionalWindow = tryFindWindow(self, absFilePath);
            if (optionalWindow) {
                optionalWindowToBeActivated = optionalWindow;
                continue;
            }
            // It would be weird to have last file path opened, but then window activate for previous file.
            optionalWindowToBeActivated = std::nullopt;

            MainWindowInputStream inputStream{self.m_input};
            openInputStream(self, inputStream);
        }
        if (optionalWindowToBeActivated) {
            (*optionalWindowToBeActivated)->activateWindow();
        }
    }

    static std::optional<MainWindow*>
    tryFindWindow(MainWindow& self, const QString& absFilePath)
    {
        // Is the file open in this window?
        if (isMainWindowMatch(self, absFilePath)) {
            return &self;
        }
        // Is the file open in another window?
        for (MainWindow* mw : self.m_mainWindowManagerToken->getMainWindowManager())
        {
            if (isMainWindowMatch(*mw, absFilePath)) {
                return mw;
            }
        }
        return std::nullopt;
    }

    static bool
    isMainWindowMatch(const MainWindow& mw, const QString& absFilePath)
    {
        const bool x = (mw.m_input.isFile() && mw.m_input.description() == absFilePath);
        return x;
    }

    static bool
    shallClose(MainWindow& self)
    {
//        LAST: HOW DOES THIS WORK IF WE HAVE A BUNCH OF NON-FILES OPEN?  (CLIPBOARD & STDIN?)
        if (1 == self.m_mainWindowManagerToken->getMainWindowManager().size() && self.m_input.isNone()) {
            const bool x = shallExit(self);
            return x;
        }
        const bool shallClose =
            (QMessageBox::StandardButton::Yes ==
                 QMessageBox::question(&self, kWindowTitle, "Are you sure you want to close?",
                                       QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                       QMessageBox::StandardButton::Yes));
        if (shallClose)
        {
            self.m_mainWindowManagerToken->getMainWindowManager().afterCloseInput(self.m_input);

            if (1 == self.m_mainWindowManagerToken->getMainWindowManager().size())
            {
                self.m_input = MainWindowInput::kNone;
                self.setWindowTitle(kWindowTitle);
                self.m_textView->setVisible(false);
                self.m_fileCloseAction->setEnabled(false);
                self.m_statusBarTextViewLabelBaseText = "";
                self.m_statusBar->textStatsLabel()->setText(self.m_statusBarTextViewLabelBaseText);
                return false;
            }
        }
        return shallClose;
    }

    static bool
    shallExit(MainWindow& self)
    {
        const bool x =
            (QMessageBox::StandardButton::Yes ==
                 QMessageBox::question(&self, kWindowTitle, "Are you sure you want to exit?",
                                       QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                       QMessageBox::StandardButton::Yes));
        return x;
    }

    // TODO: Is this triggered when selection is cleared?  It seems not...
    static void
    slotSelectedTextChanged(MainWindow& self)
    {
        const TextViewSelection& selection = self.m_textView->textCursor().selection();
        if (false == selection.isValid())
        {
            self.m_statusBar->textStatsLabel()->setText(self.m_statusBarTextViewLabelBaseText);
            return;
        }
        const int requestId = self.m_threadWorker->nextRequestId();
        self.m_textStats.requestIds.slotCalcTextSelectionStatsVec.push_back(requestId);
        assert(QMetaObject::invokeMethod(self.m_threadWorker,
            [&self, selection, requestId]() {
                self.m_threadWorker->slotCalcTextSelectionStats(requestId, self.m_textStats.serviceId, selection);
            }));
    }

    static void
    slotCalcTextSelectionStatsComplete(MainWindow& self, const MainWindowThreadWorker::CalcTextSelectionStatsResult& result)
    {
        if (false == Algorithm::Vector::tryErase(self.m_textStats.requestIds.slotCalcTextSelectionStatsVec, result.requestId)) {
            return;
        }
        const QString& x =
            self.m_statusBarTextViewLabelBaseText
            + QString{" | <b><u>Selection:</u></b> %1 %2, %3 Unicode %4, %5 UTF-8 %6"}
                .arg(kLocale.toString(result.textStats.lineCount))
                .arg(1 == result.textStats.lineCount ? "line" : "lines")
                .arg(kLocale.toString(result.textStats.graphemeCount))
                .arg(1 == result.textStats.graphemeCount ? "char" : "chars")
                .arg(kLocale.toString(result.textStats.utf8ByteCount))
                .arg(1 == result.textStats.utf8ByteCount ? "byte" : "bytes");

        self.m_statusBar->textStatsLabel()->setText(x);
    }

    static void
    slotOpen(MainWindow& self)
    {
        // Appears to use as "JSON (*.json)" and wildcard is "*.json"
        const QString defaultFilter = "JSON (*.json)(*.json)";
        QString selectedFilter = defaultFilter;
        const QStringList filterList = QStringList() << defaultFilter << "All Files(*)";
        const QString filterStr = filterList.join(";;");
        QStringList absFilePathList = QFileDialog::getOpenFileNames(&self, "Select one or more files to open",
                                                                    QString(), filterStr, &selectedFilter);
        if (absFilePathList.isEmpty() == false) {
            openFilePathList(self, absFilePathList);
        }
    }

    static void
    slotFileOpenRecentMenuTriggered(MainWindow& self, QAction* action)
    {
        const MainWindowInput& input = Algorithm::Map::getOrAssert(self.m_windowMenuAction_To_Input_Map, action);
        assert(input.isFile());
        const QString& absFilePath = input.description();
        openFilePathList(self, QStringList{{absFilePath}});
    }

    static void
    slotWindowMenuTriggered(MainWindow& self, QAction* action)
    {
        const QVariant& data = action->data();
        if (QVariant::Type::String != data.type()) {
            return;
        }
        // Ex: "/home/kca/saveme/qt5/structured-data-viewer/cmake-build-debug/data/twitter.json"
        const QString& absFilePath = data.toString();

        const std::optional<MainWindow*>& optionalWindow = tryFindWindow(self, absFilePath);
        // Intentional: Do not check if optional value exists.  We know it always exists. :)
        MainWindow* const mw = *optionalWindow;
        if (mw == &self) {
            action->setChecked(true);
        }
        else {
            mw->activateWindow();
        }
    }

    static void
    slotClose(MainWindow& self)
    {
        if (shallClose(self)) {
            self.m_isClosing = true;
            self.close();
        }
    }

    static void
    slotExit(MainWindow& self)
    {
        if (shallExit(self)) {
            self.m_isClosing = true;
            self.close();
        }
    }

    static void
    openInput(MainWindow& self, const MainWindowInput& input)
    {
        const MainWindowInput::Type type = input.type();
        switch (type)
        {
            case MainWindowInput::Type::None:
            {
                break;
            }
            case MainWindowInput::Type::Clipboard:
            {
                slotOpenClipboardText(self);
                break;
            }
            case MainWindowInput::Type::Stdin:
            case MainWindowInput::Type::File:
            {
                MainWindowInputStream inputStream{input};
                openInputStream(self, inputStream);
                break;
            }
            default: assert(false);
        }
    }

    static void
    slotOpenClipboardText(MainWindow& self)
    {
        QClipboard* const clipboard = QGuiApplication::clipboard();
        const QString& text = clipboard->text();
        if (text.isEmpty())
        {
            QMessageBox::warning(&self, kWindowTitle, "Clipboard contains no text",
                                 QMessageBox::StandardButtons{QMessageBox::StandardButton::Ok},
                                 QMessageBox::StandardButton::Ok);
            return;
        }
        const int requestId = self.m_threadWorker->nextRequestId();
        self.m_textStats.requestIds.slotOpenClipboardTextVec.push_back(requestId);
        assert(QMetaObject::invokeMethod(self.m_threadWorker,
            [&self, requestId, &text]() {
                // The return signal is connected to 'slotOpenClipboardTextComplete(...)'.
                self.m_threadWorker->slotOpenClipboardText(requestId, self.m_textStats.serviceId, text);
            }));
    }

    static void
    slotOpenClipboardTextComplete(MainWindow& self, const MainWindowThreadWorker::OpenClipboardTextResult& result)
    {
        if (false == isSignalForThisInstance(self.m_textStats.requestIds.slotOpenClipboardTextVec, result.requestId)) {
            return;
        }
        if (result.jsonParseResult.IsError())
        {
            // Ref: https://github.com/Tencent/rapidjson/blob/master/example/pretty/pretty.cpp
            const char* parserError = rapidjson::GetParseError_En(result.jsonParseResult.Code());

            const QString& msg =
                QString("Failed to parse clipboard text at offset %1: %2").arg(result.jsonParseResult.Offset()).arg(parserError);

            QMessageBox::critical(&self, kWindowTitle, msg,
                                  QMessageBox::StandardButtons{QMessageBox::StandardButton::Ok},
                                  QMessageBox::StandardButton::Ok);
            return;
        }
        const MainWindowInput& input = MainWindowInput::createClipboard();
        openComplete(self, result, input);
    }

    static bool
    isSignalForThisInstance(std::vector<int>& requestIdVec, const int requestId)
    {
        const bool b = Algorithm::Vector::tryErase(requestIdVec, requestId);
        return b;
    }

    static void
    openComplete(MainWindow& self, const MainWindowThreadWorker::OpenResult& result, const MainWindowInput& input)
    {
        if (self.m_input.isNone())
        {
            openComplete0(self, result, input);
        }
        else {
            MainWindow* const mw =
                new MainWindow(
                    self.m_mainWindowManagerToken->getMainWindowManager(), self.m_formatMap, self.m_threadWorker,
                    // Intentional: Call ctor with None, then update *after* ctor.  Prevent infinite callback loop!
                    MainWindowInput::kNone,
                    self.parentWidget(), self.windowFlags());

            // TODO: Does the order of these three methods matter to UX?
            mw->adjustGeometry(&self);
            mw->show();
            // Important: Pass '*mw' here, not self!
            openComplete0(*mw, result, input);
        }
    }

    static void
    openComplete0(MainWindow& self, const MainWindowThreadWorker::OpenResult& result, const MainWindowInput& input)
    {
        self.m_input = input;
        // Enable only if necessary.
//        self.m_jsonTree = result.jsonTree;
        self.m_jsonNodePositionService = std::make_unique<TextViewJsonNodePositionService>(*result.jsonTree);
        self.m_textViewDecorator->setJsonTree(result.jsonTree);
        applyFormats(self, result.jsonTree);
        self.m_textView->setDoc(result.doc);
        self.m_textView->setVisible(true);
        self.m_fileCloseAction->setEnabled(true);
        self.setWindowTitle(self.m_input.description() + " - " + kWindowTitle);
        setStatusBarText(self, result.textStats);
        self.m_mainWindowManagerToken->getMainWindowManager().afterOpenInput(self.m_input);
    }

    static void
    applyFormats(MainWindow& self, const std::shared_ptr<TextViewJsonTree>& jsonTree)
    {
        std::unordered_map<int, TextView::ForegroundFormatSet>& map = self.m_textView->lineIndex_To_ForegroundFormatSet_Map();
        map.clear();

        const TextViewJsonNode& jsonNode = jsonTree->rootJsonNode();
        applyFormats0(self, jsonTree, jsonNode);
        // Intentional: Do not call self.m_textView->update() here, as self.m_textView->setDoc() will call it.
    }

    // Infix recursive algorithm: Visit jsonNode, then visit jsonNode->childVec().
    static void
    applyFormats0(MainWindow& self,
                  const std::shared_ptr<TextViewJsonTree>& jsonTree,
                  const TextViewJsonNode& jsonNode)
    {
        applyFormats1(self, jsonTree, jsonNode);

        const std::vector<std::shared_ptr<TextViewJsonNode>>& childVec = jsonNode.childVec();
        for (const std::shared_ptr<TextViewJsonNode>& child : childVec)
        {
            applyFormats0(self, jsonTree, *child);
        }
    }

    static void
    applyFormats1(MainWindow& self,
                  const std::shared_ptr<TextViewJsonTree>& jsonTree,
                  const TextViewJsonNode& jsonNode)
    {
        auto iter = kJsonNodeType_To_PaintFgFunctor.find(jsonNode.type());
        if (kJsonNodeType_To_PaintFgFunctor.end() == iter) {
            // Captain Obvious says: Not all JSON nodes have formatting, e.g., object begin/end: { }.
            return;
        }
        const TextViewPosition& pos = jsonNode.pos();
        std::unordered_map<int, TextView::ForegroundFormatSet>& map = self.m_textView->lineIndex_To_ForegroundFormatSet_Map();
        // This *might* insert a new map entry.
        TextView::ForegroundFormatSet& set = map[pos.lineIndex];

        const std::shared_ptr<PaintForegroundFunctor>& f =
            Algorithm::Map::getOrAssert(kJsonNodeType_To_PaintFgFunctor, jsonNode.type());

        LineSegment seg{.charIndex = pos.charIndex, .length = jsonNode.text().length()};
        LineFormatForeground lff{seg, f};
        Algorithm::Set::insertNewOrAssert(set, lff);
    }

    static void
    setStatusBarText(MainWindow& self, const TextViewTextStatsService::Result& result)
    {
        self.m_statusBarTextViewLabelBaseText =
            QString("%1 %2, %3 Unicode %4, %5 UTF-8 %6")
                .arg(kLocale.toString(result.lineCount))
                .arg(1 == result.lineCount ? "line" : "lines")
                .arg(kLocale.toString(result.graphemeCount))
                .arg(1 == result.graphemeCount ? "char" : "chars")
                .arg(kLocale.toString(result.utf8ByteCount))
                .arg(1 == result.utf8ByteCount ? "byte" : "bytes");

        self.m_statusBar->textStatsLabel()->setText(self.m_statusBarTextViewLabelBaseText);
    }

    static void
    openInputStream(MainWindow& self, const MainWindowInputStream& inputStream)
    {
        if (false == inputStream.isValid())
        {
            const QString& text =
                QString("Failed to open file: %1\n\nError: %2").arg(inputStream.input().description()).arg(inputStream.errorString());

            // TODO: Create dialog with nice view :)
            QMessageBox::critical(&self, kWindowTitle, text,
                                  QMessageBox::StandardButtons{QMessageBox::StandardButton::Ok},
                                  QMessageBox::StandardButton::Ok);
            return;
        }
        const int requestId = self.m_threadWorker->nextRequestId();
        self.m_textStats.requestIds.slotOpenInputStreamVec.push_back(requestId);
        assert(QMetaObject::invokeMethod(self.m_threadWorker,
            // Intentional: Copy 'inputStream'.  Reference is dangerous!
            [&self, inputStream, requestId]() {
                // The return signal is connected to 'slotOpenInputStreamComplete(...)'.
                self.m_threadWorker->slotOpenInputStream(requestId, self.m_textStats.serviceId, inputStream);
            }));
    }

    static void
    slotOpenInputStreamComplete(MainWindow& self, const MainWindowThreadWorker::OpenInputStreamResult& result)
    {
        if (false == isSignalForThisInstance(self.m_textStats.requestIds.slotOpenInputStreamVec, result.requestId)) {
            return;
        }
        // TODO: Can we merge this code w/ clipboard?
        if (result.jsonParseResult.IsError())
        {
            // Ref: https://github.com/Tencent/rapidjson/blob/master/example/pretty/pretty.cpp
            const char* parserError = rapidjson::GetParseError_En(result.jsonParseResult.Code());

            const QString& text =
                QString("File: %1\nFailed to parse at offset %2: %3")
                    .arg(result.inputStream.input().description()).arg(result.jsonParseResult.Offset()).arg(parserError);

            QMessageBox::critical(&self, kWindowTitle, text,
                                  QMessageBox::StandardButtons{QMessageBox::StandardButton::Ok},
                                  QMessageBox::StandardButton::Ok);
            return;
        }
        const MainWindowInput& input = result.inputStream.input();
        openComplete(self, result, input);
        self.m_mainWindowManagerToken->getMainWindowManager().tryAddRecentFileOpen(result.inputStream.input());
    }

    static void
    slotAbout(MainWindow& self)
    {
        QMessageBox::about(&self, kWindowTitle,
            QString{"<font size='+1'><b>Structured Data Viewer</b></font>"}
            + "<p>"
            + "A viewer for structured data -- JSON, XML, HTML, YAML, etc."
            + "<p>"
            + "Copyright by Kevin Connor ARPE (<a href='mailto:kevinarpe@gmail.com'>kevinarpe@gmail.com</a>)"
            + "<br>License: <a href='https://www.gnu.org/licenses/gpl-3.0.en.html'>https://www.gnu.org/licenses/gpl-3.0.en.html</a>"
            + "<br>Source Code: <a href='https://github.com/kevinarpe/sdv'>https://github.com/kevinarpe/sdv</a>"
            );
    }

    static void
    slotTextCursorPositionChanged(MainWindow& self)
    {
        // TODO: Move off-thread for speed?  Not sure...
        const TextViewGraphemePosition& pos = self.m_textView->textCursor().position();
        self.m_statusBar->slotSetTextCursorPosition(pos.pos.lineIndex, pos.pos.charIndex);
        // Also: Update status bar to show text cursor position.
        // @Nullable
        const std::shared_ptr<TextViewJsonNode>& jsonNode = self.m_jsonNodePositionService->tryFind(pos.pos);
        if (nullptr == jsonNode)
        {
            // clear
        }
        else {
            // how do we know the index of an array element?
            // TODO: Walk to root.  Each step can be a separate hyperlink so user can click to jump.
        }
    }
};

// public static
const QString MainWindow::kWindowTitle = "Structured Data Viewer";

// public
MainWindow::
MainWindow(MainWindowManager& mainWindowManager,
           const std::unordered_map<JsonNodeType, TextFormat>& formatMap,
           MainWindowThreadWorker* threadWorker,
           const MainWindowInput& input,
           QWidget* parent /*= nullptr*/,
           Qt::WindowFlags flags /*= Qt::WindowFlags()*/)
    : Base{parent, flags},
      m_formatMap{formatMap},
      // Intentional: Always start as 'None'.  Only update 'm_input' if 'input' is opened successfully.
      m_input{MainWindowInput::kNone},
      m_statusBar{new MainWindowStatusBar{this}},
      m_mainWindowManagerToken{mainWindowManager.add(*this)},
      m_isClosing{false},
      m_threadWorker{threadWorker},
      m_jsonNodePositionService{std::make_unique<TextViewJsonNodePositionService>()}
{
    setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    setWindowTitle(kWindowTitle);
    {
        QWidget* centralWidget = new QWidget{this};
        m_textView = new TextView{centralWidget};
        m_textView->setPaintForegroundContext(std::make_shared<PaintForegroundContextImp>());
        m_textViewLineNumberArea = new TextViewLineNumberArea{*m_textView, centralWidget};
        m_textViewDecorator = new TextViewDecorator{*m_textView};

        QHBoxLayout* hboxLayout = new QHBoxLayout{};
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setSpacing(0);
        hboxLayout->addWidget(m_textViewLineNumberArea);
        hboxLayout->addWidget(m_textView);
        centralWidget->setLayout(hboxLayout);
        setCentralWidget(centralWidget);
    }
    m_textStats.service = std::make_shared<TextViewTextStatsService>(m_textView->docViewPtr());
    m_textStats.serviceId = m_threadWorker->insertNewTextStatsServiceOrAssert(m_textStats.service);

    setAcceptDrops(true);
    setStatusBar(m_statusBar);
    m_statusBar->textStatsLabel()->setTextFormat(Qt::TextFormat::RichText);

    m_textView->setFont(QFont{"Deja Vu Sans Mono", 12});
    // Intentional: Use default from IntelliJ.
    m_textView->textCursor().slotSetBlinkMillis(500);
    m_textView->setVisible(false);

    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(
        "&Open File...", [this]() { Private::slotOpen(*this); }, QKeySequence::StandardKey::Open);

    fileMenu->addAction("Open from Clipboard", [this]() { Private::slotOpenClipboardText(*this); });

    m_fileOpenRecentMenu = fileMenu->addMenu("Open &Recent");
    QObject::connect(m_fileOpenRecentMenu, &QMenu::triggered,
                     [this](QAction* action) { Private::slotFileOpenRecentMenuTriggered(*this, action); });
    {
        int num = 0;
        for (const MainWindowInput& input2 : mainWindowManager.fileOpenRecentVec())
        {
            ++num;
            addRecentFileOpen(num, input2);
        }
    }
    m_fileCloseAction =
        fileMenu->addAction("&Close", [this]() { Private::slotClose(*this); }, QKeySequence::StandardKey::Close);
    m_fileCloseAction->setEnabled(false);

    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", [this]() { Private::slotExit(*this); }, QKeySequence::StandardKey::Quit);

    QMenu* const editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(m_textView->createCopyAction(editMenu));
    editMenu->addAction(m_textView->createSelectAllAction(editMenu));
    editMenu->addAction(m_textView->createDeselectAction(editMenu));
    // TODO: Add again later...
//    editMenu->addSeparator();
//    editMenu->addAction("&Find...", m_textWidget, &TextWidget::slotFind, QKeySequence::StandardKey::Find);
//    editMenu->addAction("&Go To...", m_textWidget, &TextWidget::slotGoTo, QKeySequence{Qt::CTRL + Qt::Key_G});

    m_windowMenu = menuBar()->addMenu("&Window");
    QObject::connect(m_windowMenu, &QMenu::triggered,
                     [this](QAction* action) { Private::slotWindowMenuTriggered(*this, action); });

    for (const MainWindowInput& input2 : mainWindowManager.openInputVec())
    {
        addOpenInput(input2);
    }
    // TODO: Window->Tile: Tiny icons to represent size & location: max, 1/2, 1/4, 1/9

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", [this]() { Private::slotAbout(*this); });
    helpMenu->addAction("About &Qt", qApp, &QApplication::aboutQt);
//
//    // Ref: https://code.qt.io/cgit/qt/qtbase.git/tree/examples/widgets/mainwindows/application/mainwindow.cpp?h=5.14
//    setUnifiedTitleAndToolBarOnMac(true);

    QObject::connect(m_threadWorker, &MainWindowThreadWorker::signalOpenInputStreamComplete,
                     // Important: Provide context QObject so the connection type will be queued (thread-safe).
                     this,
                     [this](const MainWindowThreadWorker::OpenInputStreamResult& result)
                     {
                         Private::slotOpenInputStreamComplete(*this, result);
                     });

    QObject::connect(m_threadWorker, &MainWindowThreadWorker::signalOpenClipboardTextComplete,
                     // Important: Provide context QObject so the connection type will be queued (thread-safe).
                     this,
                     [this](const MainWindowThreadWorker::OpenClipboardTextResult& result)
                     {
                         Private::slotOpenClipboardTextComplete(*this, result);
                     });

    QObject::connect(m_textView, &TextView::signalSelectedTextChanged,
                     // Optional: Provide context QObject to help with disconnect (during dtor).
                     this,
                     [this]() { Private::slotSelectedTextChanged(*this); });

    QObject::connect(m_threadWorker, &MainWindowThreadWorker::signalCalcTextSelectionStatsComplete,
                     // Important: Provide context QObject so the connection type will be queued (thread-safe).
                     this,
                     [this](const MainWindowThreadWorker::CalcTextSelectionStatsResult& result)
                     {
                         Private::slotCalcTextSelectionStatsComplete(*this, result);
                     });

    QObject::connect(&(m_textView->textCursor()), &TextViewTextCursor::signalPositionChanged,
                     // Optional: Provide context QObject to help with disconnect (during dtor).
                     this,
                     [this]() { Private::slotTextCursorPositionChanged(*this); });

    Private::openInput(*this, input);
}

// public
MainWindow::
~MainWindow()  // override
{
    m_threadWorker->eraseTextStatsServiceOrAssert(m_textStats.serviceId);
    for (const QMetaObject::Connection& c : m_qObjectConnectionVec)
    {
        QObject::disconnect(c);
    }
}

// public
void
MainWindow::
adjustGeometry(MainWindow* other /*= nullptr*/)
{
    const QList<QScreen*> screenList = QGuiApplication::screens();
    if (nullptr == other) {
        // TODO: FIXME: "2" is a personal pref of far left screen!
        QScreen* const screen = screenList[2];
        const QRect screen0Rect = screen->availableGeometry();
        const QRect rect =
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                QSize{1024, 768},
                screen0Rect);
        Base::setGeometry(rect);
    }
    else {
        QRect rect = other->geometry();
        rect.moveRight(rect.right() + 50.0);
        rect.moveTop(rect.top() + 50.0);
        Base::setGeometry(rect);
    }
}

// public
void
MainWindow::
addRecentFileOpen(const int number, const MainWindowInput& input)
{
    assert(number >= 1);
    assert(input.isFile());
    // Ex: "&3: /path/to/file"
    // Ex: "12: <clipboard>:7"
    const QString text =
        // Only shortcuts 1...9 make sense!
        ((number <= 9) ? "&" : "")
        + QString::number(number) + ": " + input.description();

    QAction* const action = m_fileOpenRecentMenu->addAction(text);
    Algorithm::Map::insertNewOrAssert(m_windowMenuAction_To_Input_Map, action, input);

    const QMetaObject::Connection& c =
        QObject::connect(action, &QObject::destroyed,
                         [this, action](QObject*) { Algorithm::Map::tryEraseByKey(m_windowMenuAction_To_Input_Map, action); });
    m_qObjectConnectionVec.emplace_back(c);
}

// public
void
MainWindow::
addOpenInput(const MainWindowInput& input)
{
    const QString& desc = input.description();
    QAction* const action = m_windowMenu->addAction(desc);
    if (input == m_input)
    {
        action->setCheckable(true);
        action->setChecked(true);
    }
    Algorithm::Map::insertNewOrAssert(m_windowMenuAction_To_Input_Map, action, input);

    const QMetaObject::Connection& c =
        QObject::connect(action, &QObject::destroyed,
                         [this, action](QObject*) { Algorithm::Map::tryEraseByKey(m_windowMenuAction_To_Input_Map, action); });
    m_qObjectConnectionVec.emplace_back(c);
}

// public
void
MainWindow::
removeOpenInput(const MainWindowInput& input)
{
    auto iter = Algorithm::Map::findByValueOrAssert(m_windowMenuAction_To_Input_Map, input);
    QAction* const action = iter->first;
    m_windowMenu->removeAction(action);
    // This will trigger signal QObject::destroyed, which will remove 'action' from 'm_windowMenuAction_To_Input_Map'.
    delete action;
}

/**
 * This is triggered when user clicks top-right X button from window manager,
 * or user closes window via system menu (Alt+F4).
 */
// protected override
void
MainWindow::
closeEvent(QCloseEvent* event)
{
    if (m_isClosing || Private::shallClose(*this))
    {
        QWidget::closeEvent(event);
    }
    else {
        event->ignore();
    }
}

// protected
void
MainWindow::
dragEnterEvent(QDragEnterEvent* event)  // override
{
    qDebug() << "dragEnterEvent(): " << event->mimeData()->formats() << "[" << event->mimeData()->text() << "]";

    // dragEnterEvent():  ["text/uri-list", "text/plain", "application/x-kde4-urilist"]
    // Ex: "file:///home/kca/saveme/qt5/structured-data-viewer/data/string.json"
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

// protected
void
MainWindow::
dropEvent(QDropEvent* event)  // override
{
    // Ref: https://wiki.qt.io/Drag_and_Drop_of_files
    assert(event->mimeData()->hasUrls());
    qDebug() << "dropEvent(): " << event->mimeData()->formats();

    QList<QUrl> urlList = event->mimeData()->urls();
    QStringList absFilePathList{};
    const int count = urlList.size();
    // Prevent "bombing" with thousands of files
    const int maxOpenCount = 32;
    for (int i = 0; i < count && absFilePathList.size() < maxOpenCount; ++i)
    {
        const QUrl& url = urlList[i];
        if (url.isLocalFile())
        {
            const QString& absDirOrFilePath = url.toLocalFile();
            QFileInfo fi{absDirOrFilePath};
            if (fi.exists() && fi.isFile())
            {
                absFilePathList.append(absDirOrFilePath);
            }
        }
    }
    if (false == absFilePathList.isEmpty())
    {
        Private::openFilePathList(*this, absFilePathList);
    }
    // TODO: Warn user if some skipped due to "bombing" or not local file or does not exist or is not a regular file?
}

}  // namespace SDV
