//
// Created by kca on 3/4/2020.
//

#include "MainWindow.h"
#include <utility>
#include <fstream>
//#include <rapidjson/document.h>
#include "rapidjson/document.h"
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <QDebug>
#include <iostream>
#include "MainWindowManager.h"
#include "StatusBar.h"
#include "TabWidget.h"
#include "TextWidget.h"
#include "FindLineEdit.h"
#include "PlainTextEdit.h"
#include "PrettyWriter2.h"
#include "QTextBoundaryFinders.h"
#include "TreeModel.h"

namespace SDV {

static QLocale locale_{};

struct shared_ptrs
{
    template<typename T>
    static std::shared_ptr<T>
    do_not_delete(T* ptr)
    {
        // Ref: https://stackoverflow.com/a/20131949/257299
        return std::shared_ptr<T>{ptr, [](T*){}};
    }
};

struct MainWindow::Private
{
    static void
    openFilePathList(MainWindow& self, const QStringList& absFilePathList)
    {
        assert( ! absFilePathList.isEmpty());
        // If there are multiple windows to be activated, only activate the *last*.
        std::optional<MainWindow*> optionalWindowToBeActivated{};
        const int count = absFilePathList.size();
        for (int i = 0; i < count; ++i) {
            const QString& absFilePath = absFilePathList[i];
            const std::optional<MainWindow*>& optionalWindow = tryFindWindow(self, absFilePath);
            if (optionalWindow) {
                optionalWindowToBeActivated = optionalWindow;
                continue;
            }
            // It would be weird to have last file path opened, but then window activate for previous file.
            optionalWindowToBeActivated = std::nullopt;
            // Use this window if not file open.
            if (self.m_absFilePath.isEmpty()) {
                self.m_absFilePath = absFilePath;
                openFile(self);
            }
            else {
                MainWindow* mw =
                    new MainWindow(
                        self.m_mainWindowManagerToken.getMainWindowManager(), self.m_formatMap, absFilePath,
                        self.parentWidget(), self.windowFlags());
                mw->setGeometry(&self);
                mw->show();
            }
        }
        if (optionalWindowToBeActivated) {
            (*optionalWindowToBeActivated)->activateWindow();
        }
    }

    static std::optional<MainWindow*>
    tryFindWindow(MainWindow& self, const QString& absFilePath)
    {
        // Is the file open in this window?
        if (absFilePath == self.m_absFilePath) {
            return &self;
        }
        // Is the file open in another window?
        for (MainWindow* mw : self.m_mainWindowManagerToken.getMainWindowManager()) {
            if (mw->absFilePath() == absFilePath) {
                return mw;
            }
        }
        return std::nullopt;
    }

    static void
    openFile(MainWindow& self)
    {
        Input input{getInput(self)};
        if (input.bufferCapacity < 0) {
            return;
        }
        // Ref: https://stackoverflow.com/a/45257251/257299
        rapidjson::IStreamWrapper isw{*input.inputStream};
        rapidjson::Document doc;
        doc.ParseStream(isw);
        if (doc.HasParseError()) {
            // Ref: https://github.com/Tencent/rapidjson/blob/master/example/pretty/pretty.cpp
            const char* parserError = rapidjson::GetParseError_En(doc.GetParseError());
            const QString& text =
                QString("File: %1\nFailed to parse at offset %2: %3")
                    .arg(self.m_absFilePath).arg(doc.GetErrorOffset()).arg(parserError);

            QMessageBox::critical(&self, WINDOW_TITLE, text,
                                  QMessageBox::StandardButtons(QMessageBox::StandardButton::Ok),
                                  QMessageBox::StandardButton::Ok);
            return;
        }
        openDoc(self, doc, input.bufferCapacity, input.inputDescription);
        self.m_mainWindowManagerToken.getMainWindowManager().afterFileOpen(self.m_absFilePath);
    }

    struct Input {
        // Why a shared_ptr?  std::istream is weird about coping.
        // Ref: https://stackoverflow.com/questions/2159452/c-assign-cin-to-an-ifstream-variable
        std::shared_ptr<std::istream> inputStream;
        const qint64 bufferCapacity;
        const QString inputDescription;

        static const Input INVALID;
        static const Input STDIN;
    };

    static Input
    getInput(MainWindow& self) {
        if (QLatin1Char('-') == self.m_absFilePath) {
            return Input::STDIN;
        }
        QFile file(self.m_absFilePath);
        qint64 size = -1;
        // Scope to force RAII destructor call on QFile file.
        {
            // Ref: https://stackoverflow.com/questions/9465727/convert-qfile-to-file
//            fopen(qPrintable(m_absFilePath), )
            if ( ! file.open(QIODevice::ReadOnly)) {
                const QString& text =
                    QString("Failed to open file: %1\n\nError: %2").arg(self.m_absFilePath).arg(file.errorString());
                QMessageBox::critical(&self, WINDOW_TITLE, text,
                                      QMessageBox::StandardButtons(QMessageBox::StandardButton::Ok),
                                      QMessageBox::StandardButton::Ok);
                return Input::INVALID;
            }
            size = file.size();
            const int fd = file.handle();
            int dummy = 1;  // debug breakpoint
        }
        // Ref: qPrintable(): https://wiki.qt.io/Technical_FAQ#How_can_I_convert_a_QString_to_char.2A_and_vice_versa.3F
        // Ref: https://stackoverflow.com/a/37133527/257299
        std::shared_ptr<std::ifstream> ifs = std::make_shared<std::ifstream>(qPrintable(self.m_absFilePath));
        // Ref: https://stackoverflow.com/a/2954161/257299
//    std::ifstream ifs(m_absFilePath.toStdString());
        if ( ! ifs->is_open()) {
            // Ref: https://stackoverflow.com/questions/17337602/how-to-get-error-message-when-ifstream-open-fails
            // errno: do some detective work to discover WHY
            const QString& text = QString("Failed to open file: %1").arg(self.m_absFilePath);
            QMessageBox::critical(&self, WINDOW_TITLE, text,
                                  QMessageBox::StandardButtons(QMessageBox::StandardButton::Ok),
                                  QMessageBox::StandardButton::Ok);
            return Input::INVALID;
        }
        return Input{
            .inputStream{ifs},
            .bufferCapacity = size,
            .inputDescription{file.fileName()}
        };
    }

    /**
     * @param inputDescription
     *        usually filename, but might be STDIN_ or CLIPBOARD_
     */
    static void
    openDoc(MainWindow& self, const rapidjson::Document& doc, const int bufferCapacity, const QString& inputDescription)
    {
        rapidjson::StringBuffer sb{nullptr, static_cast<size_t>(bufferCapacity)};
        PrettyWriter2 pw{sb, static_cast<size_t>(bufferCapacity), self.m_formatMap};
//        RapidJsonHandler rjh{m_formatMap, static_cast<size_t>(bufferCapacity)};
//        rapidjson::StringBuffer buffer{nullptr, static_cast<size_t>(bufferCapacity)};
//        rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyWriter{buffer};
//        prettyWriter.SetIndent(' ', 6);
//        prettyWriter.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
//        doc.Accept(prettyWriter);
        assert(doc.Accept(pw));
        self.m_textWidget->slotSetPlainText(pw.jsonText(), pw.formatRangeVec());
        self.m_tabWidget->setHidden(false);
        self.m_fileCloseAction->setEnabled(true);
        TreeModel* const treeModel = new TreeModel{pw.rootVec(), self.m_treeView};
        self.m_treeView->setModel(treeModel);
        // Intentional: The TreeView is dead due to performance issues.
//        treeModel->setIndexWidgets(self.m_treeView);
        self.m_treeView->expandAll();

        if (CLIPBOARD_ != self.m_absFilePath && STDIN_ != self.m_absFilePath) {
            self.m_mainWindowManagerToken.getMainWindowManager().tryAddFileOpenRecent(self.m_absFilePath);
        }
        self.setWindowTitle(inputDescription + " - " + WINDOW_TITLE);
        const int lineCount = 1 + pw.jsonText().count(QLatin1Char{'\n'});
        const int graphemeCount =
            QTextBoundaryFinders::countBoundaries(QTextBoundaryFinder::BoundaryType::Grapheme, pw.jsonText());
        const int byteCount = countBytes(pw.jsonText());
        self.m_statusBarTextViewLabelBaseText =
            QString("%1 %2 | %3 Unicode %4 | %5 UTF-8 %6")
                .arg(locale_.toString(lineCount))
                .arg(1 == lineCount ? "line" : "lines")
                .arg(locale_.toString(graphemeCount))
                .arg(1 == graphemeCount ? "char" : "chars")
                .arg(locale_.toString(byteCount))
                .arg(1 == byteCount ? "byte" : "bytes");

        self.m_statusBar->textViewLabel()->setText(self.m_statusBarTextViewLabelBaseText);
    }

    static int
    countBytes(const QString& text)
    {
        int byteCount = 0;
        const QChar* data = text.constData();
        const int codePointCount = text.size();
        for (int i = 0; i < codePointCount; ++i, ++data) {
            ++byteCount;
            if (data->unicode() > 0xff) {
                ++byteCount;
            }
        }
        return byteCount;
    }

    static bool
    shallClose(MainWindow& self)
    {
        if (1 == self.m_mainWindowManagerToken.getMainWindowManager().size() && self.m_absFilePath.isEmpty()) {
            const bool x = shallExit(self);
            return x;
        }
        const bool shallClose =
            (QMessageBox::StandardButton::Yes ==
                 QMessageBox::question(&self, WINDOW_TITLE, "Are you sure you want to close?",
                                       QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                       QMessageBox::StandardButton::Yes));
        if (shallClose) {
            self.m_mainWindowManagerToken.getMainWindowManager().afterFileClose(self.m_absFilePath);

            if (1 == self.m_mainWindowManagerToken.getMainWindowManager().size()) {
                self.m_absFilePath.clear();
                self.m_tabWidget->setHidden(true);
                self.m_fileCloseAction->setEnabled(false);
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
                 QMessageBox::question(&self, WINDOW_TITLE, "Are you sure you want to exit?",
                                       QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                       QMessageBox::StandardButton::Yes));
        return x;
    }

    static void
    slotTabChanged(MainWindow& self, const int tabIndex)
    {
        if (0 == tabIndex) {
            self.m_statusBar->textViewLabel()->setVisible(true);
        }
        else if (1 == tabIndex) {
            self.m_statusBar->textViewLabel()->setVisible(false);
        }
        else {
            assert(false);
        }
    }

    static void
    slotTextSelectionChanged(MainWindow& self)
    {
        const QTextCursor& textCursor = self.m_textWidget->plainTextEdit()->textCursor();
        if (textCursor.selectionStart() == textCursor.selectionEnd()) {
            self.m_statusBar->textViewLabel()->setText(self.m_statusBarTextViewLabelBaseText);
            return;
        }
        const QString& selectedText = textCursor.selectedText();
        // Important: New line ('\n') is not used!
        // Ref: https://graphemica.com/2029
        const int lineCount =
            1 + selectedText.count(QChar::ParagraphSeparator) + (selectedText.endsWith(QChar::ParagraphSeparator) ? -1 : 0);
        const int graphemeCount =
            QTextBoundaryFinders::countBoundaries(QTextBoundaryFinder::BoundaryType::Grapheme, selectedText);
        const int byteCount = countBytes(selectedText);
        const QString& x = self.m_statusBarTextViewLabelBaseText
                           + QString{" || <b><u>Selection</u></b>: %1 %2 | %3 Unicode %4 | %5 UTF-8 %6"}
                               .arg(locale_.toString(lineCount))
                               .arg(1 == lineCount ? "line" : "lines")
                               .arg(locale_.toString(graphemeCount))
                               .arg(1 == graphemeCount ? "char" : "chars")
                               .arg(locale_.toString(byteCount))
                               .arg(1 == byteCount ? "byte" : "bytes");

        self.m_statusBar->textViewLabel()->setText(x);
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
        if ( ! absFilePathList.isEmpty()) {
            openFilePathList(self, absFilePathList);
        }
    }

    static void
    slotFileOpenRecentMenuTriggered(MainWindow& self, QAction* action)
    {
        // Ex: "/home/kca/saveme/qt5/structured-data-viewer/cmake-build-debug/data/twitter.json"
        const QString& absFilePath = action->data().toString();
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
            qApp->exit(EXIT_SUCCESS);
        }
    }

    static void
    slotOpenFromClipboard(MainWindow& self)
    {
        QClipboard* clipboard = QGuiApplication::clipboard();
        const QString& text = clipboard->text();
        if (text.isEmpty()) {
            QMessageBox::warning(&self, WINDOW_TITLE, "Clipboard contains no text",
                                 QMessageBox::StandardButtons(QMessageBox::StandardButton::Ok),
                                 QMessageBox::StandardButton::Ok);
            return;
        }
        rapidjson::Document doc;
        const char* str = qPrintable(text);
        doc.Parse(str);
        if (doc.HasParseError()) {
            // Ref: https://github.com/Tencent/rapidjson/blob/master/example/pretty/pretty.cpp
            const char* parserError = rapidjson::GetParseError_En(doc.GetParseError());
            const QString& text =
                QString("Failed to parse clipboard text at offset %1: %2").arg(doc.GetErrorOffset()).arg(parserError);

            QMessageBox::critical(&self, WINDOW_TITLE, text,
                                  QMessageBox::StandardButtons(QMessageBox::StandardButton::Ok),
                                  QMessageBox::StandardButton::Ok);
            return;
        }
        if (self.m_absFilePath.isEmpty()) {
            self.m_absFilePath = CLIPBOARD_;
            openDoc(self, doc, text.length(), CLIPBOARD_);
        }
        else {
            MainWindow* mw =
                new MainWindow(
                    self.m_mainWindowManagerToken.getMainWindowManager(), self.m_formatMap, CLIPBOARD_,
                    self.parentWidget(), self.windowFlags());
            mw->setGeometry(&self);
            mw->show();
        }
    }

    static void
    slotAbout(MainWindow& self)
    {
        QMessageBox::about(&self, WINDOW_TITLE, "Structured Data Viewer");
    }
};

// public static
const QString MainWindow::WINDOW_TITLE = "Structured Data Viewer";

// private static
const QString MainWindow::STDIN_ = "<stdin>";

// private static
const QString MainWindow::CLIPBOARD_ = "<clipboard>";

// public static
const MainWindow::Private::Input
MainWindow::Private::Input::
INVALID{
    .inputStream{},
    .bufferCapacity = -1,
    .inputDescription{}
};

// public static
const MainWindow::Private::Input
MainWindow::Private::Input::
STDIN{
    .inputStream{shared_ptrs::do_not_delete(&std::cin)},
    // We have no idea how large is the input.  Use a reasonable default.
    .bufferCapacity = rapidjson::StringBuffer::kDefaultCapacity,
    .inputDescription{MainWindow::STDIN_}
};

// public
MainWindow::
MainWindow(MainWindowManager& mainWindowManager,
           const std::unordered_map<JsonNodeType, TextFormat>& formatMap,
           QString absFilePath /*= QString()*/,
           QWidget* parent /*= nullptr*/,
           Qt::WindowFlags flags /*= Qt::WindowFlags()*/)
    : Base{parent, flags},
      m_formatMap{formatMap},
      m_absFilePath{std::move(absFilePath)},
      m_statusBar{new StatusBar{}},
      m_tabWidget{new TabWidget{}},
      m_textWidget{new TextWidget{}},
      m_treeView{new QTreeView{}},
      m_mainWindowManagerToken{mainWindowManager.add(*this)},
      m_isClosing{false}
{
    setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    setWindowTitle(WINDOW_TITLE);
    setCentralWidget(m_tabWidget);
    setAcceptDrops(true);
    setStatusBar(m_statusBar);
    m_statusBar->textViewLabel()->setTextFormat(Qt::TextFormat::RichText);

//    TreeModel* const treeModel = new TreeModel{, this};
//    m_treeView->setModel(treeModel);
    m_treeView->setHeaderHidden(true);
    m_treeView->setAnimated(false);
//    QAbstractItemDelegate* z = m_treeView->itemDelegate();
//    m_treeView->setIndexWidget(treeModel->index(0, 0, QModelIndex{}), new QLabel{"<html><font color='red'>this</font> is my <font color='green'><b>text</b></font></html>"});

    m_tabWidget->addTab(m_textWidget, "Text");
    m_tabWidget->addTab(m_treeView, "Tree");
    m_tabWidget->setHidden(true);
    QObject::connect(m_tabWidget, &TabWidget::currentChanged,
                     [this](int tabIndex) { Private::slotTabChanged(*this, tabIndex); });
    Private::slotTabChanged(*this, m_tabWidget->currentIndex());

    QObject::connect(m_textWidget->plainTextEdit(), &PlainTextEdit::selectionChanged,
                     [this]() { Private::slotTextSelectionChanged(*this); });

    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Open File...", [this]() { Private::slotOpen(*this); }, QKeySequence::StandardKey::Open);
    fileMenu->addAction("Open from Clipboard", [this]() { Private::slotOpenFromClipboard(*this); });

    m_fileOpenRecentMenu = fileMenu->addMenu("Open &Recent");
    QObject::connect(m_fileOpenRecentMenu, &QMenu::triggered,
                     [this](QAction* action) { Private::slotFileOpenRecentMenuTriggered(*this, action); });
    {
        int num = 0;
        for (const QString& absFilePath : mainWindowManager.fileOpenRecentAbsFilePathVec()) {
            ++num;
            addFileOpenRecent(num, absFilePath);
        }
    }
    m_fileCloseAction =
        fileMenu->addAction("&Close", [this]() { Private::slotClose(*this); }, QKeySequence::StandardKey::Close);
    m_fileCloseAction->setEnabled(false);

    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", [this]() { Private::slotExit(*this); }, QKeySequence::StandardKey::Quit);

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("&Find...", m_textWidget, &TextWidget::slotFind, QKeySequence::StandardKey::Find);
    editMenu->addAction("&Go To...", m_textWidget, &TextWidget::slotGoTo, QKeySequence{Qt::CTRL + Qt::Key_G});

    m_windowMenu = menuBar()->addMenu("&Window");
    QObject::connect(m_windowMenu, &QMenu::triggered,
                     [this](QAction* action) { Private::slotWindowMenuTriggered(*this, action); });

    for (const QString& absFilePath2 : mainWindowManager.openAbsFilePathVec()) {
        addOpenFile(absFilePath2);
    }
// Tile!
// Tiny icons to represent size & location: max, 1/2, 1/4, 1/9

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", [this]() { Private::slotAbout(*this); });
    helpMenu->addAction("About &Qt", qApp, &QApplication::aboutQt);
//
//    // Ref: https://code.qt.io/cgit/qt/qtbase.git/tree/examples/widgets/mainwindows/application/mainwindow.cpp?h=5.14
//    setUnifiedTitleAndToolBarOnMac(true);

    if ( ! m_absFilePath.isEmpty()) {
        if (CLIPBOARD_ == m_absFilePath) {
            m_absFilePath.clear();
            Private::slotOpenFromClipboard(*this);
        }
        else {
            Private::openFile(*this);
        }
    }
}

// public
void
MainWindow::
setGeometry(MainWindow* other /*= nullptr*/)
{
    const QList<QScreen*> screenList = QGuiApplication::screens();
    if (nullptr == other) {
        QScreen* const screen = screenList[0];
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
addFileOpenRecent(const int number, const QString& absFilePath)
{
    const QString text = ((number <= 9) ? "&" : "") + QString("%1: " + absFilePath).arg(number);
    QAction* action = m_fileOpenRecentMenu->addAction(text);
    action->setData(absFilePath);
}

// public
void
MainWindow::
addOpenFile(const QString& absFilePath)
{
    QAction* action = m_windowMenu->addAction(absFilePath);
    if (absFilePath == m_absFilePath) {
        action->setCheckable(true);
        action->setChecked(true);
    }
    action->setData(absFilePath);
}

// public
void
MainWindow::
removeOpenFile(const QString& absFilePath)
{
    for (QAction* action : m_windowMenu->actions()) {
        const QVariant& data = action->data();
        if (QVariant::Type::String != data.type()) {
            continue;
        }
        // Ex: "/home/kca/saveme/qt5/structured-data-viewer/cmake-build-debug/data/twitter.json"
        const QString& absFilePath2 = data.toString();
        if (absFilePath2 == absFilePath) {
            m_windowMenu->removeAction(action);
            return;
        }
    }
    assert(false);
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
    if (m_isClosing || Private::shallClose(*this)) {
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

    // dragEnterEvent():  ("text/uri-list", "text/plain", "application/x-kde4-urilist")
    // Ex: "file:///home/kca/saveme/qt5/structured-data-viewer/data/string.json"
    if (event->mimeData()->hasUrls()) {
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
    for (int i = 0; i < count && absFilePathList.size() < maxOpenCount; ++i) {
        const QUrl& url = urlList[i];
        if (url.isLocalFile()) {
            const QString& absDirOrFilePath = url.toLocalFile();
            QFileInfo fi{absDirOrFilePath};
            if (fi.exists() && fi.isFile()) {
                absFilePathList.append(absDirOrFilePath);
            }
        }
    }
    if ( ! absFilePathList.isEmpty()) {
        Private::openFilePathList(*this, absFilePathList);
    }
    // TODO: Warn user if some skipped due to "bombing" or not local file or does not exist or is not a regular file?
}

}  // namespace SDV
