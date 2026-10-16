#include "dltmcpplugin_tools_common.h"

QJsonObject makeJsonRpcError(int code, const QString& message) {
    return QJsonObject{{"code", code}, {"message", message}};
}

bool compileSearchPatternRegex(const QString& pattern, QRegularExpression& regex,
                               QString& errorMessage) {
    if (pattern.trimmed().isEmpty()) {
        regex = QRegularExpression();
        errorMessage.clear();
        return true;
    }

    regex = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
    if (regex.isValid()) {
        errorMessage.clear();
        return true;
    }

    errorMessage = QString("Invalid regular expression in pattern: %1").arg(regex.errorString());
    return false;
}

bool matchesSearchPattern(const QString& payload, const QString& pattern,
                          const QRegularExpression& regex) {
    if (pattern.trimmed().isEmpty())
        return true;

    return regex.match(payload).hasMatch();
}

QJsonObject buildMessageObject(const QDltMsg& msg, int requestedIndex, bool useFilter) {
    QJsonObject msgObj;
    const qint64 rawMonotonicTimestamp = monotonicTimestampValue(msg);
    const QString wallClockTime = msg.getTimeString();
    msgObj[useFilter ? "filtered_index" : "index"] = requestedIndex;
    msgObj["global_index"] = msg.getIndex();
    msgObj["time"] = wallClockTime;
    msgObj["wall_clock_time"] = wallClockTime;
    // Numeric timestamp fields: use JSON numbers instead of strings for programmatic parsing
    const double timestampSeconds = static_cast<double>(rawMonotonicTimestamp) / 10000.0;
    msgObj["timestamp"] = timestampSeconds;
    msgObj["timestamp_raw"] = static_cast<double>(rawMonotonicTimestamp);
    msgObj["timestamp_raw_unit"] = "0.1ms";
    msgObj["timestamp_unit"] = "s_since_boot";
    msgObj["session_id"] = static_cast<double>(msg.getSessionid());
    msgObj["count"] = static_cast<int>(msg.getMessageCounter());
    msgObj["ecuid"] = msg.getEcuid();
    msgObj["apid"] = msg.getApid();
    msgObj["ctid"] = msg.getCtid();
    msgObj["type"] = msg.getTypeString();
    msgObj["subtype"] = msg.getSubtypeString();
    msgObj["mode"] = msg.getModeString();
    msgObj["payload"] = msg.toStringPayload();
    return msgObj;
}

qint64 monotonicTimestampValue(const QDltMsg& msg) {
    return static_cast<qint64>(msg.getTimestamp());
}

QString monotonicTimestampSecondsString(const QDltMsg& msg) {
    return monotonicTimestampSecondsString(monotonicTimestampValue(msg));
}

QString monotonicTimestampSecondsString(qint64 rawTimestamp) {
    return QString::number(static_cast<double>(rawTimestamp) / 10000.0, 'f', 4);
}

bool readWallClockDateTime(const QDltMsg& msg, QDateTime& dateTime) {
    if (msg.getTime() == 0 && msg.getMicroseconds() == 0)
        return false;

    dateTime =
        QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(msg.getTime()) * 1000ll +
                                           (static_cast<qint64>(msg.getMicroseconds()) / 1000ll),
                                       Qt::LocalTime);
    return dateTime.isValid();
}

qint64 timeOfDayDistanceMs(const QTime& lhs, const QTime& rhs) {
    static const qint64 dayMs = 24ll * 60ll * 60ll * 1000ll;
    const qint64 lhsMs = QTime(0, 0).msecsTo(lhs);
    const qint64 rhsMs = QTime(0, 0).msecsTo(rhs);
    const qint64 directDistance = qAbs(lhsMs - rhsMs);
    return qMin(directDistance, dayMs - directDistance);
}
