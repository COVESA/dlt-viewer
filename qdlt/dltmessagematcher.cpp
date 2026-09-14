#include "dltmessagematcher.h"

#include <qdltmsg.h>

DltMessageMatcher::DltMessageMatcher() {}

bool DltMessageMatcher::match(const QDltMsg &msg, const char *searchText) const
{
    return matchText(msg, QString::fromUtf8(searchText ? searchText : ""));
}

bool DltMessageMatcher::match(const QDltMsg &msg, const QString &searchText) const
{
    return matchText(msg, searchText);
}

bool DltMessageMatcher::match(const QDltMsg &msg, const Pattern& pattern) const
{
    if (!passesPreFilters(msg))
        return false;

    if (std::holds_alternative<QRegularExpression>(pattern))
        return matchHeaderAndPayload(msg, std::get<QRegularExpression>(pattern));

    return matchHeaderAndPayload(msg, std::get<QString>(pattern));
}

bool DltMessageMatcher::matchText(const QDltMsg &msg, const QString &searchText) const
{
    if (!passesPreFilters(msg))
        return false;

    return matchHeaderAndPayload(msg, searchText);
}

bool DltMessageMatcher::passesPreFilters(const QDltMsg &msg) const
{
    if (!matchAppId(msg.getApid()) || !matchCtxId(msg.getCtid()))
        return false;

    if (!matchTimestampRange(msg.getTimestamp()))
        return false;

    if (m_timeRangeMs)
    {
        const qint64 timestampMSecsSinceEpoch = msg.getTime() * 1000 + msg.getMicroseconds() / 1000;
        if (!matchTimeRangeMs(timestampMSecsSinceEpoch))
            return false;
    }

    return true;
}

bool DltMessageMatcher::matchHeaderAndPayload(const QDltMsg &msg, const QString &searchText) const
{
    if (m_headerSearchEnabled) {
        auto header = msg.toStringHeader();
        if (m_messageIdFormat)
            header += ' ' + QString::asprintf(m_messageIdFormat->toUtf8(), msg.getMessageId());
        if (searchText.isEmpty() || header.contains(searchText, m_caseSensitivity))
            return true;
    }

    if (m_payloadSearchEnabled) {
        const auto payload = msg.toStringPayload();
        return searchText.isEmpty() || payload.contains(searchText, m_caseSensitivity);
    }

    return false;
}

bool DltMessageMatcher::matchHeaderAndPayload(const QDltMsg &msg, const QRegularExpression &pattern) const
{
    if (m_headerSearchEnabled) {
        auto header = msg.toStringHeader();
        if (m_messageIdFormat)
            header += ' ' + QString::asprintf(m_messageIdFormat->toUtf8(), msg.getMessageId());
        if (header.contains(pattern))
            return true;
    }

    if (m_payloadSearchEnabled) {
        const auto payload = msg.toStringPayload();
        return payload.contains(pattern);
    }

    return false;
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

bool DltMessageMatcher::matchTimeRangeMs(qint64 msSinceEpoch) const
{
    if (!m_timeRangeMs)
        return true;

    return (m_timeRangeMs->startMsSinceEpoch < msSinceEpoch) && (msSinceEpoch < m_timeRangeMs->endMsSinceEpoch);
}
