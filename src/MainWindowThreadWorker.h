//
// Created by kca on 12/7/2020.
//

#ifndef SDV_MAINWINDOWTHREADWORKER_H
#define SDV_MAINWINDOWTHREADWORKER_H

#include <QObject>
#include "rapidjson/document.h"
#include <rapidjson/error/error.h>
#include "TextViewTextStatsService.h"
#include "MainWindowInputStream.h"
#include "Constants.h"
#include "TextFormat.h"
#include "ThreadSafeSharedPointerMap.h"

namespace SDV {

class MainWindowInputStream;
class TextViewDocument;
class JsonTree;
class MainWindow;

class MainWindowThreadWorker : public QObject
{
    Q_OBJECT

public:
    using Base = QObject;
    static MainWindowThreadWorker* create(const std::unordered_map<JsonNodeType, TextFormat>& formatMap);
    ~MainWindowThreadWorker() override = default;

    int insertNewTextStatsServiceOrAssert(const std::shared_ptr<TextViewTextStatsService>& p);
    void eraseTextStatsServiceOrAssert(const int textStatsServiceId);
    int nextRequestId();

    struct OpenResult
    {
        int requestId;
        rapidjson::ParseResult jsonParseResult;
        std::shared_ptr<JsonTree> jsonTree;
        std::shared_ptr<TextViewDocument> doc;
        TextViewTextStatsService::Result textStats;
    };

    struct OpenClipboardTextResult : public OpenResult {};

    struct OpenInputStreamResult : public OpenResult
    {
        MainWindowInputStream inputStream;
    };

    struct CalcTextSelectionStatsResult
    {
        int requestId;
        TextViewTextStatsService::Result textStats;
    };

public slots:
    void slotOpenInputStream(int requestId, int textStatsServiceId, const MainWindowInputStream& inputStream);
    void slotOpenClipboardText(int requestId, int textStatsServiceId, const QString& text);
    void slotCalcTextSelectionStats(int requestId, int textStatsServiceId, const TextViewSelection& selection);

signals:
    void signalOpenInputStreamComplete(const OpenInputStreamResult& result);
    void signalOpenClipboardTextComplete(const OpenClipboardTextResult& result);
    void signalCalcTextSelectionStatsComplete(const CalcTextSelectionStatsResult& result);

private:
    // TODO: Move 'formatMap' to a global singleton to allow updates at runtime and read/write to config files
    MainWindowThreadWorker(const std::unordered_map<JsonNodeType, TextFormat>& formatMap);

    struct Private;
    const std::unordered_map<JsonNodeType, TextFormat>& m_formatMap;
    ThreadSafeSharedPointerMap<TextViewTextStatsService> m_textViewTextStatsServiceMap;
    NextIdService m_nextIdService;
};

}  // namespace SDV

#endif //SDV_MAINWINDOWTHREADWORKER_H
