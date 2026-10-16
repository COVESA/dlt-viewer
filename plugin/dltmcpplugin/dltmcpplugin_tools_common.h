#ifndef DLTMCPPLUGIN_TOOLS_COMMON_H
#define DLTMCPPLUGIN_TOOLS_COMMON_H

#include <QDateTime>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTime>

#include "qdltmsg.h"

//! JSON-RPC 2.0 error codes we actually use in the MCP plugin.
//! Named constants remove the need to remember which numeric code applies
//! to client-side mistakes vs server-side faults when returning errors.
namespace JsonRpcError {
//! -32602 — request arguments are invalid, missing, or out of range.
//! Use for validation failures on `arguments` passed by the MCP client.
constexpr int InvalidParams = -32602;

//! -32603 — the server reached a state it cannot recover from, or a
//! required internal dependency (e.g. QDltControl) is unavailable.
constexpr int InternalError = -32603;
} // namespace JsonRpcError

//! Builds a JSON-RPC 2.0 error object of the form
//!   { "code": <code>, "message": <message> }
//! Use together with `response["error"] = makeJsonRpcError(...)` in tool handlers.
QJsonObject makeJsonRpcError(int code, const QString& message);

bool readWallClockDateTime(const QDltMsg& msg, QDateTime& dateTime);
qint64 timeOfDayDistanceMs(const QTime& lhs, const QTime& rhs);
qint64 monotonicTimestampValue(const QDltMsg& msg);
QString monotonicTimestampSecondsString(const QDltMsg& msg);
QString monotonicTimestampSecondsString(qint64 rawTimestamp);
bool compileSearchPatternRegex(const QString& pattern, QRegularExpression& regex,
                               QString& errorMessage);
bool matchesSearchPattern(const QString& payload, const QString& pattern,
                          const QRegularExpression& regex);
QJsonObject buildMessageObject(const QDltMsg& msg, int requestedIndex, bool useFilter);

#endif // DLTMCPPLUGIN_TOOLS_COMMON_H
