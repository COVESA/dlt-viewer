#include "dltmessagematcher.h"

#include <qdltmsg.h>

DltMessageMatcher::DltMessageMatcher() {}

bool DltMessageMatcher::match(const QDltMsg &msg) const
{
    if (!matchAppId(msg.getApid()) || !matchCtxId(msg.getCtid()))
        return false;

    if (!matchTimestampRange(msg.getTimestamp())) {
        return false;
    }

    // TODO: implement

    return true;
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
