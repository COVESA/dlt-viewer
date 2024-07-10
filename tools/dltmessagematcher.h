#ifndef DLTMESSAGEMATCHER_H
#define DLTMESSAGEMATCHER_H

#include <QString>

class QDltMsg;

class DltMessageMatcher
{
public:
    DltMessageMatcher();

    void setCaseSentivity(Qt::CaseSensitivity caseSensitivity) {
        m_caseSensitivity = caseSensitivity;
    }

    void setSearchAppId(const QString& appId) {
        m_appId = appId;
    }

    void setSearchCtxId(const QString& ctxId) {
        m_ctxId = ctxId;
    }

    bool match(const QDltMsg& message) const;
private:
    bool matchAppId(const QString& appId) const;
    bool matchCtxId(const QString& ctxId) const;
private:
    QString m_ctxId;
    QString m_appId;

    Qt::CaseSensitivity m_caseSensitivity{Qt::CaseInsensitive};
};

#endif // DLTMESSAGEMATCHER_H
