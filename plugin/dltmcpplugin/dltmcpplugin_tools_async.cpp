#include "dltmcpplugin.h"
#include "dltmcpplugin_tools_common.h"

void DltMcpPlugin::cancelAllActiveJobs(const QString& reasonTemplate) {
    if (findMessageByTimestampJob.env.active)
        failJob(findMessageByTimestampJob, reasonTemplate.arg("find_message_by_timestamp"));
    if (searchLogsAroundIndexJob.env.active)
        failJob(searchLogsAroundIndexJob, reasonTemplate.arg("search_logs_around_index"));
    if (searchLogsJob.env.active)
        failJob(searchLogsJob, reasonTemplate.arg("search_logs"));
    if (getErrorMessagesJob.env.active)
        failJob(getErrorMessagesJob, reasonTemplate.arg("get_error_messages"));
}

void DltMcpPlugin::finishFindMessageByTimestampJob() {
    if (!findMessageByTimestampJob.env.active)
        return;

    if (!findMessageByTimestampJob.hasBest) {
        failJob(findMessageByTimestampJob,
                "No messages with usable wall-clock timestamps were found");
        return;
    }

    if (messageDecoder)
        messageDecoder->decodeMsg(findMessageByTimestampJob.matchedMsg, 0);

    QJsonObject resultObject = buildMessageObject(findMessageByTimestampJob.matchedMsg,
                                                  findMessageByTimestampJob.bestIndex,
                                                  findMessageByTimestampJob.env.useFilter);
    resultObject["requested_timestamp"] = findMessageByTimestampJob.timestampText;
    resultObject["input_kind"] = findMessageByTimestampJob.timeOnly ? "time_only" : "datetime";
    resultObject["delta_ms"] = QString::number(findMessageByTimestampJob.bestDistanceMs);
    resultObject["matched_datetime"] =
        findMessageByTimestampJob.matchedDateTime.toString("yyyy/MM/dd HH:mm:ss.zzz");

    const QJsonDocument doc(resultObject);
    finishJobWithText(findMessageByTimestampJob,
                      QString("Nearest message for timestamp %1 (%2, %3):\n%4")
                          .arg(findMessageByTimestampJob.timestampText)
                          .arg(findMessageByTimestampJob.timeOnly ? "time-only" : "datetime")
                          .arg(findMessageByTimestampJob.env.useFilter ? "filtered" : "unfiltered")
                          .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented))));
}

void DltMcpPlugin::processFindMessageByTimestampChunk() {
    if (!findMessageByTimestampJob.env.active)
        return;

    if (!dltFile) {
        failJob(findMessageByTimestampJob, "No DLT file is open in DLT Viewer");
        return;
    }

    const int chunkEnd =
        qMin(findMessageByTimestampJob.env.nextIndex + findMessageByTimestampJob.env.chunkSize,
             findMessageByTimestampJob.env.total);
    for (int i = findMessageByTimestampJob.env.nextIndex; i < chunkEnd; ++i) {
        QDltMsg msg;
        QDateTime messageDateTime;
        if (!readMessageAt(i, findMessageByTimestampJob.env.useFilter, msg) ||
            !readWallClockDateTime(msg, messageDateTime))
            continue;

        qint64 distanceMs = 0;
        if (findMessageByTimestampJob.timeOnly)
            distanceMs = timeOfDayDistanceMs(messageDateTime.time(),
                                             findMessageByTimestampJob.requestedTimeOfDay);
        else
            distanceMs = qAbs(messageDateTime.msecsTo(findMessageByTimestampJob.requestedDateTime));

        if (!findMessageByTimestampJob.hasBest ||
            distanceMs < findMessageByTimestampJob.bestDistanceMs) {
            findMessageByTimestampJob.bestIndex = i;
            findMessageByTimestampJob.bestDistanceMs = distanceMs;
            findMessageByTimestampJob.matchedDateTime = messageDateTime;
            findMessageByTimestampJob.matchedMsg = msg;
            findMessageByTimestampJob.hasBest = true;
        }
    }

    findMessageByTimestampJob.env.nextIndex = chunkEnd;

    if (findMessageByTimestampJob.env.nextIndex >= findMessageByTimestampJob.env.total) {
        finishFindMessageByTimestampJob();
        return;
    }

    scheduleChunk(findMessageByTimestampJob, &DltMcpPlugin::processFindMessageByTimestampChunk);
}

void DltMcpPlugin::finishSearchLogsAroundIndexJob() {
    if (!searchLogsAroundIndexJob.env.active)
        return;

    if (!searchLogsAroundIndexJob.hasBest) {
        failJob(searchLogsAroundIndexJob,
                "No anchor message was available for the around-index search");
        return;
    }

    QJsonObject resultObject;
    resultObject["anchor_index"] = searchLogsAroundIndexJob.bestIndex;
    resultObject["pattern"] = searchLogsAroundIndexJob.pattern;
    resultObject["scope"] = searchLogsAroundIndexJob.env.useFilter ? "filtered" : "unfiltered";
    resultObject["window_before_ms"] = QString::number(searchLogsAroundIndexJob.windowBeforeMs);
    resultObject["window_after_ms"] = QString::number(searchLogsAroundIndexJob.windowAfterMs);
    resultObject["anchor_datetime"] =
        searchLogsAroundIndexJob.matchedDateTime.toString("yyyy/MM/dd HH:mm:ss.zzz");
    resultObject["anchor_timestamp"] =
        monotonicTimestampSecondsString(searchLogsAroundIndexJob.anchorMonotonic);
    resultObject["anchor_timestamp_raw"] =
        QString::number(searchLogsAroundIndexJob.anchorMonotonic);
    resultObject["anchor_timestamp_raw_unit"] = "0.1ms";
    resultObject["anchor_timestamp_unit"] = "s_since_boot";
    resultObject["searched_start_index"] = searchLogsAroundIndexJob.startIndex;
    resultObject["searched_end_index"] = searchLogsAroundIndexJob.endIndex;
    resultObject["scanned_messages"] = searchLogsAroundIndexJob.scannedMessages;
    resultObject["returned_results"] = searchLogsAroundIndexJob.results.size();
    resultObject["max_results"] = searchLogsAroundIndexJob.maxResults;
    resultObject["matches"] = searchLogsAroundIndexJob.results;

    const QJsonDocument doc(resultObject);
    finishJobWithText(searchLogsAroundIndexJob,
                      QString("Found %1 matches around anchor index %2 (%3):\n%4")
                          .arg(searchLogsAroundIndexJob.results.size())
                          .arg(searchLogsAroundIndexJob.bestIndex)
                          .arg(searchLogsAroundIndexJob.env.useFilter ? "filtered" : "unfiltered")
                          .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented))));
}

void DltMcpPlugin::processSearchLogsAroundIndexChunk() {
    if (!searchLogsAroundIndexJob.env.active)
        return;

    if (!dltFile) {
        failJob(searchLogsAroundIndexJob, "No DLT file is open in DLT Viewer");
        return;
    }

    QRegularExpression patternRegex;
    QString regexErrorMessage;
    if (!compileSearchPatternRegex(searchLogsAroundIndexJob.pattern, patternRegex,
                                   regexErrorMessage)) {
        failJob(searchLogsAroundIndexJob, regexErrorMessage);
        return;
    }

    if (searchLogsAroundIndexJob.phase == SearchLogsAroundIndexJob::ExpandingStart) {
        int processed = 0;
        while (searchLogsAroundIndexJob.env.nextIndex >= 0 &&
               processed < searchLogsAroundIndexJob.env.chunkSize) {
            QDltMsg msg;
            if (!readMessageAt(searchLogsAroundIndexJob.env.nextIndex,
                               searchLogsAroundIndexJob.env.useFilter, msg)) {
                searchLogsAroundIndexJob.startIndex = searchLogsAroundIndexJob.env.nextIndex;
                --searchLogsAroundIndexJob.env.nextIndex;
                ++processed;
                continue;
            }

            const qint64 value = monotonicTimestampValue(msg);
            if (searchLogsAroundIndexJob.likelySorted &&
                value < searchLogsAroundIndexJob.lowerBound)
                break;
            if (value >= searchLogsAroundIndexJob.lowerBound) {
                searchLogsAroundIndexJob.startIndex = searchLogsAroundIndexJob.env.nextIndex;
                --searchLogsAroundIndexJob.env.nextIndex;
                ++processed;
                continue;
            }
            break;
        }

        if (searchLogsAroundIndexJob.env.nextIndex >= 0) {
            QDltMsg msg;
            if (readMessageAt(searchLogsAroundIndexJob.env.nextIndex,
                              searchLogsAroundIndexJob.env.useFilter, msg)) {
                const qint64 value = monotonicTimestampValue(msg);
                if ((!searchLogsAroundIndexJob.likelySorted &&
                     value >= searchLogsAroundIndexJob.lowerBound) ||
                    (searchLogsAroundIndexJob.likelySorted &&
                     value >= searchLogsAroundIndexJob.lowerBound &&
                     processed >= searchLogsAroundIndexJob.env.chunkSize)) {
                    scheduleChunk(searchLogsAroundIndexJob,
                                  &DltMcpPlugin::processSearchLogsAroundIndexChunk);
                    return;
                }
            } else if (processed >= searchLogsAroundIndexJob.env.chunkSize) {
                scheduleChunk(searchLogsAroundIndexJob,
                              &DltMcpPlugin::processSearchLogsAroundIndexChunk);
                return;
            }
        }

        searchLogsAroundIndexJob.phase = SearchLogsAroundIndexJob::ExpandingEnd;
        searchLogsAroundIndexJob.env.nextIndex = searchLogsAroundIndexJob.endIndex + 1;
        scheduleChunk(searchLogsAroundIndexJob, &DltMcpPlugin::processSearchLogsAroundIndexChunk);
        return;
    }

    if (searchLogsAroundIndexJob.phase == SearchLogsAroundIndexJob::ExpandingEnd) {
        int processed = 0;
        while (searchLogsAroundIndexJob.env.nextIndex < searchLogsAroundIndexJob.env.total &&
               processed < searchLogsAroundIndexJob.env.chunkSize) {
            QDltMsg msg;
            if (!readMessageAt(searchLogsAroundIndexJob.env.nextIndex,
                               searchLogsAroundIndexJob.env.useFilter, msg)) {
                searchLogsAroundIndexJob.endIndex = searchLogsAroundIndexJob.env.nextIndex;
                ++searchLogsAroundIndexJob.env.nextIndex;
                ++processed;
                continue;
            }

            const qint64 value = monotonicTimestampValue(msg);
            if (searchLogsAroundIndexJob.likelySorted &&
                value > searchLogsAroundIndexJob.upperBound)
                break;
            if (value <= searchLogsAroundIndexJob.upperBound) {
                searchLogsAroundIndexJob.endIndex = searchLogsAroundIndexJob.env.nextIndex;
                ++searchLogsAroundIndexJob.env.nextIndex;
                ++processed;
                continue;
            }
            break;
        }

        if (searchLogsAroundIndexJob.env.nextIndex < searchLogsAroundIndexJob.env.total) {
            QDltMsg msg;
            if (readMessageAt(searchLogsAroundIndexJob.env.nextIndex,
                              searchLogsAroundIndexJob.env.useFilter, msg)) {
                const qint64 value = monotonicTimestampValue(msg);
                if ((!searchLogsAroundIndexJob.likelySorted &&
                     value <= searchLogsAroundIndexJob.upperBound) ||
                    (searchLogsAroundIndexJob.likelySorted &&
                     value <= searchLogsAroundIndexJob.upperBound &&
                     processed >= searchLogsAroundIndexJob.env.chunkSize)) {
                    scheduleChunk(searchLogsAroundIndexJob,
                                  &DltMcpPlugin::processSearchLogsAroundIndexChunk);
                    return;
                }
            } else if (processed >= searchLogsAroundIndexJob.env.chunkSize) {
                scheduleChunk(searchLogsAroundIndexJob,
                              &DltMcpPlugin::processSearchLogsAroundIndexChunk);
                return;
            }
        }

        searchLogsAroundIndexJob.phase = SearchLogsAroundIndexJob::CollectingMatches;
        searchLogsAroundIndexJob.env.nextIndex = searchLogsAroundIndexJob.startIndex;
        scheduleChunk(searchLogsAroundIndexJob, &DltMcpPlugin::processSearchLogsAroundIndexChunk);
        return;
    }

    const int chunkEnd =
        qMin(searchLogsAroundIndexJob.env.nextIndex + searchLogsAroundIndexJob.env.chunkSize,
             searchLogsAroundIndexJob.endIndex + 1);
    for (int i = searchLogsAroundIndexJob.env.nextIndex;
         i < chunkEnd &&
         searchLogsAroundIndexJob.results.size() < searchLogsAroundIndexJob.maxResults;
         ++i) {
        QDltMsg msg;
        if (!readMessageAt(i, searchLogsAroundIndexJob.env.useFilter, msg))
            continue;

        const qint64 currentMonotonic = monotonicTimestampValue(msg);
        if (currentMonotonic < searchLogsAroundIndexJob.lowerBound ||
            currentMonotonic > searchLogsAroundIndexJob.upperBound)
            continue;

        ++searchLogsAroundIndexJob.scannedMessages;
        if (messageDecoder)
            messageDecoder->decodeMsg(msg, 0);
        const QString payload = msg.toStringPayload();
        if (!matchesSearchPattern(payload, searchLogsAroundIndexJob.pattern, patternRegex))
            continue;
        if (!searchLogsAroundIndexJob.ecuid.isEmpty() &&
            msg.getEcuid().compare(searchLogsAroundIndexJob.ecuid, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsAroundIndexJob.apid.isEmpty() &&
            msg.getApid().compare(searchLogsAroundIndexJob.apid, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsAroundIndexJob.ctid.isEmpty() &&
            msg.getCtid().compare(searchLogsAroundIndexJob.ctid, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsAroundIndexJob.type.isEmpty() &&
            msg.getTypeString().compare(searchLogsAroundIndexJob.type, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsAroundIndexJob.subtype.isEmpty() &&
            msg.getSubtypeString().compare(searchLogsAroundIndexJob.subtype, Qt::CaseInsensitive) !=
                0)
            continue;
        if (!searchLogsAroundIndexJob.mode.isEmpty() &&
            msg.getModeString().compare(searchLogsAroundIndexJob.mode, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsAroundIndexJob.sessionId.isEmpty() &&
            QString::number(msg.getSessionid())
                    .compare(searchLogsAroundIndexJob.sessionId, Qt::CaseInsensitive) != 0)
            continue;

        QJsonObject msgObj = buildMessageObject(msg, i, searchLogsAroundIndexJob.env.useFilter);
        msgObj["delta_timestamp_ms"] =
            QString::number(qAbs(currentMonotonic - searchLogsAroundIndexJob.anchorMonotonic) / 10);
        searchLogsAroundIndexJob.results.append(msgObj);
    }

    searchLogsAroundIndexJob.env.nextIndex = chunkEnd;
    if (searchLogsAroundIndexJob.env.nextIndex > searchLogsAroundIndexJob.endIndex ||
        searchLogsAroundIndexJob.results.size() >= searchLogsAroundIndexJob.maxResults) {
        finishSearchLogsAroundIndexJob();
        return;
    }

    scheduleChunk(searchLogsAroundIndexJob, &DltMcpPlugin::processSearchLogsAroundIndexChunk);
}

void DltMcpPlugin::finishSearchLogsJob() {
    if (!searchLogsJob.env.active)
        return;

    QJsonObject resultObject;
    resultObject["pattern"] = searchLogsJob.pattern;
    resultObject["scope"] = searchLogsJob.env.useFilter ? "filtered" : "unfiltered";
    resultObject["start_index"] = searchLogsJob.startIndex;
    resultObject["searched_until_index"] = (searchLogsJob.env.nextIndex > searchLogsJob.startIndex)
                                               ? (searchLogsJob.env.nextIndex - 1)
                                               : searchLogsJob.startIndex;
    resultObject["scanned_messages"] = searchLogsJob.scannedMessages;
    resultObject["returned_results"] = searchLogsJob.results.size();
    resultObject["max_results"] = searchLogsJob.maxResults;
    resultObject["completed"] = searchLogsJob.completed;
    resultObject["next_index"] = searchLogsJob.completed ? -1 : searchLogsJob.env.nextIndex;
    resultObject["scan_limit"] = searchLogsJob.scanLimit;
    resultObject["total_messages"] = searchLogsJob.env.total;
    resultObject["matches"] = searchLogsJob.results;

    const QJsonDocument doc(resultObject);
    finishJobWithText(searchLogsJob,
                      QString("Found %1 matching messages after scanning %2 %3 messages starting "
                              "at index %4. Completed: %5. Next index: %6\n%7")
                          .arg(searchLogsJob.results.size())
                          .arg(searchLogsJob.scannedMessages)
                          .arg(searchLogsJob.env.useFilter ? "filtered" : "unfiltered")
                          .arg(searchLogsJob.startIndex)
                          .arg(searchLogsJob.completed ? "yes" : "no")
                          .arg(searchLogsJob.completed ? -1 : searchLogsJob.env.nextIndex)
                          .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented))));
}

void DltMcpPlugin::finishGetErrorMessagesJob() {
    if (!getErrorMessagesJob.env.active)
        return;

    const QJsonDocument doc(getErrorMessagesJob.results);
    finishJobWithText(getErrorMessagesJob,
                      QString("Found %1 error/warning messages (searched %2 %3 messages):\n%4")
                          .arg(getErrorMessagesJob.results.size())
                          .arg(getErrorMessagesJob.env.total)
                          .arg(getErrorMessagesJob.env.useFilter ? "filtered" : "unfiltered")
                          .arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented))));
}

void DltMcpPlugin::processSearchLogsChunk() {
    if (!searchLogsJob.env.active)
        return;

    if (!dltFile) {
        failJob(searchLogsJob, "No DLT file is open in DLT Viewer");
        return;
    }

    QRegularExpression patternRegex;
    QString regexErrorMessage;
    if (!compileSearchPatternRegex(searchLogsJob.pattern, patternRegex, regexErrorMessage)) {
        failJob(searchLogsJob, regexErrorMessage);
        return;
    }

    const int chunkEnd = searchLogsJob.env.nextIndex +
                         qMin(searchLogsJob.env.chunkSize,
                              searchLogsJob.scanLimit - searchLogsJob.env.nextIndex);
    int i = searchLogsJob.env.nextIndex;
    for (; i < chunkEnd && searchLogsJob.results.size() < searchLogsJob.maxResults; ++i) {
        QDltMsg msg;
        if (!readMessageAt(i, searchLogsJob.env.useFilter, msg))
            continue;
        if (!searchLogsJob.ecuid.isEmpty() &&
            msg.getEcuid().compare(searchLogsJob.ecuid, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsJob.apid.isEmpty() &&
            msg.getApid().compare(searchLogsJob.apid, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsJob.ctid.isEmpty() &&
            msg.getCtid().compare(searchLogsJob.ctid, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsJob.type.isEmpty() &&
            msg.getTypeString().compare(searchLogsJob.type, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsJob.subtype.isEmpty() &&
            msg.getSubtypeString().compare(searchLogsJob.subtype, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsJob.mode.isEmpty() &&
            msg.getModeString().compare(searchLogsJob.mode, Qt::CaseInsensitive) != 0)
            continue;
        if (!searchLogsJob.sessionId.isEmpty() &&
            QString::number(msg.getSessionid())
                    .compare(searchLogsJob.sessionId, Qt::CaseInsensitive) != 0)
            continue;

        ++searchLogsJob.scannedMessages;

        if (messageDecoder)
            messageDecoder->decodeMsg(msg, 0);

        const QString payload = msg.toStringPayload();
        if (!matchesSearchPattern(payload, searchLogsJob.pattern, patternRegex))
            continue;

        QJsonObject msgObj = buildMessageObject(msg, i, searchLogsJob.env.useFilter);
        searchLogsJob.results.append(msgObj);
    }

    searchLogsJob.env.nextIndex = i;
    searchLogsJob.completed = (i >= searchLogsJob.env.total);

    if (searchLogsJob.completed || i >= searchLogsJob.scanLimit ||
        searchLogsJob.results.size() >= searchLogsJob.maxResults) {
        finishSearchLogsJob();
        return;
    }

    scheduleChunk(searchLogsJob, &DltMcpPlugin::processSearchLogsChunk);
}

void DltMcpPlugin::processGetErrorMessagesChunk() {
    if (!getErrorMessagesJob.env.active)
        return;

    if (!dltFile) {
        failJob(getErrorMessagesJob, "No DLT file is open in DLT Viewer");
        return;
    }

    const int chunkEnd = qMin(getErrorMessagesJob.env.nextIndex + getErrorMessagesJob.env.chunkSize,
                              getErrorMessagesJob.env.total);
    for (int i = getErrorMessagesJob.env.nextIndex;
         i < chunkEnd && getErrorMessagesJob.results.size() < getErrorMessagesJob.maxResults; ++i) {
        QDltMsg msg;
        if (!readMessageAt(i, getErrorMessagesJob.env.useFilter, msg))
            continue;
        if (messageDecoder)
            messageDecoder->decodeMsg(msg, 0);

        if (msg.getType() != QDltMsg::DltTypeLog)
            continue;

        const int subtype = msg.getSubtype();
        if (subtype == QDltMsg::DltLogFatal || subtype == QDltMsg::DltLogError ||
            subtype == QDltMsg::DltLogWarn) {
            QJsonObject msgObj = buildMessageObject(msg, i, getErrorMessagesJob.env.useFilter);
            getErrorMessagesJob.results.append(msgObj);
        }
    }

    getErrorMessagesJob.env.nextIndex = chunkEnd;

    if (getErrorMessagesJob.env.nextIndex >= getErrorMessagesJob.env.total ||
        getErrorMessagesJob.results.size() >= getErrorMessagesJob.maxResults) {
        finishGetErrorMessagesJob();
        return;
    }

    scheduleChunk(getErrorMessagesJob, &DltMcpPlugin::processGetErrorMessagesChunk);
}
