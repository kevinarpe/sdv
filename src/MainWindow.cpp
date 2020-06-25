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
#include "PlainTextEdit.h"
#include "PrettyWriter2.h"
#include "QTextBoundaryFinders.h"
#include "TextView.h"
#include "TextViewLineNumberArea.h"
#include "TextViewDocument.h"
#include "TextViewTextCursor.h"
#include "PaintEventContextImp.h"
#include "PaintEventFunctor.h"

namespace SDV {

//namespace SDV {

// Number: const QColor colorBlue = QColor{0, 0, 255};
struct PaintEventFunctorImp : public PaintEventFunctor
{
    PaintEventFunctorImp(const QColor& color)
        : m_pen{QPen{color}}
    {}

    ~PaintEventFunctorImp() override = default;

    void operator()(QWidget& widget,
                    // @Nullable
                    PaintEventContext* context,
                    QPaintEvent& event,
                    QPainter& painter,
                    const QRectF& textBoundingRect) override
    {
        painter.setPen(m_pen);
    }

private:
    const QPen m_pen;
};

// Null or Bool: const QColor colorDarkBlue = QColor{0, 0, 128};
// String: const QColor colorGreen = QColor{0, 128, 0};
// Key: const QColor& colorPurple = QColor{102, 14, 122};
struct BoldFontPaintEventFunctorImp : public PaintEventFunctor
{
    BoldFontPaintEventFunctorImp(const QColor& color)
        : m_pen{QPen{color}}
    {}

    ~BoldFontPaintEventFunctorImp() override = default;

    void operator()(QWidget& widget,
                    // @Nullable
                    PaintEventContext* context,
                    QPaintEvent& event,
                    QPainter& painter,
                    const QRectF& textBoundingRect) override
    {
        PaintEventContextImp& c = dynamic_cast<PaintEventContextImp&>(*context);
        painter.setFont(c.boldFont());
        painter.setPen(m_pen);
    }

private:
    const QPen m_pen;
};

// Null or Bool: const QColor colorDarkBlue = QColor{0, 0, 128};
// String: const QColor colorGreen = QColor{0, 128, 0};
// Key: const QColor& colorPurple = QColor{102, 14, 122};
static const QColor kColorDarkBlue = QColor{0, 0, 128};
static const QColor kColorBlue = QColor{0, 0, 255};
static const QColor kColorGreen = QColor{0, 128, 0};
static const QColor kColorPurple = QColor{102, 14, 122};

static const std::shared_ptr<BoldFontPaintEventFunctorImp> kJsonNullOrBoolPaintEventFunctor =
    std::make_shared<BoldFontPaintEventFunctorImp>(kColorDarkBlue);

static const std::shared_ptr<PaintEventFunctorImp> kJsonNumberPaintEventFunctor =
    std::make_shared<PaintEventFunctorImp>(kColorBlue);

static const std::shared_ptr<BoldFontPaintEventFunctorImp> kJsonStringPaintEventFunctor =
    std::make_shared<BoldFontPaintEventFunctorImp>(kColorGreen);

static const std::shared_ptr<BoldFontPaintEventFunctorImp> kJsonKeyPaintEventFunctor =
    std::make_shared<BoldFontPaintEventFunctorImp>(kColorPurple);

//}  // namespace SDV

static const QLocale kLocale{};

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

            QMessageBox::critical(&self, kWindowTitle, text,
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

        static const Input kInvalid;
        static const Input kStdin;
    };

    static Input
    getInput(MainWindow& self) {
        if (QLatin1Char('-') == self.m_absFilePath) {
            return Input::kStdin;
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
                QMessageBox::critical(&self, kWindowTitle, text,
                                      QMessageBox::StandardButtons(QMessageBox::StandardButton::Ok),
                                      QMessageBox::StandardButton::Ok);
                return Input::kInvalid;
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
            QMessageBox::critical(&self, kWindowTitle, text,
                                  QMessageBox::StandardButtons(QMessageBox::StandardButton::Ok),
                                  QMessageBox::StandardButton::Ok);
            return Input::kInvalid;
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
        assert(doc.Accept(pw));
        const PrettyWriterResult& result = pw.result();
        self.m_textWidget->setResult(result);
//        self.m_tabWidget->setHidden(false);
//        self.m_textWidget->setVisible(true);
        const QStringList& lineList = result.m_jsonText.split(QRegularExpression{"\\r?\\n"});
        std::vector<QString> lineVec{lineList.begin(), lineList.end()};
        self.m_textView->setDoc(std::make_shared<TextViewDocument>(std::move(lineVec)));
        // Cursor overlap plus text selection breaks these text formats.
        if (false)
        {
            std::unordered_map<int, TextView::TextFormatSet>& map = self.m_textView->lineIndex_To_TextFormatSet_Map();
            {
                const int lineIndex = 1;
                TextView::TextFormatSet& set = map[lineIndex];
                assert(set.insert(TextViewLineTextFormat{4, 10, kJsonKeyPaintEventFunctor}).second);
            }
            {
                const int lineIndex = 4;
                TextView::TextFormatSet& set = map[lineIndex];
                assert(set.insert(TextViewLineTextFormat{16, 13, kJsonKeyPaintEventFunctor}).second);
                assert(set.insert(TextViewLineTextFormat{16 + 13 + 2, 10, kJsonStringPaintEventFunctor}).second);
            }
        }
        self.m_textView->setVisible(true);
        self.m_fileCloseAction->setEnabled(true);

        if (kClipboardAbsFilePath != self.m_absFilePath && kStdinAbsFilePath != self.m_absFilePath) {
            self.m_mainWindowManagerToken.getMainWindowManager().tryAddFileOpenRecent(self.m_absFilePath);
        }
        self.setWindowTitle(inputDescription + " - " + kWindowTitle);
        setStatusBarText(self, result);
    }

    static void
    setStatusBarText(MainWindow& self, const PrettyWriterResult& result)
    {
        // In practice, RapidJSON does not append a find newline.
        const int lineCount = 1 + result.m_jsonText.count(QLatin1Char{'\n'});

        const int graphemeCount =
            QTextBoundaryFinders::countBoundaries(QTextBoundaryFinder::BoundaryType::Grapheme, result.m_jsonText);

        const int byteCount = countBytes(result.m_jsonText);

        self.m_statusBarTextViewLabelBaseText =
            QString("%1 %2 | %3 Unicode %4 | %5 UTF-8 %6")
                .arg(kLocale.toString(lineCount))
                .arg(1 == lineCount ? "line" : "lines")
                .arg(kLocale.toString(graphemeCount))
                .arg(1 == graphemeCount ? "char" : "chars")
                .arg(kLocale.toString(byteCount))
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
                 QMessageBox::question(&self, kWindowTitle, "Are you sure you want to close?",
                                       QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                       QMessageBox::StandardButton::Yes));
        if (shallClose)
        {
            self.m_mainWindowManagerToken.getMainWindowManager().afterFileClose(self.m_absFilePath);

            if (1 == self.m_mainWindowManagerToken.getMainWindowManager().size()) {
                self.m_absFilePath.clear();
//                self.m_tabWidget->setHidden(true);
                self.m_textWidget->setVisible(false);
                self.m_fileCloseAction->setEnabled(false);
                self.m_statusBarTextViewLabelBaseText = "";
                self.m_statusBar->textViewLabel()->setText(self.m_statusBarTextViewLabelBaseText);
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
//
//    static void
//    slotTabChanged(MainWindow& self, const int tabIndex)
//    {
//        if (0 == tabIndex) {
//            self.m_statusBar->textViewLabel()->setVisible(true);
//        }
//        else if (1 == tabIndex) {
//            self.m_statusBar->textViewLabel()->setVisible(false);
//        }
//        else {
//            assert(false);
//        }
//    }

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
                               .arg(kLocale.toString(lineCount))
                               .arg(1 == lineCount ? "line" : "lines")
                               .arg(kLocale.toString(graphemeCount))
                               .arg(1 == graphemeCount ? "char" : "chars")
                               .arg(kLocale.toString(byteCount))
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
            QMessageBox::warning(&self, kWindowTitle, "Clipboard contains no text",
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
            const QString& msg =
                QString("Failed to parse clipboard text at offset %1: %2").arg(doc.GetErrorOffset()).arg(parserError);

            QMessageBox::critical(&self, kWindowTitle, msg,
                                  QMessageBox::StandardButtons(QMessageBox::StandardButton::Ok),
                                  QMessageBox::StandardButton::Ok);
            return;
        }
        if (self.m_absFilePath.isEmpty()) {
            self.m_absFilePath = kClipboardAbsFilePath;
            openDoc(self, doc, text.length(), kClipboardAbsFilePath);
        }
        else {
            MainWindow* mw =
                new MainWindow(
                    self.m_mainWindowManagerToken.getMainWindowManager(), self.m_formatMap, kClipboardAbsFilePath,
                    self.parentWidget(), self.windowFlags());
            mw->setGeometry(&self);
            mw->show();
        }
    }

    static void
    slotAbout(MainWindow& self)
    {
        QMessageBox::about(&self, kWindowTitle,
            QString{"<font size='+1'><b>Structured Data Viewer</b></font>"}
            + "<p>"
            + "A fast, convenient viewer for structured data -- JSON, XML, HTML, etc."
            + "<p>"
            + "Copyright by Kevin Connor ARPE (<a href='mailto:kevinarpe@gmail.com'>kevinarpe@gmail.com</a>)"
            + "<br>License: <a href='https://www.gnu.org/licenses/gpl-3.0.en.html'>https://www.gnu.org/licenses/gpl-3.0.en.html</a>"
            + "<br>Source Code: <a href='https://github.com/kevinarpe/sdv'>https://github.com/kevinarpe/sdv</a>"
            );
    }
};

// public static
const QString MainWindow::kWindowTitle = "Structured Data Viewer";

// private static
const QString MainWindow::kStdinAbsFilePath = "<stdin>";

// private static
const QString MainWindow::kClipboardAbsFilePath = "<clipboard>";

// public static
const MainWindow::Private::Input
MainWindow::Private::Input::
kInvalid{
    .inputStream{},
    .bufferCapacity = -1,
    .inputDescription{}
};

// public static
const MainWindow::Private::Input
MainWindow::Private::Input::
kStdin{
    .inputStream{shared_ptrs::do_not_delete(&std::cin)},
    // We have no idea how large is the input.  Use a reasonable default.
    .bufferCapacity = rapidjson::StringBuffer::kDefaultCapacity,
    .inputDescription{MainWindow::kStdinAbsFilePath}
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
      m_statusBar{new StatusBar{this}},
//      m_tabWidget{new TabWidget{this}},
      m_textWidget{new TextWidget{this}},
      m_mainWindowManagerToken{mainWindowManager.add(*this)},
      m_isClosing{false}
{
    setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    setWindowTitle(kWindowTitle);
//    setCentralWidget(m_textWidget);
    {
        QWidget* centralWidget = new QWidget{this};
        m_textView = new TextView{centralWidget};
        m_textView->setPaintEventContext(std::make_shared<PaintEventContextImp>());
        m_textViewLineNumberArea = new TextViewLineNumberArea{*m_textView, centralWidget};

        QHBoxLayout* hboxLayout = new QHBoxLayout{};
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setSpacing(0);
        hboxLayout->addWidget(m_textViewLineNumberArea);
        hboxLayout->addWidget(m_textView);
        centralWidget->setLayout(hboxLayout);
        setCentralWidget(centralWidget);
    }
    setAcceptDrops(true);
    setStatusBar(m_statusBar);
    m_statusBar->textViewLabel()->setTextFormat(Qt::TextFormat::RichText);
//
//    m_tabWidget->addTab(m_textWidget, "Text");
//    m_tabWidget->addTab(m_treeView, "Tree");
//    m_tabWidget->setHidden(true);
//    QObject::connect(m_tabWidget, &TabWidget::currentChanged,
//                     [this](int tabIndex) { Private::slotTabChanged(*this, tabIndex); });
//    Private::slotTabChanged(*this, m_tabWidget->currentIndex());

    m_textWidget->setVisible(false);
    QObject::connect(m_textWidget->plainTextEdit(), &PlainTextEdit::selectionChanged,
                     [this]() { Private::slotTextSelectionChanged(*this); });

    m_textView->setFont(QFont{"Deja Vu Sans Mono", 12});
    // Intentional: Use default from IntelliJ.
    m_textView->textCursor().slotSetBlinkMillis(500);
    m_textView->setVisible(false);

    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction(
        "&Open File...", [this]() { Private::slotOpen(*this); }, QKeySequence::StandardKey::Open);

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
        if (kClipboardAbsFilePath == m_absFilePath) {
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
