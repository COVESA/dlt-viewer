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

    void setTimestapmRange(double start, double end) {
        m_timestampRange = {start, end};
    }

    bool match(const QDltMsg& message) const;
private:
    bool matchAppId(const QString& appId) const;
    bool matchCtxId(const QString& ctxId) const;
    bool matchTimestampRange(unsigned int ts) const;
private:
    QString m_ctxId;
    QString m_appId;

    struct TimestampRange {
        double start;
        double end;
    };
    std::optional<TimestampRange> m_timestampRange;

    Qt::CaseSensitivity m_caseSensitivity{Qt::CaseInsensitive};
};

#endif // DLTMESSAGEMATCHER_H
