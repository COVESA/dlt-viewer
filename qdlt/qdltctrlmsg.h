#ifndef QDLTCTRLMSG_H
#define QDLTCTRLMSG_H

#include <string>
#include <vector>
#include <variant>

#include <dlt_common.h>

namespace qdlt::msg::payload {

using IdType = std::array<char, DLT_ID_SIZE>;

std::string asString(const IdType& id);

struct GetLogInfo {
    struct App {
        struct Ctx {
            IdType id;
            int8_t logLevel;
            int8_t traceStatus;
            std::string description;
        };

        IdType id;
        std::string description;
        std::vector<Ctx> ctxs;
    };

    uint8_t status;
    std::vector<App> apps;
};

struct GetSoftwareVersion {
    std::string version;
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
    IdType appid;
    IdType ctxid;
};

using Type = std::variant<GetLogInfo, GetSoftwareVersion, GetDefaultLogLevel, SetLogLevel, Timezone,
                          UnregisterContext>;

Type parse(std::string_view data, bool isBigEndian);

} // namespace qdlt::msg::payload

#endif // QDLTCTRLMSG_H
