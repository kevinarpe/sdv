//
// Created by kca on 12/7/2020.
//

#include "MainWindowThreadWorker.h"
#include <istream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <QRegularExpression>
#include <QApplication>
#include <QThread>
#include "PrettyWriter2.h"
#include "TextViewDocument.h"
#include "TextViewDocumentView.h"
#include "TextViewTextStatsService.h"
#include "QStrings.h"

namespace SDV {

// Ref: https://stackoverflow.com/a/22056222/257299
static const int z1 =
//    qRegisterMetaType<MainWindowThreadWorker::OpenInputStreamResult>("SDV::MainWindowThreadWorker::OpenInputStreamResult");
//    qRegisterMetaType<MainWindowThreadWorker::OpenInputStreamResult>("MainWindowThreadWorker::OpenInputStreamResult");
    qRegisterMetaType<MainWindowThreadWorker::OpenInputStreamResult>("OpenInputStreamResult");

static const int z2 =
    qRegisterMetaType<MainWindowThreadWorker::OpenClipboardTextResult>("OpenClipboardTextResult");

static const int z3 =
    qRegisterMetaType<MainWindowThreadWorker::CalcTextSelectionStatsResult>("CalcTextSelectionStatsResult");

struct MainWindowThreadWorker::Private
{
    static void
    open(MainWindowThreadWorker& self,
         const rapidjson::Document& doc,
         const int utf8BufferCapacity,
         const int textStatsServiceId,
         OpenResult* result)
    {
        assert(nullptr != result);

        result->jsonParseResult = rapidjson::ParseResult{doc};
        if (doc.HasParseError())
        {
            result->textStats.invalidate();
            return;
        }
        rapidjson::StringBuffer sb{nullptr, static_cast<size_t>(utf8BufferCapacity)};
        PrettyWriter2 pw{sb, static_cast<std::size_t>(utf8BufferCapacity), self.m_formatMap};
        assert(doc.Accept(pw));
        result->jsonTree = pw.result();

        // TODO: Can this very wasteful split be avoided within PrettyWriter2?
        const QStringList& lineList = result->jsonTree->jsonText.split(QRegularExpression{"\\r?\\n"});
        std::vector<QString> lineVec{lineList.begin(), lineList.end()};
        result->doc = std::make_shared<TextViewDocument>(std::move(lineVec));

        const std::shared_ptr<TextViewTextStatsService>& tss = self.m_textViewTextStatsServiceMap.getOrAssert(textStatsServiceId);
        result->textStats = tss->setDoc(result->doc);
    }
};

// public static
MainWindowThreadWorker*
MainWindowThreadWorker::
create(const std::unordered_map<JsonNodeType, TextFormat>& formatMap)
{
    MainWindowThreadWorker* const self = new MainWindowThreadWorker{formatMap};
    QThread* const thread = new QThread{QCoreApplication::instance()};
    // This *sometimes* works on Linux using CLion/GDB.
    // Ref: https://stackoverflow.com/a/26179765/257299
    thread->setObjectName("main-window-worker");
    // Order of these two statements does not seem to matter.
    thread->start();
    // This will re-parent 'self' from nullptr to 'thread'.
    self->moveToThread(thread);
    // Intentional: Do not delete 'this'.  Why?  It will cause race conditions when other objects that hold a reference to 'this' try to
    // call 'eraseTextStatsServiceOrAssert(...)' from their own destructor.
//    QObject::connect(thread, &QThread::finished, self, &QObject::deleteLater);

//    qWarning("thread(%p)", thread);

    // It *seems* like QueuedConnection is not required, but this is only by empirical observation.
//    QObject::connect(qApp, &QApplication::lastWindowClosed, thread, &QThread::quit, Qt::ConnectionType::QueuedConnection);
    QObject::connect(qApp, &QApplication::lastWindowClosed, thread, &QThread::quit);
    return self;
}

// private
MainWindowThreadWorker::
MainWindowThreadWorker(const std::unordered_map<JsonNodeType, TextFormat>& formatMap)
    : Base{nullptr},
      m_formatMap{formatMap},
      m_nextIdService{NextIdService::withFirstId(1)}
{}

// public
int
MainWindowThreadWorker::
insertNewTextStatsServiceOrAssert(const std::shared_ptr<TextViewTextStatsService>& p)
{
    const int x = m_textViewTextStatsServiceMap.insertNewOrAssert(p);
    return x;
}

// public
void
MainWindowThreadWorker::
eraseTextStatsServiceOrAssert(const int textStatsServiceId)
{
    m_textViewTextStatsServiceMap.eraseOrAssert(textStatsServiceId);
}

// public
int
MainWindowThreadWorker::
nextRequestId()
{
    const int x = m_nextIdService.nextId();
    return x;
}

// public slot
void
MainWindowThreadWorker::
slotOpenInputStream(const int requestId, const int textStatsServiceId, const MainWindowInputStream& inputStream)
{
    OpenInputStreamResult result{};
    result.requestId = requestId;
    result.inputStream = inputStream;

    // Ref: https://stackoverflow.com/a/45257251/257299
    rapidjson::IStreamWrapper isw{inputStream.inputStream()};
    rapidjson::Document doc{};
    doc.ParseStream(isw);

    Private::open(*this, doc, inputStream.bufferCapacity(), textStatsServiceId, &result);
    emit signalOpenInputStreamComplete(result);
}

// public slot
void
MainWindowThreadWorker::
slotOpenClipboardText(const int requestId, const int textStatsServiceId, const QString& text)
{
    assert(text.length() > 0);

    OpenClipboardTextResult result{};
    result.requestId = requestId;

    rapidjson::Document doc{};
    const char* str = qUtf8Printable(text);
    doc.Parse(str);
    const int utf8ByteCount = QStrings::utf8ByteCount(text);

    Private::open(*this, doc, utf8ByteCount, textStatsServiceId, &result);
    emit signalOpenClipboardTextComplete(result);
}

// public slot
void
MainWindowThreadWorker::
slotCalcTextSelectionStats(const int requestId, const int textStatsServiceId, const TextViewSelection& selection)
{
    const std::shared_ptr<TextViewTextStatsService>& tss = m_textViewTextStatsServiceMap.getOrAssert(textStatsServiceId);
    const TextViewTextStatsService::Result textStats = tss->calcStats(selection);

    const CalcTextSelectionStatsResult result{.requestId = requestId, .textStats = textStats};
    emit signalCalcTextSelectionStatsComplete(result);
}

}  // namespace SDV
