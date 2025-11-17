#ifndef DLTMESSAGEMATCHER_H
#define DLTMESSAGEMATCHER_H

#include "export_rules.h"

#include <QString>
#include <QRegularExpression>
#include <QDateTime>

#include <optional>
#include <variant>

class QDltMsg;

class QDLT_EXPORT DltMessageMatcher
{
public:
    using Pattern = std::variant<QString, QRegularExpression>;

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

    void setTimestampRange(double start, double end) {
        m_timestampRange = {start, end};
    }

    void setTimeRange(QDateTime start, QDateTime end) {
        m_timeRange = {std::move(start), std::move(end)};
    }

    void setHeaderSearchEnabled(bool enabled) {
        m_headerSearchEnabled = enabled;
    }

    void setPayloadSearchEnabled(bool enabled) {
        m_payloadSearchEnabled = enabled;
    }

    void setMessageIdFormat(const QString& msgIdFormat) {
        m_messageIdFormat = msgIdFormat;
    }

    bool match(const QDltMsg& message, const Pattern& pattern) const;
private:
    bool matchAppId(const QString& appId) const;
    bool matchCtxId(const QString& ctxId) const;
    bool matchTimestampRange(unsigned int ts) const;
    bool matchTimeRange(const QDateTime& seconds) const;
private:
    QString m_ctxId;
    QString m_appId;

    struct TimestampRange {
        double start;
        double end;
    };
    std::optional<TimestampRange> m_timestampRange;

    struct TimeRange {
        QDateTime start;
        QDateTime end;
    };
    std::optional<TimeRange> m_timeRange;

    Qt::CaseSensitivity m_caseSensitivity{Qt::CaseInsensitive};

    bool m_headerSearchEnabled{true};
    bool m_payloadSearchEnabled{true};

    std::optional<QString> m_messageIdFormat;
};

#endif // DLTMESSAGEMATCHER_H
