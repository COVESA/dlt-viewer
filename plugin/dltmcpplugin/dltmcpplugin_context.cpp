#include "dltmcpplugin.h"
#include <QMap>
#include <algorithm>

QJsonObject DltMcpPlugin::makeTextResult(const QString& text, const QJsonObject& data) const {
    QJsonObject result;
    result["content"] = QJsonArray{{QJsonObject{{"type", "text"}, {"text", text}}}};
    if (!data.isEmpty()) {
        result["data"] = data;
    }
    return result;
}

QJsonObject DltMcpPlugin::buildContextData() const {
    const int total = dltFile ? dltFile->size() : 0;
    const int filtered = dltFile ? dltFile->sizeFilter() : 0;
    const bool filterActive = (filtered != total);
    const QString fileName = dltFile ? dltFile->getFileName() : QString();

    QDltFilterList filterList = dltFile ? dltFile->getFilterList() : QDltFilterList();
    int enabledFilters = 0;
    for (int i = 0; i < filterList.filters.size(); ++i) {
        if (filterList.filters.at(i)->enableFilter)
            ++enabledFilters;
    }

    // Sample only a bounded prefix to keep context generation responsive on large traces.
    const int sampleLimit = qMin(total, 2000);
    QMap<QString, int> typeCounts;
    QMap<QString, int> ecuCounts;
    QMap<QString, int> apidCounts;
    QMap<QString, int> ctidCounts;

    for (int i = 0; i < sampleLimit; ++i) {
        QDltMsg msg;
        if (!dltFile || !dltFile->getMsg(i, msg))
            continue;

        if (messageDecoder)
            messageDecoder->decodeMsg(msg, 0);

        typeCounts[messageTypeToString(msg.getType())] =
            typeCounts.value(messageTypeToString(msg.getType())) + 1;

        const QString ecuid = msg.getEcuid().trimmed();
        const QString apid = msg.getApid().trimmed();
        const QString ctid = msg.getCtid().trimmed();

        if (!ecuid.isEmpty())
            ecuCounts[ecuid] = ecuCounts.value(ecuid) + 1;
        if (!apid.isEmpty())
            apidCounts[apid] = apidCounts.value(apid) + 1;
        if (!ctid.isEmpty())
            ctidCounts[ctid] = ctidCounts.value(ctid) + 1;
    }

    // Keep a stable machine-readable schema for MCP clients.
    QJsonObject contextData;
    contextData["trace_file"] = fileName.isEmpty() ? QJsonValue::Null : QJsonValue(fileName);
    contextData["total_messages"] = total;
    contextData["filtered_messages"] = filtered;
    contextData["filter_active"] = filterActive;
    contextData["configured_filters"] = filterList.filters.size();
    contextData["enabled_filters"] = enabledFilters;

    QJsonObject messageCounts;
    for (auto it = typeCounts.constBegin(); it != typeCounts.constEnd(); ++it) {
        messageCounts[it.key()] = it.value();
    }
    contextData["message_types"] = messageCounts;

    contextData["unique_ecus"] = ecuCounts.size();
    QJsonObject topEcus;
    for (auto it = ecuCounts.constBegin(); it != ecuCounts.constEnd(); ++it) {
        topEcus[it.key()] = it.value();
    }
    contextData["ecus"] = topEcus;

    contextData["unique_apids"] = apidCounts.size();
    QJsonObject topApids;
    for (auto it = apidCounts.constBegin(); it != apidCounts.constEnd(); ++it) {
        topApids[it.key()] = it.value();
    }
    contextData["apids"] = topApids;

    contextData["unique_ctids"] = ctidCounts.size();
    QJsonObject topCtids;
    for (auto it = ctidCounts.constBegin(); it != ctidCounts.constEnd(); ++it) {
        topCtids[it.key()] = it.value();
    }
    contextData["ctids"] = topCtids;

    contextData["sample_size"] = sampleLimit;

    return contextData;
}

QString DltMcpPlugin::messageTypeToString(QDltMsg::DltTypeDef type) const {
    switch (type) {
    case QDltMsg::DltTypeLog:
        return "Log";
    case QDltMsg::DltTypeAppTrace:
        return "AppTrace";
    case QDltMsg::DltTypeNwTrace:
        return "NwTrace";
    case QDltMsg::DltTypeControl:
        return "Control";
    default:
        return "Unknown";
    }
}

QString DltMcpPlugin::buildContextText() const {
    const int total = dltFile ? dltFile->size() : 0;
    const int filtered = dltFile ? dltFile->sizeFilter() : 0;
    const bool filterActive = (filtered != total);
    const QString fileName = dltFile ? dltFile->getFileName() : QString();

    QDltFilterList filterList = dltFile ? dltFile->getFilterList() : QDltFilterList();
    int enabledFilters = 0;
    for (int i = 0; i < filterList.filters.size(); ++i) {
        if (filterList.filters.at(i)->enableFilter)
            ++enabledFilters;
    }

    // Mirror buildContextData sampling while formatting a concise human-readable summary.
    const int sampleLimit = qMin(total, 2000);
    QMap<QString, int> typeCounts;
    QMap<QString, int> ecuCounts;
    QMap<QString, int> apidCounts;
    QMap<QString, int> ctidCounts;

    for (int i = 0; i < sampleLimit; ++i) {
        QDltMsg msg;
        if (!dltFile || !dltFile->getMsg(i, msg))
            continue;

        if (messageDecoder)
            messageDecoder->decodeMsg(msg, 0);

        typeCounts[messageTypeToString(msg.getType())] =
            typeCounts.value(messageTypeToString(msg.getType())) + 1;

        const QString ecuid = msg.getEcuid().trimmed();
        const QString apid = msg.getApid().trimmed();
        const QString ctid = msg.getCtid().trimmed();

        if (!ecuid.isEmpty())
            ecuCounts[ecuid] = ecuCounts.value(ecuid) + 1;
        if (!apid.isEmpty())
            apidCounts[apid] = apidCounts.value(apid) + 1;
        if (!ctid.isEmpty())
            ctidCounts[ctid] = ctidCounts.value(ctid) + 1;
    }

    auto topKeys = [](const QMap<QString, int>& counts, int limit) -> QString {
        QList<QPair<QString, int>> pairs;
        for (QMap<QString, int>::const_iterator it = counts.constBegin(); it != counts.constEnd();
             ++it)
            pairs.append(qMakePair(it.key(), it.value()));

        std::sort(pairs.begin(), pairs.end(),
                  [](const QPair<QString, int>& lhs, const QPair<QString, int>& rhs) {
                      if (lhs.second != rhs.second)
                          return lhs.second > rhs.second;
                      return lhs.first < rhs.first;
                  });

        QStringList parts;
        const int actualLimit = qMin(limit, pairs.size());
        for (int i = 0; i < actualLimit; ++i)
            parts << QString("%1 (%2)").arg(pairs.at(i).first).arg(pairs.at(i).second);

        return parts.isEmpty() ? QString("none") : parts.join(", ");
    };

    QStringList lines;
    lines << QString("Trace: %1").arg(fileName.isEmpty() ? "<unknown>" : fileName);
    lines << QString("Total: %1 | Filtered: %2 | Active: %3")
                 .arg(total)
                 .arg(filtered)
                 .arg(filterActive ? "yes" : "no");
    lines << QString("Filters: %1 configured (%2 enabled)")
                 .arg(filterList.filters.size())
                 .arg(enabledFilters);
    lines << "";
    lines << QString("Sample (first %1 messages):").arg(sampleLimit);
    lines << QString("  Types: %1").arg(topKeys(typeCounts, 8));
    lines
        << QString("  ECUs: %1 unique | Top: %2").arg(ecuCounts.size()).arg(topKeys(ecuCounts, 5));
    lines << QString("  APIDs: %1 unique | Top: %2")
                 .arg(apidCounts.size())
                 .arg(topKeys(apidCounts, 5));
    lines << QString("  CTIDs: %1 unique | Top: %2")
                 .arg(ctidCounts.size())
                 .arg(topKeys(ctidCounts, 5));

    return lines.join("\n");
}

QString DltMcpPlugin::buildDictionaryText(const QString& term) const {
    QList<DictionaryEntry> entries = builtInDictionaryEntries();
    for (const DictionaryEntry& customEntry : customDictionaryEntries) {
        bool replaced = false;
        for (int i = 0; i < entries.size(); ++i) {
            if (entries.at(i).key == customEntry.key) {
                entries[i] = customEntry;
                replaced = true;
                break;
            }
        }
        if (!replaced)
            entries.append(customEntry);
    }

    const QString normalizedTerm = term.trimmed().toLower();
    QStringList lines;
    lines << "DLT terminology dictionary:";
    if (!customDictionaryEntries.isEmpty())
        lines
            << QString("Custom dictionary entries loaded: %1").arg(customDictionaryEntries.size());

    bool found = normalizedTerm.isEmpty();
    for (int i = 0; i < entries.size(); ++i) {
        const DictionaryEntry& entry = entries.at(i);
        bool match = normalizedTerm.isEmpty() || entry.key.contains(normalizedTerm);
        if (!match) {
            for (int j = 0; j < entry.aliases.size(); ++j) {
                const QString alias = entry.aliases.at(j);
                if (alias.contains(normalizedTerm) || normalizedTerm.contains(alias)) {
                    match = true;
                    break;
                }
            }
        }

        if (!match)
            continue;

        found = true;
        lines << "";
        lines << QString("- %1").arg(entry.text);
    }

    if (!found) {
        lines << "";
        lines << QString("No dedicated dictionary entry found for \"%1\".").arg(term);
        lines << "Try broader terms such as trace, ECUID, APID, CTID, filter, filtered view, or "
                 "log levels.";
    }

    return lines.join("\n");
}

QList<DltMcpPlugin::DictionaryEntry> DltMcpPlugin::builtInDictionaryEntries() const {
    return QList<DictionaryEntry>()
           << DictionaryEntry{"trace",
                              QStringList() << "log" << "log file" << "session" << "loaded file",
                              "DLT data currently loaded in DLT Viewer (usually .dlt file or live "
                              "capture)."}
           << DictionaryEntry{"filtered view",
                              QStringList() << "filter" << "visible messages" << "current view",
                              "Subset of messages after active DLT Viewer filters applied (what "
                              "user sees)."}
           << DictionaryEntry{"unfiltered view",
                              QStringList() << "full file" << "full trace" << "all messages",
                              "Complete underlying DLT file, ignoring active filters. Use "
                              "use_filter=false."}
           << DictionaryEntry{"ecuid", QStringList() << "ecu" << "ecu id",
                              "Electronic Control Unit ID. Identifies which ECU produced the "
                              "message."}
           << DictionaryEntry{"apid",
                              QStringList() << "application" << "application id" << "app id",
                              "Application ID within ECU. Typically identifies software component "
                              "or process."}
           << DictionaryEntry{"ctid",
                              QStringList() << "context" << "context id" << "con id" << "tc",
                              "Context ID within application (subsystem, class, module)."}
           << DictionaryEntry{"verbose mode",
                              QStringList()
                                  << "verbose" << "typed arguments" << "decoded arguments",
                              "Payload contains typed DLT arguments directly; decodable without "
                              "external schema."}
           << DictionaryEntry{"non-verbose mode",
                              QStringList() << "non verbose" << "nonverbose" << "message id based",
                              "Payload uses Message ID + external schema for decoding."}
           << DictionaryEntry{"message id", QStringList() << "msg id" << "id" << "non-verbose id",
                              "ID used by non-verbose messages to select decode schema."}
           << DictionaryEntry{"argument", QStringList() << "payload argument" << "typed value",
                              "Typed value in verbose payload (int, float, string, raw, etc.)."}
           << DictionaryEntry{"message counter", QStringList() << "mcnt" << "counter" << "sequence",
                              "Sequence counter from standard header; detects drops/ordering "
                              "issues."}
           << DictionaryEntry{"standard header", QStringList() << "std header" << "header",
                              "Core DLT header: protocol flags, message counter, ECU ID, Session "
                              "ID, timestamp."}
           << DictionaryEntry{"log level",
                              QStringList() << "levels" << "severity" << "fatal" << "error"
                                            << "warn" << "info" << "debug" << "verbose",
                              "Severity: Fatal > Error > Warn > Info > Debug > Verbose."}
           << DictionaryEntry{"log", QStringList() << "log message" << "log level",
                              "Standard DLT log entry with severity level."}
           << DictionaryEntry{"apptrace", QStringList() << "app trace",
                              "Application trace message (function entry/exit, state transitions)."}
           << DictionaryEntry{"control", QStringList() << "control message",
                              "DLT control/request/response for protocol interactions."}
           << DictionaryEntry{"control service id",
                              QStringList() << "service id" << "ctrl service" << "control sid",
                              "Identifies which DLT control operation (request/response)."}
           << DictionaryEntry{"injection", QStringList() << "inject" << "send injection",
                              "Send data/commands client-side to ECU/app via DLT control "
                              "(diagnostics/runtime control)."}
           << DictionaryEntry{"sessionid",
                              QStringList()
                                  << "session id" << "sessionid" << "pid" << "process id" << "cpid",
                              "Session ID in standard header. Often PID, but sender-dependent."}
           << DictionaryEntry{"time",
                              QStringList() << "wall-clock time" << "receive time" << "logger time",
                              "Wall-clock time when logger received message (MCP: time / "
                              "wall_clock_time)."}
           << DictionaryEntry{"timestamp",
                              QStringList() << "monotonic time" << "uptime" << "ecu time",
                              "ECU monotonic time since boot (not wall-clock). MCP: timestamp (s) "
                              "= timestamp_raw (0.1 ms) / 10000."}
           << DictionaryEntry{"filter types",
                              QStringList()
                                  << "positive filter" << "negative filter" << "marker filter",
                              "Positive: keep matches. Negative: remove matches. Marker: highlight "
                              "matches."}
           << DictionaryEntry{
                  "dltv2", QStringList() << "dlt v2" << "tags" << "privacy" << "segmentation",
                  "Protocol extensions: tags, privacy, segmentation, source metadata (file/line)."};
}
