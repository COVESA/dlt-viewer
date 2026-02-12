#ifndef QDLTCTRLMSG_H
#define QDLTCTRLMSG_H

#include "export_rules.h"

#include <vector>
#include <variant>

#include <QByteArray>
#include <QString>


namespace qdlt::msg::payload {

struct GetLogInfo {
    struct App {
        struct Ctx {
            QString id;
            int8_t logLevel;
            int8_t traceStatus;
            QString description;
        };

        QString id;
        QString description;
        std::vector<Ctx> ctxs;
    };

    uint8_t status;
    std::vector<App> apps;
};

struct GetSoftwareVersion {
};

struct GetDefaultLogLevel
{
    uint8_t logLevel;
    uint8_t status;
};

struct SetLogLevel {
    uint8_t status;
};

struct Timezone {
    uint8_t status;
    int32_t timezone;
    uint8_t isDst;
};

struct UnregisterContext {
    uint8_t status;
    QString appid;
    QString ctxid;
};

struct Uninteresting {
    uint32_t serviceId;
    bool parseError = false;
};

using Type = std::variant<GetLogInfo, GetSoftwareVersion, GetDefaultLogLevel, SetLogLevel, Timezone,
                          UnregisterContext, Uninteresting>;

QDLT_EXPORT Type parse(const QByteArray&, bool isBigEndian);

} // namespace qdlt::msg::payload

// helper type for the visitor
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

#endif // QDLTCTRLMSG_H
