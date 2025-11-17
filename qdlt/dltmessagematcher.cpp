#include "dltmessagematcher.h"

#include <qdltmsg.h>

DltMessageMatcher::DltMessageMatcher() {}

bool DltMessageMatcher::match(const QDltMsg &msg, const Pattern& pattern) const
{
    if (!matchAppId(msg.getApid()) || !matchCtxId(msg.getCtid()))
        return false;

    if (!matchTimestampRange(msg.getTimestamp())) {
        return false;
    }

    qint64 timestampMSecsSinceEpoch = msg.getTime() * 1000 + msg.getMicroseconds() / 1000;
    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(timestampMSecsSinceEpoch);
    if (!matchTimeRange(timestamp)) {
        return false;
    }

    bool matchFound = false;
    if (m_headerSearchEnabled) {
        auto header = msg.toStringHeader();
        if (m_messageIdFormat)
            header += ' ' + QString::asprintf(m_messageIdFormat->toUtf8(), msg.getMessageId());
        if (std::holds_alternative<QRegularExpression>(pattern)) {
            matchFound = header.contains(std::get<QRegularExpression>(pattern));
        } else {
            const auto& searchText = std::get<QString>(pattern);
            matchFound = searchText.isEmpty() || header.contains(searchText, m_caseSensitivity);
        }
    }

    if (matchFound)
        return true;

    if (m_payloadSearchEnabled) {
        const auto payload = msg.toStringPayload();
        if (std::holds_alternative<QRegularExpression>(pattern)) {
            matchFound = payload.contains(std::get<QRegularExpression>(pattern));
        } else {
            const auto& searchText = std::get<QString>(pattern);
            matchFound = payload.contains(searchText, m_caseSensitivity);
        }
    }

    return matchFound;
}

bool DltMessageMatcher::matchAppId(const QString& appId) const
{
    return m_appId.isEmpty() || appId.compare(m_appId, m_caseSensitivity) == 0;
}

bool DltMessageMatcher::matchCtxId(const QString& ctxId) const
{
    return m_ctxId.isEmpty() || ctxId.compare(m_ctxId, m_caseSensitivity) == 0;
}

bool DltMessageMatcher::matchTimestampRange(unsigned int ts) const
{
    if (!m_timestampRange)
        return true;

    // timestamp is displayed as floating number in UI and hence user provides timestamp ranges as floating numbers too
    // in DltMsg stores timestamp as integer which is transformed to UI display floating number by QltMgs::toStringHeader
    // method more or less as follows
    const auto uiTs = static_cast<double>(ts) / 10'000;

    return (m_timestampRange->start <= uiTs) && (uiTs <= m_timestampRange->end);
}

bool DltMessageMatcher::matchTimeRange(const QDateTime& dt) const
{
    if (!m_timeRange)
        return true;

    return (m_timeRange->start < dt) && (dt < m_timeRange->end);
}
