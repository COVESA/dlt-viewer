// Dispatches MCP tool calls and validates their arguments before accessing DLT Viewer.
// Short operations run synchronously and return their response immediately.

#include "dltmcpplugin.h"
#include "dltmcpplugin_tools_common.h"
#include <QAbstractItemModel>
#include <QDateTime>
#include <QDebug>
#include <cmath>
#include <limits>

namespace {

QString connectionStateToString(QDltConnection::QDltConnectionState state) {
    switch (state) {
    case QDltConnection::QDltConnectionOffline:
        return "offline";
    case QDltConnection::QDltConnectionConnecting:
        return "connecting";
    case QDltConnection::QDltConnectionOnline:
        return "online";
    case QDltConnection::QDltConnectionError:
        return "error";
    default:
        return "unknown";
    }
}

struct ParsedTimestamp {
    bool isValid;
    bool timeOnly;
    QDateTime dateTime;
    QTime timeOfDay;

    ParsedTimestamp() : isValid(false), timeOnly(false) {}
};

ParsedTimestamp parseStrictTimestamp(const QString& timestampText, QString& errorMessage) {
    ParsedTimestamp parsed;
    const QString trimmed = timestampText.trimmed();
    if (trimmed.isEmpty()) {
        errorMessage = "Timestamp cannot be empty";
        return parsed;
    }

    const QStringList dateTimeFormats = QStringList() << "yyyy/MM/dd HH:mm:ss.zzz"
                                                      << "yyyy/MM/dd HH:mm:ss";
    for (int i = 0; i < dateTimeFormats.size(); ++i) {
        const QDateTime dateTime = QDateTime::fromString(trimmed, dateTimeFormats.at(i));
        if (dateTime.isValid()) {
            parsed.isValid = true;
            parsed.timeOnly = false;
            parsed.dateTime = dateTime;
            parsed.dateTime.setTimeSpec(Qt::LocalTime);
            parsed.timeOfDay = parsed.dateTime.time();
            return parsed;
        }
    }

    const QStringList timeFormats = QStringList() << "HH:mm:ss.zzz"
                                                  << "HH:mm:ss";
    for (int i = 0; i < timeFormats.size(); ++i) {
        const QTime time = QTime::fromString(trimmed, timeFormats.at(i));
        if (time.isValid()) {
            parsed.isValid = true;
            parsed.timeOnly = true;
            parsed.timeOfDay = time;
            return parsed;
        }
    }

    errorMessage = "Unsupported timestamp format. Use yyyy/MM/dd HH:mm:ss[.zzz] or HH:mm:ss[.zzz].";
    return parsed;
}

bool canMutateFiltersSafely(const QDltFile* dltFile, int filterMutationMessageLimit,
                            QString& errorMessage) {
    if (!dltFile)
        return true;

    // Mutating filters triggers a synchronous full filtered-index rebuild in DLT Viewer.
    // On very large traces this can freeze the UI, so MCP blocks it above a size limit.
    const int totalMessages = dltFile->size();
    if (filterMutationMessageLimit < 0 || totalMessages <= filterMutationMessageLimit)
        return true;

    errorMessage = QString("Filter mutation via MCP is blocked for this trace because rebuilding "
                           "the filtered view for %1 messages may freeze DLT Viewer. Use "
                           "read/search tools with use_filter=false for temporary unfiltered "
                           "analysis, or change filters manually in the DLT Viewer UI.")
                       .arg(totalMessages);
    return false;
}

} // namespace

bool DltMcpPlugin::ensureDltControl(QJsonObject& response) const {
    if (dltControl)
        return true;
    response["error"] =
        makeJsonRpcError(JsonRpcError::InternalError, "DLT Viewer control interface not available");
    return false;
}

bool DltMcpPlugin::ensureIndexInRange(int index, int size, const QString& label,
                                      QJsonObject& response) const {
    if (index >= 0 && index < size)
        return true;
    response["error"] = makeJsonRpcError(
        JsonRpcError::InvalidParams,
        QString("%1 index %2 is out of range (0\u2013%3)").arg(label).arg(index).arg(size - 1));
    return false;
}

bool DltMcpPlugin::readMessageAt(int index, bool useFilter, QDltMsg& msg) const {
    // The input index belongs to the requested view; msg.index always identifies
    // the underlying file message, even when filters hide preceding rows.
    bool ok = false;
    if (useFilter) {
        const QByteArray buf = dltFile->getMsgFilter(index);
        ok = !buf.isEmpty();
        if (ok) {
            msg.setMsg(buf);
            msg.setIndex(dltFile->getMsgFilterPos(index));
        }
    } else {
        ok = dltFile->getMsg(index, msg);
        if (ok)
            msg.setIndex(index);
    }
    return ok;
}

bool DltMcpPlugin::isMonotonicOrderLikelySorted(bool useFilter, int total) const {
    // Sampling detects some timestamp resets or interleaving, but does not prove
    // ordering. Around-index window expansion uses this only as a heuristic.
    if (total <= 1)
        return true;

    const int sampleCount = qMin(total, 64);
    const int step = qMax(1, total / sampleCount);
    bool hasPrevious = false;
    qint64 previous = 0;

    for (int i = 0; i < total; i += step) {
        QDltMsg msg;
        if (!readMessageAt(i, useFilter, msg))
            continue;
        const qint64 current = monotonicTimestampValue(msg);
        if (hasPrevious && current < previous)
            return false;
        previous = current;
        hasPrevious = true;
    }

    return true;
}

int DltMcpPlugin::findAnchorIndex(const QString& timestampText, bool useFilter, int total,
                                  qint64& bestDistanceMs, QDateTime& matchedDateTime,
                                  QDltMsg& matchedMsg, bool& timeOnly,
                                  QString& errorMessage) const {
    const ParsedTimestamp parsed = parseStrictTimestamp(timestampText, errorMessage);
    if (!parsed.isValid)
        return -1;

    timeOnly = parsed.timeOnly;
    bestDistanceMs = 0;
    matchedDateTime = QDateTime();
    matchedMsg.clear();

    int bestIndex = -1;
    bool hasBest = false;

    for (int i = 0; i < total; ++i) {
        QDltMsg msg;
        QDateTime messageDateTime;
        if (!readMessageAt(i, useFilter, msg) || !readWallClockDateTime(msg, messageDateTime))
            continue;

        qint64 distanceMs = 0;
        if (parsed.timeOnly) {
            distanceMs = timeOfDayDistanceMs(messageDateTime.time(), parsed.timeOfDay);
        } else {
            distanceMs = qAbs(messageDateTime.msecsTo(parsed.dateTime));
        }

        if (!hasBest || distanceMs < bestDistanceMs) {
            bestIndex = i;
            bestDistanceMs = distanceMs;
            matchedDateTime = messageDateTime;
            matchedMsg = msg;
            hasBest = true;
        }
    }

    if (!hasBest && errorMessage.isEmpty())
        errorMessage = "No messages with usable wall-clock timestamps were found";

    return bestIndex;
}

// Returns the lazily-initialised MCP tool dispatch table. Each entry maps
// a tool name to its handler member function and describes whether the
// handler requires an open DLT file and whether it manages its own response
// emission (async) or relies on onServerMessage to emit (sync).
const QHash<QString, DltMcpPlugin::ToolEntry>& DltMcpPlugin::toolHandlerTable() const {
    static const QHash<QString, ToolEntry> table = {
        // name                            handler requiresFile  async
        {"get_context", {&DltMcpPlugin::handleGetContext, true, false}},
        {"get_dictionary", {&DltMcpPlugin::handleGetDictionary, false, false}},
        {"list_connections", {&DltMcpPlugin::handleListConnections, false, false}},
        {"get_connection_states", {&DltMcpPlugin::handleGetConnectionStates, false, false}},
        {"get_file_info", {&DltMcpPlugin::handleGetFileInfo, true, false}},
        {"get_messages_range", {&DltMcpPlugin::handleGetMessagesRange, true, false}},
        {"search_logs", {&DltMcpPlugin::handleSearchLogs, true, true}},
        {"find_message_by_timestamp", {&DltMcpPlugin::handleFindMessageByTimestamp, true, true}},
        {"search_logs_around_index", {&DltMcpPlugin::handleSearchLogsAroundIndex, true, true}},
        {"get_error_messages", {&DltMcpPlugin::handleGetErrorMessages, true, true}},
        {"list_filters", {&DltMcpPlugin::handleListFilters, true, false}},
        {"add_filter", {&DltMcpPlugin::handleAddFilter, true, false}},
        {"remove_filter", {&DltMcpPlugin::handleRemoveFilter, true, false}},
        {"set_filter_enabled", {&DltMcpPlugin::handleSetFilterEnabled, true, false}},
        {"connect_ecu", {&DltMcpPlugin::handleConnectEcu, false, false}},
        {"disconnect_ecu", {&DltMcpPlugin::handleDisconnectEcu, false, false}},
        {"connect_all_ecus", {&DltMcpPlugin::handleConnectAllEcus, false, false}},
        {"disconnect_all_ecus", {&DltMcpPlugin::handleDisconnectAllEcus, false, false}},
        {"navigate_to_message", {&DltMcpPlugin::handleNavigateToMessage, true, false}},
        {"clear_logs", {&DltMcpPlugin::handleClearLogs, false, false}},
    };
    return table;
}

// Handles MCP tools/call messages by looking up the requested tool in
// toolHandlerTable() and delegating to its handler. Sync handlers only
// populate `response`; async handlers emit their own response (immediately
// on validation errors, or later from their chunked job driver).
void DltMcpPlugin::onServerMessage(const QString& clientId, const QJsonObject& message) {
    const QJsonObject params = message["params"].toObject();
    const QString toolName = params["name"].toString();
    const QJsonObject arguments = params["arguments"].toObject();

    const QHash<QString, ToolEntry>& table = toolHandlerTable();
    const auto it = table.constFind(toolName);
    const bool isKnownAsync = (it != table.constEnd()) && it.value().async;

    const QString reqKey = requestIdKey(message["id"]);
    pendingToolNamesByRequestId.insert(reqKey, toolName);
    latestToolCallRequestKey = reqKey;
    setLastToolCallStatus(toolName,
                          isKnownAsync ? QStringLiteral("running") : QStringLiteral("processing"));

    QJsonObject response;
    response["jsonrpc"] = "2.0";
    response["id"] = message["id"];

    if (it == table.constEnd()) {
        response["error"] = makeJsonRpcError(JsonRpcError::InvalidParams,
                                             QString("Unknown tool: %1").arg(toolName));
        emit responseReady(clientId, response);
        return;
    }

    const ToolEntry& entry = it.value();
    if (entry.requiresFile && !dltFile) {
        response["error"] =
            makeJsonRpcError(JsonRpcError::InternalError, "No DLT file is open in DLT Viewer");
        emit responseReady(clientId, response);
        return;
    }

    (this->*entry.handler)(clientId, message, arguments, response);

    // Async handlers own their own response emission.
    if (!entry.async)
        emit responseReady(clientId, response);
}

// === Tool handlers ===

void DltMcpPlugin::handleGetContext(const QString& clientId, const QJsonObject& message,
                                    const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    Q_UNUSED(arguments);
    const QJsonDocument doc(buildContextData());
    response["result"] = makeTextResult(QString::fromUtf8(doc.toJson(QJsonDocument::Indented)));
}

void DltMcpPlugin::handleGetDictionary(const QString& clientId, const QJsonObject& message,
                                       const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    response["result"] = makeTextResult(buildDictionaryText(arguments["term"].toString()));
}

void DltMcpPlugin::handleListConnections(const QString& clientId, const QJsonObject& message,
                                         const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    Q_UNUSED(arguments);
    QJsonArray connections;
    for (int i = 0; i < configuredConnections.size(); ++i) {
        QJsonObject connection;
        connection["index"] = i;
        connection["description"] = configuredConnections.at(i);
        const QString hostname = connectionHostnames.value(i);
        if (!hostname.isEmpty())
            connection["hostname"] = hostname;
        connections.append(connection);
    }

    const QJsonDocument doc(connections);
    response["result"] =
        makeTextResult(QString("%1 configured connection(s):\n%2")
                           .arg(connections.size())
                           .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented))));
}

void DltMcpPlugin::handleGetConnectionStates(const QString& clientId, const QJsonObject& message,
                                             const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    Q_UNUSED(arguments);
    QJsonArray states;
    for (int i = 0; i < configuredConnections.size(); ++i) {
        const QDltConnection::QDltConnectionState state =
            connectionStates.value(i, QDltConnection::QDltConnectionOffline);
        QJsonObject connection;
        connection["index"] = i;
        connection["description"] = configuredConnections.at(i);
        connection["state"] = connectionStateToString(state);
        connection["state_code"] = static_cast<int>(state);
        const QString hostname = connectionHostnames.value(i);
        if (!hostname.isEmpty())
            connection["hostname"] = hostname;
        states.append(connection);
    }

    const QJsonDocument doc(states);
    response["result"] =
        makeTextResult(QString("Connection states for %1 configured ECU(s):\n%2")
                           .arg(states.size())
                           .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented))));
}

void DltMcpPlugin::handleGetFileInfo(const QString& clientId, const QJsonObject& message,
                                     const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    Q_UNUSED(arguments);
    const int total = dltFile->size();
    const int filtered = dltFile->sizeFilter();
    const bool filterActive = (filtered != total);

    response["result"] =
        makeTextResult(QString("DLT file info:\n"
                               "  Total messages   : %1\n"
                               "  Filtered messages: %2\n"
                               "  Filter active    : %3\n"
                               "\n"
                               "Use get_messages_range to read a slice of the filtered view, "
                               "or search_logs / get_error_messages to search within it.")
                           .arg(total)
                           .arg(filtered)
                           .arg(filterActive ? "yes" : "no"));
}

void DltMcpPlugin::handleGetMessagesRange(const QString& clientId, const QJsonObject& message,
                                          const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    const bool useFilter = arguments["use_filter"].toBool(true);
    const int total = useFilter ? dltFile->sizeFilter() : dltFile->size();
    const int start = arguments["start"].toInt(0);
    const int end = qMin(arguments["end"].toInt(start + 100), total);
    // Keep each call bounded to avoid large synchronous decode/JSON payloads that can stall UI.
    const int maxPerCall = 500;
    const int actualEnd = qMin(end, start + maxPerCall);

    QJsonArray results;
    for (int i = start; i < actualEnd; ++i) {
        QDltMsg msg;
        if (!readMessageAt(i, useFilter, msg))
            continue;
        if (messageDecoder)
            messageDecoder->decodeMsg(msg, 0);

        QJsonObject msgObj = buildMessageObject(msg, i, useFilter);
        results.append(msgObj);
    }

    const QJsonDocument doc(results);
    response["result"] =
        makeTextResult(QString("Messages %1\u2013%2 of %3 (%4):\n%5")
                           .arg(start)
                           .arg(actualEnd - 1)
                           .arg(total)
                           .arg(useFilter ? "filtered" : "unfiltered")
                           .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented))));
}

void DltMcpPlugin::handleSearchLogs(const QString& clientId, const QJsonObject& message,
                                    const QJsonObject& arguments, QJsonObject& response) {
    const QString pattern = arguments["pattern"].toString();
    const QString ecuid = arguments["ecuid"].toString().trimmed();
    const QString apid = arguments["apid"].toString().trimmed();
    const QString ctid = arguments["ctid"].toString().trimmed();
    const QString type = arguments["type"].toString().trimmed();
    const QString subtype = arguments["subtype"].toString().trimmed();
    const QString mode = arguments["mode"].toString().trimmed();
    const QString sessionId = arguments["session_id"].toString().trimmed();
    const bool useFilter = arguments["use_filter"].toBool(true);
    const int total = useFilter ? dltFile->sizeFilter() : dltFile->size();
    const int startIndex = qMax(0, arguments["start_index"].toInt(0));
    const int defaultMaxScan = useFilter ? total : 10000;
    const int maxScan = qMax(1, arguments["max_scan"].toInt(defaultMaxScan));
    const int maxResults = qBound(1, arguments["max_results"].toInt(100), 200);
    if (pattern.trimmed().isEmpty() && ecuid.isEmpty() && apid.isEmpty() && ctid.isEmpty() &&
        type.isEmpty() && subtype.isEmpty() && mode.isEmpty() && sessionId.isEmpty()) {
        response["error"] = makeJsonRpcError(
            JsonRpcError::InvalidParams,
            "search_logs requires a non-empty pattern or at least one header filter");
        emit responseReady(clientId, response);
        return;
    }

    if (startIndex >= total) {
        response["error"] = makeJsonRpcError(
            JsonRpcError::InvalidParams, QString("start_index %1 is out of range for the %2 view")
                                             .arg(startIndex)
                                             .arg(useFilter ? "filtered" : "unfiltered"));
        emit responseReady(clientId, response);
        return;
    }

    QRegularExpression patternRegex;
    QString regexErrorMessage;
    if (!compileSearchPatternRegex(pattern, patternRegex, regexErrorMessage)) {
        response["error"] = makeJsonRpcError(JsonRpcError::InvalidParams, regexErrorMessage);
        emit responseReady(clientId, response);
        return;
    }

    // Concurrency policy: latest request wins for these long-running scans.
    // search_logs and get_error_messages share execution capacity and cancel each other.
    if (searchLogsJob.env.active)
        failJob(searchLogsJob,
                "search_logs was cancelled because a newer search_logs request replaced it");
    if (getErrorMessagesJob.env.active)
        failJob(getErrorMessagesJob,
                "get_error_messages was cancelled because a newer search_logs request replaced it");

    const quint64 generation = nextJobGeneration++;
    searchLogsJob = SearchLogsJob();
    searchLogsJob.env.active = true;
    searchLogsJob.env.clientId = clientId;
    searchLogsJob.env.requestId = message["id"];
    searchLogsJob.env.generation = generation;
    searchLogsJob.pattern = pattern;
    searchLogsJob.ecuid = ecuid;
    searchLogsJob.apid = apid;
    searchLogsJob.ctid = ctid;
    searchLogsJob.type = type;
    searchLogsJob.subtype = subtype;
    searchLogsJob.mode = mode;
    searchLogsJob.sessionId = sessionId;
    searchLogsJob.env.useFilter = useFilter;
    searchLogsJob.env.total = total;
    searchLogsJob.env.nextIndex = startIndex;
    searchLogsJob.startIndex = startIndex;
    searchLogsJob.scanLimit = startIndex + qMin(maxScan, total - startIndex);
    searchLogsJob.maxResults = maxResults;
    emit viewerStateChanged();

    scheduleChunk(searchLogsJob, &DltMcpPlugin::processSearchLogsChunk);
}

void DltMcpPlugin::handleFindMessageByTimestamp(const QString& clientId, const QJsonObject& message,
                                                const QJsonObject& arguments,
                                                QJsonObject& response) {
    const QString timestampText = arguments["timestamp"].toString();
    const bool useFilter = arguments["use_filter"].toBool(true);
    const int total = useFilter ? dltFile->sizeFilter() : dltFile->size();
    QString errorMessage;
    const ParsedTimestamp parsed = parseStrictTimestamp(timestampText, errorMessage);
    if (!parsed.isValid) {
        const int parseErrorCode =
            (errorMessage.startsWith("Unsupported") || errorMessage.startsWith("Timestamp cannot"))
                ? JsonRpcError::InvalidParams
                : JsonRpcError::InternalError;
        response["error"] = makeJsonRpcError(parseErrorCode, errorMessage);
        emit responseReady(clientId, response);
        return;
    }

    if (findMessageByTimestampJob.env.active)
        failJob(findMessageByTimestampJob, "find_message_by_timestamp was cancelled because a "
                                           "newer find_message_by_timestamp request replaced it");

    const quint64 generation = nextJobGeneration++;
    findMessageByTimestampJob = FindMessageByTimestampJob();
    findMessageByTimestampJob.env.active = true;
    findMessageByTimestampJob.env.clientId = clientId;
    findMessageByTimestampJob.env.requestId = message["id"];
    findMessageByTimestampJob.env.generation = generation;
    findMessageByTimestampJob.timestampText = timestampText;
    findMessageByTimestampJob.env.useFilter = useFilter;
    findMessageByTimestampJob.env.total = total;
    findMessageByTimestampJob.timeOnly = parsed.timeOnly;
    findMessageByTimestampJob.requestedTimeOfDay = parsed.timeOfDay;
    findMessageByTimestampJob.requestedDateTime = parsed.dateTime;
    emit viewerStateChanged();

    scheduleChunk(findMessageByTimestampJob, &DltMcpPlugin::processFindMessageByTimestampChunk);
}

void DltMcpPlugin::handleSearchLogsAroundIndex(const QString& clientId, const QJsonObject& message,
                                               const QJsonObject& arguments,
                                               QJsonObject& response) {
    const int index = arguments["index"].toInt(-1);
    const QString pattern = arguments["pattern"].toString();
    const QString ecuid = arguments["ecuid"].toString().trimmed();
    const QString apid = arguments["apid"].toString().trimmed();
    const QString ctid = arguments["ctid"].toString().trimmed();
    const QString type = arguments["type"].toString().trimmed();
    const QString subtype = arguments["subtype"].toString().trimmed();
    const QString mode = arguments["mode"].toString().trimmed();
    const QString sessionId = arguments["session_id"].toString().trimmed();
    const bool useFilter = arguments["use_filter"].toBool(true);
    const int total = dltFile->size();
    const int scopedTotal = useFilter ? dltFile->sizeFilter() : total;
    const int maxResults = qBound(1, arguments["max_results"].toInt(100), 200);
    const qint64 windowBefore =
        static_cast<qint64>(qMax(0, arguments["window_before_ms"].toInt(5000)));
    const qint64 windowAfter =
        static_cast<qint64>(qMax(0, arguments["window_after_ms"].toInt(5000)));

    if (index < 0 || index >= total) {
        response["error"] = makeJsonRpcError(JsonRpcError::InvalidParams,
                                             QString("Global index %1 is out of range (0\u2013%2)")
                                                 .arg(index)
                                                 .arg(total - 1));
        emit responseReady(clientId, response);
        return;
    }

    if (pattern.trimmed().isEmpty() && ecuid.isEmpty() && apid.isEmpty() && ctid.isEmpty() &&
        type.isEmpty() && subtype.isEmpty() && mode.isEmpty() && sessionId.isEmpty()) {
        response["error"] = makeJsonRpcError(
            JsonRpcError::InvalidParams,
            "search_logs_around_index requires a non-empty pattern or at least one header filter");
        emit responseReady(clientId, response);
        return;
    }

    QRegularExpression patternRegex;
    QString regexErrorMessage;
    if (!compileSearchPatternRegex(pattern, patternRegex, regexErrorMessage)) {
        response["error"] = makeJsonRpcError(JsonRpcError::InvalidParams, regexErrorMessage);
        emit responseReady(clientId, response);
        return;
    }

    int scopedAnchorIndex = index;
    if (useFilter) {
        const QVector<qint64> filterIndices = dltFile->getIndexFilter();
        scopedAnchorIndex = filterIndices.indexOf(static_cast<qint64>(index));
        if (scopedAnchorIndex < 0) {
            response["error"] = makeJsonRpcError(
                JsonRpcError::InvalidParams,
                QString("Global index %1 is not visible in the current filtered scope")
                    .arg(index));
            emit responseReady(clientId, response);
            return;
        }
    }

    QDltMsg anchorMsg;
    if (!readMessageAt(index, false, anchorMsg)) {
        response["error"] =
            makeJsonRpcError(JsonRpcError::InternalError,
                             QString("Failed to read anchor message at global index %1")
                                 .arg(index));
        emit responseReady(clientId, response);
        return;
    }

    QDateTime anchorDateTime;
    readWallClockDateTime(anchorMsg, anchorDateTime);

    if (searchLogsAroundIndexJob.env.active)
        failJob(searchLogsAroundIndexJob, "search_logs_around_index was cancelled because a newer "
                                          "search_logs_around_index request replaced it");

    const quint64 generation = nextJobGeneration++;
    searchLogsAroundIndexJob = SearchLogsAroundIndexJob();
    searchLogsAroundIndexJob.env.active = true;
    searchLogsAroundIndexJob.env.clientId = clientId;
    searchLogsAroundIndexJob.env.requestId = message["id"];
    searchLogsAroundIndexJob.env.generation = generation;
    searchLogsAroundIndexJob.pattern = pattern;
    searchLogsAroundIndexJob.ecuid = ecuid;
    searchLogsAroundIndexJob.apid = apid;
    searchLogsAroundIndexJob.ctid = ctid;
    searchLogsAroundIndexJob.type = type;
    searchLogsAroundIndexJob.subtype = subtype;
    searchLogsAroundIndexJob.mode = mode;
    searchLogsAroundIndexJob.sessionId = sessionId;
    searchLogsAroundIndexJob.env.useFilter = useFilter;
    searchLogsAroundIndexJob.env.total = scopedTotal;
    searchLogsAroundIndexJob.maxResults = maxResults;
    searchLogsAroundIndexJob.windowBeforeMs = windowBefore;
    searchLogsAroundIndexJob.windowAfterMs = windowAfter;
    searchLogsAroundIndexJob.bestIndex = index;
    searchLogsAroundIndexJob.hasBest = true;
    searchLogsAroundIndexJob.matchedDateTime = anchorDateTime;
    searchLogsAroundIndexJob.matchedMsg = anchorMsg;
    searchLogsAroundIndexJob.anchorMonotonic = monotonicTimestampValue(anchorMsg);
    searchLogsAroundIndexJob.lowerBound =
        searchLogsAroundIndexJob.anchorMonotonic - (windowBefore * 10);
    searchLogsAroundIndexJob.upperBound =
        searchLogsAroundIndexJob.anchorMonotonic + (windowAfter * 10);
    searchLogsAroundIndexJob.likelySorted =
        isMonotonicOrderLikelySorted(useFilter, scopedTotal);
    searchLogsAroundIndexJob.startIndex = scopedAnchorIndex;
    searchLogsAroundIndexJob.endIndex = scopedAnchorIndex;
    searchLogsAroundIndexJob.phase = SearchLogsAroundIndexJob::ExpandingStart;
    searchLogsAroundIndexJob.env.nextIndex = scopedAnchorIndex - 1;
    emit viewerStateChanged();

    scheduleChunk(searchLogsAroundIndexJob, &DltMcpPlugin::processSearchLogsAroundIndexChunk);
}

void DltMcpPlugin::handleGetErrorMessages(const QString& clientId, const QJsonObject& message,
                                          const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(response);
    const bool useFilter = arguments["use_filter"].toBool(true);
    const int total = useFilter ? dltFile->sizeFilter() : dltFile->size();
    const int maxResults = qBound(1, arguments["max_results"].toInt(100), 200);
    if (searchLogsJob.env.active)
        failJob(searchLogsJob,
                "search_logs was cancelled because a newer get_error_messages request replaced it");
    if (getErrorMessagesJob.env.active)
        failJob(getErrorMessagesJob, "get_error_messages was cancelled because a newer "
                                     "get_error_messages request replaced it");

    const quint64 generation = nextJobGeneration++;
    getErrorMessagesJob = GetErrorMessagesJob();
    getErrorMessagesJob.env.active = true;
    getErrorMessagesJob.env.clientId = clientId;
    getErrorMessagesJob.env.requestId = message["id"];
    getErrorMessagesJob.env.generation = generation;
    getErrorMessagesJob.env.useFilter = useFilter;
    getErrorMessagesJob.env.total = total;
    getErrorMessagesJob.maxResults = maxResults;
    emit viewerStateChanged();

    scheduleChunk(getErrorMessagesJob, &DltMcpPlugin::processGetErrorMessagesChunk);
}

void DltMcpPlugin::handleListFilters(const QString& clientId, const QJsonObject& message,
                                     const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    Q_UNUSED(arguments);
    const QDltFilterList filterList = dltFile->getFilterList();
    QJsonArray filterArray;
    for (int i = 0; i < filterList.filters.size(); ++i) {
        const QDltFilter* f = filterList.filters.at(i);
        QJsonObject fObj;
        fObj["index"] = i;
        fObj["name"] = f->name;
        fObj["type"] = f->isPositive() ? "positive" : f->isNegative() ? "negative" : "marker";
        fObj["enabled"] = f->enableFilter;
        fObj["ecuid"] = f->enableEcuid ? f->ecuid : QString();
        fObj["apid"] = f->enableApid ? f->apid : QString();
        fObj["ctid"] = f->enableCtid ? f->ctid : QString();
        fObj["header"] = f->enableHeader ? f->header : QString();
        fObj["payload"] = f->enablePayload ? f->payload : QString();
        filterArray.append(fObj);
    }
    const QJsonDocument doc(filterArray);
    response["result"] =
        makeTextResult(QString("%1 filter(s) configured:\n%2")
                           .arg(filterArray.size())
                           .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented))));
}

void DltMcpPlugin::handleAddFilter(const QString& clientId, const QJsonObject& message,
                                   const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    QString filterMutationError;
    if (!canMutateFiltersSafely(dltFile, filterMutationMessageLimit, filterMutationError)) {
        response["error"] = makeJsonRpcError(JsonRpcError::InternalError, filterMutationError);
        return;
    }

    const QString typeName = arguments["type"].toString("positive");
    QDltFilter* f = new QDltFilter();
    f->clear();
    f->name = arguments["name"].toString();
    f->enableFilter = true;

    if (typeName == "negative")
        f->type = QDltFilter::negative;
    else if (typeName == "marker")
        f->type = QDltFilter::marker;
    else
        f->type = QDltFilter::positive;

    if (!arguments["ecuid"].toString().isEmpty()) {
        f->ecuid = arguments["ecuid"].toString();
        f->enableEcuid = true;
    }
    if (!arguments["apid"].toString().isEmpty()) {
        f->apid = arguments["apid"].toString();
        f->enableApid = true;
    }
    if (!arguments["ctid"].toString().isEmpty()) {
        f->ctid = arguments["ctid"].toString();
        f->enableCtid = true;
    }
    if (!arguments["header"].toString().isEmpty()) {
        f->header = arguments["header"].toString();
        f->enableHeader = true;
    }
    if (!arguments["payload"].toString().isEmpty()) {
        f->payload = arguments["payload"].toString();
        f->enablePayload = true;
    }
    f->compileRegexps();

    QDltFilterList filterList = dltFile->getFilterList();
    filterList.addFilter(f);
    dltFile->setFilterList(filterList);
    dltFile->createIndexFilter();

    const int newIndex = dltFile->getFilterList().filters.size() - 1;
    response["result"] =
        makeTextResult(QString("Filter added at index %1. Filtered view now contains %2 messages.")
                           .arg(newIndex)
                           .arg(dltFile->sizeFilter()));
}

void DltMcpPlugin::handleRemoveFilter(const QString& clientId, const QJsonObject& message,
                                      const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    QString filterMutationError;
    if (!canMutateFiltersSafely(dltFile, filterMutationMessageLimit, filterMutationError)) {
        response["error"] = makeJsonRpcError(JsonRpcError::InternalError, filterMutationError);
        return;
    }

    const int idx = arguments["index"].toInt(-1);
    QDltFilterList filterList = dltFile->getFilterList();
    if (!ensureIndexInRange(idx, filterList.filters.size(), "Filter", response))
        return;

    delete filterList.filters.takeAt(idx);
    dltFile->setFilterList(filterList);
    dltFile->createIndexFilter();
    response["result"] =
        makeTextResult(QString("Filter %1 removed. Filtered view now contains %2 messages.")
                           .arg(idx)
                           .arg(dltFile->sizeFilter()));
}

void DltMcpPlugin::handleSetFilterEnabled(const QString& clientId, const QJsonObject& message,
                                          const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    QString filterMutationError;
    if (!canMutateFiltersSafely(dltFile, filterMutationMessageLimit, filterMutationError)) {
        response["error"] = makeJsonRpcError(JsonRpcError::InternalError, filterMutationError);
        return;
    }

    const int idx = arguments["index"].toInt(-1);
    const bool enabled = arguments["enabled"].toBool(true);
    QDltFilterList filterList = dltFile->getFilterList();
    if (!ensureIndexInRange(idx, filterList.filters.size(), "Filter", response))
        return;

    filterList.filters.at(idx)->enableFilter = enabled;
    dltFile->setFilterList(filterList);
    dltFile->createIndexFilter();
    response["result"] =
        makeTextResult(QString("Filter %1 %2. Filtered view now contains %3 messages.")
                           .arg(idx)
                           .arg(enabled ? "enabled" : "disabled")
                           .arg(dltFile->sizeFilter()));
}

void DltMcpPlugin::handleConnectEcu(const QString& clientId, const QJsonObject& message,
                                    const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    if (!ensureDltControl(response))
        return;
    const int index = arguments["index"].toInt(-1);
    if (!ensureIndexInRange(index, configuredConnections.size(), "Connection", response))
        return;

    dltControl->connectEcu(index);
    response["result"] = makeTextResult(QString("Connect requested for ECU %1: %2")
                                            .arg(index)
                                            .arg(configuredConnections.at(index)));
}

void DltMcpPlugin::handleDisconnectEcu(const QString& clientId, const QJsonObject& message,
                                       const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    if (!ensureDltControl(response))
        return;
    const int index = arguments["index"].toInt(-1);
    if (!ensureIndexInRange(index, configuredConnections.size(), "Connection", response))
        return;

    dltControl->disconnectEcu(index);
    response["result"] = makeTextResult(QString("Disconnect requested for ECU %1: %2")
                                            .arg(index)
                                            .arg(configuredConnections.at(index)));
}

void DltMcpPlugin::handleConnectAllEcus(const QString& clientId, const QJsonObject& message,
                                        const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    Q_UNUSED(arguments);
    if (!ensureDltControl(response))
        return;
    dltControl->connectAllEcu();
    response["result"] = makeTextResult("Connect requested for all configured ECUs. DLT Viewer "
                                        "will start receiving messages from them.");
}

void DltMcpPlugin::handleDisconnectAllEcus(const QString& clientId, const QJsonObject& message,
                                           const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    Q_UNUSED(arguments);
    if (!ensureDltControl(response))
        return;
    dltControl->disconnectAllEcu();
    response["result"] = makeTextResult("Disconnect requested for all configured ECUs. DLT Viewer "
                                        "will stop receiving messages from them.");
}

void DltMcpPlugin::handleNavigateToMessage(const QString& clientId, const QJsonObject& message,
                                           const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    const QJsonValue indexValue = arguments.value("index");
    int index = -1;
    bool indexOk = false;

    if (indexValue.isDouble()) {
        const double numericIndex = indexValue.toDouble();
        const double floored = std::floor(numericIndex);
        if (numericIndex == floored &&
            numericIndex >= static_cast<double>(std::numeric_limits<int>::min()) &&
            numericIndex <= static_cast<double>(std::numeric_limits<int>::max())) {
            index = static_cast<int>(numericIndex);
            indexOk = true;
        }
    } else if (indexValue.isString()) {
        index = indexValue.toString().toInt(&indexOk);
    }

    if (!indexOk) {
        response["error"] =
            makeJsonRpcError(JsonRpcError::InvalidParams, "Invalid index. Expected an integer.");
        return;
    }
    if (!ensureDltControl(response))
        return;
    if (!dltFile) {
        response["error"] =
            makeJsonRpcError(JsonRpcError::InternalError, "No DLT file is open in DLT Viewer");
        return;
    }
    if (!ensureIndexInRange(index, dltFile->size(), "Global message", response))
        return;

    // MainWindow::jumpToMsgSignal resolves this global index against current filter state.
    emit dltControl->jumpToMsgSignal(index);
    response["result"] =
        makeTextResult(QString("Navigated DLT Viewer to global message index %1").arg(index));
}

void DltMcpPlugin::handleClearLogs(const QString& clientId, const QJsonObject& message,
                                   const QJsonObject& arguments, QJsonObject& response) {
    Q_UNUSED(clientId);
    Q_UNUSED(message);
    Q_UNUSED(arguments);
    if (!ensureDltControl(response))
        return;
    dltControl->clearFile();
    response["result"] = makeTextResult("Logs cleared from DLT Viewer trace view.");
}
