#include "dltmessagematcher.h"

#include <qdltmsg.h>

DltMessageMatcher::DltMessageMatcher() {}

bool DltMessageMatcher::match(const QDltMsg &msg) const
{
    if (!matchAppId(msg.getApid()) || !matchCtxId(msg.getCtid()))
        return false;

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
