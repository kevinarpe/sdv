#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <unistd.h>
#include "src/MainWindow.h"
#include "src/TextWidget.h"
#include "src/MainWindowManager.h"
#include "src/QtHash.h"
#include "src/Constants.h"
#include "src/TextFormat.h"
#include "src/MainWindowThreadWorker.h"

/*
 * Views: compressed (text), pretty print (text), QTreeView
 * Provide text formatting options
 * Ex: Space after comma?  Indent size... etc.  Pretty compressed view, etc.
 * Can "transport" between any view
 * How?  Mouse over always shows JSON Path, or XPath, or ${whatever} as tooltip
 * -> Both in text and tree views!
 * -> Text view: Mosue over should ideally highlight the current JSON *token*
 * Right click -> context menu -> Just between views or copy "path"
 * Can we handle 1GB input?  That is important test.
 * Input data: JSON, XML, HTML, B(J)SON, MessagePack, etc.
 * Search / navigate by "path"
 * *if* we need it, handle UTF-8/16LE/BE/32BE/LE, including TERRIBLE byte-order-mark (FEFF)
 * Ctrl+G -> Got to line number
 * Add simple edit menu items: Copy, Select All, Word Wrap (toggle)
 * Ultimate goal: Create statically linked binaries for release on GitHub
 * -> How does SubSurface do their releases?
 * -> More about static linking: https://drewdevault.com/dynlib.html
 * Remember recently opened files (via $HOME/.SDV/config.json ?)
 * -> Also remember search history
 * Find history should not be limited to text...
 * -> Alt+Down can show: "search text" [case sensitive + regex]
 * FindWidget: Add support for JSON Path
 * Character inspector: Once enable mouse over a char to view Unicode details
 * QWindow::setIcon(const QIcon &icon)
 * Add to GitHub account
 * Need to check TreeView's handling of very long lines... do we need word-wrap?
 * Can we allow // and / ** / comments in the JSON?  See: rapidjson::ParseFlag::kParseCommentsFlag
 * Add checkbox to show object/array sizes and object key/array element indices.
 * -> Useful in *both* text and tree views... and even "selectable" in text view is still useful.
 * Can we create a "cram-down" view?  "a":{\n\t...\n\t"b":{\n\t\t...\n\t\t"c":{\n\t\t\t...\n\t\t\t"key": 123
 * -> Again, a checkbox could be used to quickly toggle between full and cram-down visual modes.
 * Add nice shortcut keys help panel.  (Right side?)  Printable as PDF?
 * Character inspection mode: Character under cursor has balloon pop-up to share Unicode details.
 * Status bar: Add current line number and grapheme number and selection info
 * Like IntelliJ, (Shift+)Ctrl+W will (de)select by scope -- wider and wider.
 * Ctrl+MouseWheel should adjust font size.
 * Right click any JSON node: Close all branches to parent.
 * Add text cursor location (line number and column number) to the status bar
 * How many classes can be (re-)declared as 'final'?  Be explicit!
 * Ctrl++ and Ctrl+- will open/close nodes
 *
 * x Input?  x Clipboard, x file, x drag-n-drop
 * x Multiple tabs->windows for multiple inputs
 * x Searching the text should be very fluid
 * x File->Open Recent
 * x Ctrl+LeftMouseButton -> Drag like the hand from Adobe Acrobat
 * x Add line numbers!
 * x Alt+PageUp goes left one screen, and vice versa with Alt+PageDown
 * x Ctrl+Home goes to top-left.  Ctrl+End goes to bottom-right.
 * x Support drag and drop of file
 * x Rename project form hello-world!
 * x Add namespace
 * x Status bar should show number of chars + lines
 * x Refactor everything to use struct Private
 * x All "file" input to be: regular file by command line option, stdin, GUI file picker, drag and drop file, clipboard
 * x Parsing of input... be "gentle" on failures
 * x Selected text should have range and count of lines & char count displayed in status bar
 * x After closing document, clear the status bar.
 * x Double check if all QObject/QWidget sub-classes have *at least*: ~$class() override = default;
 * x Add license (GPL3+)
 */

std::unordered_map<SDV::JsonNodeType, SDV::TextFormat>
create()
{
/*
QTextCharFormat
JSON String value
FgColor: R:0,G:128,B:0 -> #008000
Bold

keyword (true, false, null)
DarkBlue: 0, 0, 128
Bold

number: Blue: 0,0,255

key: Purple: 102,14,122, Bold

valid escape seq: DarkBlue, Bold
\r\n... \u0021
 */
    QFont fontBold{SDV::TextWidget::kDefaultFont};
    fontBold.setWeight(QFont::Weight::Bold);

    // TODO: also "valid escape seq", e.g., "\n"
    QTextCharFormat jsonNullOrBoolFormat;
    const QColor colorDarkBlue = QColor{0, 0, 128};
    jsonNullOrBoolFormat.setForeground(QBrush{colorDarkBlue});
    jsonNullOrBoolFormat.setFont(fontBold);
    SDV::TextFormat jsonNullOrBoolTextFormat
        {
            .textCharFormat{jsonNullOrBoolFormat},
            .richTextPrefix{QString{"<font color='%1'><b>"}.arg(colorDarkBlue.name())},
            .richTextSuffix{"</b></font>"}
        };

    QTextCharFormat jsonValueNumberFormat;
    const QColor colorBlue = QColor{0, 0, 255};
    jsonValueNumberFormat.setForeground(QBrush{colorBlue});
    SDV::TextFormat jsonValueNumberTextFormat
        {
            .textCharFormat{jsonValueNumberFormat},
            .richTextPrefix{QString{"<font color='%1'>"}.arg(colorBlue.name())},
            .richTextSuffix{"</font>"}
        };

    QTextCharFormat jsonValueStringFormat;
    const QColor colorGreen = QColor{0, 128, 0};
    jsonValueStringFormat.setForeground(QBrush{colorGreen});
    jsonValueStringFormat.setFont(fontBold);
    SDV::TextFormat jsonValueStringTextFormat
        {
            .textCharFormat{jsonValueStringFormat},
            .richTextPrefix{QString{"<font color='%1'><b>"}.arg(colorGreen.name())},
            .richTextSuffix{"</b></font>"}
        };

    QTextCharFormat jsonKeyFormat;
    const QColor& colorPurple = QColor{102, 14, 122};
    jsonKeyFormat.setForeground(QBrush{colorPurple});
    jsonKeyFormat.setFont(fontBold);
    SDV::TextFormat jsonKeyTextFormat
        {
            .textCharFormat{jsonKeyFormat},
            .richTextPrefix{QString{"<font color='%1'><b>"}.arg(colorPurple.name())},
            .richTextSuffix{"</b></font>"}
        };

    std::unordered_map<SDV::JsonNodeType, SDV::TextFormat> formatMap{};
    formatMap.emplace(SDV::JsonNodeType::Null, jsonNullOrBoolTextFormat);
    formatMap.emplace(SDV::JsonNodeType::Bool, jsonNullOrBoolTextFormat);
    formatMap.emplace(SDV::JsonNodeType::Number, jsonValueNumberTextFormat);
    formatMap.emplace(SDV::JsonNodeType::String, jsonValueStringTextFormat);
    formatMap.emplace(SDV::JsonNodeType::Key, jsonKeyTextFormat);

    return formatMap;
}

bool
isStdinNotTerminal()
{
// Ref: https://blog.kowalczyk.info/article/j/guide-to-predefined-macros-in-c-compilers-gcc-clang-msvc-etc..html
// Ref: https://sourceforge.net/p/predef/wiki/OperatingSystems/
#ifdef __linux__
    const int fd = fileno(stdin);
    // See also: help test -> [ -t 0 ]
    const bool x = ! isatty(fd);
    return x;
#else
    return false;
#endif  // __linux__
}

QString
getAbsFilePath(const QString& filePath)
{
    if (SDV::Constants::kStdinFileName == filePath) {
        return filePath;
    }
    else {
        const QFileInfo fi(filePath);
        // Ex: "/home/kca/saveme/qt5/structured-data-viewer/cmake-build-debug/data/twitter.json"
        const QString x = fi.absoluteFilePath();
        return x;
    }
}

std::unordered_set<QString>
createSet(QStringList* filePathList)
{
    std::unordered_set<QString> absFilePathSet{static_cast<std::size_t>(filePathList->size())};
    // Intentional: Also update each element of filePathList.
    for (QString& filePath : *filePathList) {

        filePath = getAbsFilePath(filePath);
        absFilePathSet.insert(filePath);
    }
    return absFilePathSet;
}

SDV::MainWindowInput
createInput(const QString& absFilePath)
{
    if (SDV::Constants::kStdinFileName == absFilePath) {
        return SDV::MainWindowInput::kStdin;
    }
    else {
        SDV::MainWindowInput x = SDV::MainWindowInput::createFile(absFilePath);
        return x;
    }
}

/*
Raise bug ticket.  Cannot view TextViewTextCursor in GDB.  Why?

Traceback (most recent call last):
  File "/home/kca/saveme/clion/latest/bin/gdb/renderers/libstdcxx/v6/printers.py", line 1300, in __call__
    return self.lookup[basename].invoke(val)
  File "/home/kca/saveme/clion/latest/bin/gdb/renderers/libstdcxx/v6/printers.py", line 1237, in invoke
    return self.function(self.name, value)
  File "/home/kca/saveme/clion/latest/bin/gdb/renderers/default/libstdcxx_printers.py", line 128, in __init__
    raise ValueError("Unsupported implementation for unique_ptr: %s" % impl_type)
ValueError: Unsupported implementation for unique_ptr: std::__uniq_ptr_data<SDV::TextViewGraphemeCursor, std::default_delete<SDV::TextViewGraphemeCursor>, true, true>
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCommandLineParser parser{};
    parser.setApplicationDescription(SDV::MainWindow::kWindowTitle);
    parser.addHelpOption();
    QCommandLineOption fileOption(QStringList() << "f" << "file", "Input file or - for stdin", "FILE_PATH");
    assert(parser.addOption(fileOption));
    // missing docs that automatically call showHelp(EXIT_SUCCESS)
    parser.process(app);

    SDV::MainWindowManager mainWindowManager{};
    SDV::MainWindowThreadWorker* const mainWindowThreadWorker = SDV::MainWindowThreadWorker::create();

    // Ex: [ ] or [ "-" ] or [ "data/twitter.json" ] or [ "-", "data/twitter.json" ]
    // @EmptyContainerAllowed
    QStringList filePathList = parser.values(fileOption);
    if (isStdinNotTerminal())
    {
        filePathList.append(SDV::Constants::kStdinFileName);
    }
    const std::unordered_map<SDV::JsonNodeType, SDV::TextFormat> formatMap{create()};

    if (filePathList.isEmpty())
    {
        SDV::MainWindow* mainWindow =
            new SDV::MainWindow(mainWindowManager, formatMap, mainWindowThreadWorker, SDV::MainWindowInput::kNone);
        mainWindow->adjustGeometry();
        mainWindow->show();
    }
    else {
        // Be *very* carefuly about uniqueness and ordering.
        // Ideally, we want to preserve original ordering, but remove absolute file path dupes.
        // A bit tricky...
        std::unordered_set<QString> absFilePathSet = createSet(&filePathList);
        SDV::MainWindow* prevMainWindow = nullptr;
        for (const QString& absFilePath : filePathList)
        {
            if (absFilePathSet.contains(absFilePath))
            {
                absFilePathSet.erase(absFilePath);
                SDV::MainWindowInput input = createInput(absFilePath);
                SDV::MainWindow* mainWindow = new SDV::MainWindow(mainWindowManager, formatMap, mainWindowThreadWorker, input);
                mainWindow->adjustGeometry(prevMainWindow);
                mainWindow->show();
                prevMainWindow = mainWindow;
            }
        }
    }
    const int x = app.exec();
    return x;
}
