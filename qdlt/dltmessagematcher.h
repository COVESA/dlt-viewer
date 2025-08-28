#ifndef DLTMESSAGEMATCHER_H
#define DLTMESSAGEMATCHER_H

#include "export_rules.h"

#include <QString>
#include <QRegularExpression>

#include <optional>
#include <variant>

class QDltMsg;

/**
 * A class which can be used to set search parameters for matching on potential DLT messages.
 *
 * Offers functions to search different parts of a DLT message.
 * Offers the option to search either the header, the payload, or both.
 * Searches can be case-sensitive - default is case-insensitive.
 * The pattern to search a payload can be either a regular expression or plain text.
 *
 * Currently used by the UI search dialog.
 * @see SearchDialog
 */
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

    void setTimestapmRange(double start, double end) {
        m_timestampRange = {start, end};
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

    /**
     *
     * @return `true` if `msg` matches all of:
     *  - application ID set in `this`
     *  - context ID set in `this`
     *  - timestamp range set in `this`
     *  - pattern matches either the `msg` header or payload
     *  `false` otherwise.
     *
     *  @see SearchDialog::findMessages
     */
    bool match(const QDltMsg& message, const Pattern& pattern) const;

private:

    /**
     * Match the application ID in `appId` against `this`
     *
     * @return `true` if `this` has no application ID set or the IDs match exactly, false otherwise
     */
    bool matchAppId(const QString& appId) const;
    /**
     * Match the context ID in `ctxId` against `this`
     *
     * @return `true` if `this` has no application ID set or the IDs match exactly, false otherwise
     */
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

    bool m_headerSearchEnabled{true};
    bool m_payloadSearchEnabled{true};

    std::optional<QString> m_messageIdFormat;
};

#endif // DLTMESSAGEMATCHER_H
