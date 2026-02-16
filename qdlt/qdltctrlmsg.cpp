#include "qdltctrlmsg.h"

#include "dlt_common.h"

#include <exception>
#include <stdexcept>
#include <cstring>
#include <array>

#include <QByteArray>

namespace qdlt::msg::payload {

namespace {
using IdType = std::array<char, DLT_ID_SIZE>;

QString asQString(IdType&& id) {
    QString str;
    for (size_t i = 0; i < id.size() && id[i] != '\0'; i++) {
        str.append(id[i]);
    }
    return str;
}

template <typename T>
T dltPayloadRead(const char *&dataPtr, int32_t &length, bool isBigEndian)
{
    if (length < 0 || length < static_cast<int32_t>(sizeof(T))) {
        throw std::runtime_error("Invalid data length");
    }

    T value{};
    std::memcpy(&value, dataPtr, sizeof(T));
    dataPtr += sizeof(T);
    length -= sizeof(T);

    if constexpr (sizeof(T) == sizeof(uint32_t)) {
        value = DLT_ENDIAN_GET_32((isBigEndian ? DLT_HTYP_MSBF : 0), value);
    }

    if constexpr (sizeof(T) == sizeof(uint16_t)) {
        value = DLT_ENDIAN_GET_16((isBigEndian ? DLT_HTYP_MSBF : 0), value);
    }

    return value;
}

IdType dltPayloadReadId(const char *&dataPtr, int32_t &length)
{
    if (length < 0 || length < DLT_ID_SIZE) {
        throw std::runtime_error("Invalid ID length");
    }
    IdType id{};
    std::copy(dataPtr, dataPtr + DLT_ID_SIZE, id.begin());
    dataPtr += DLT_ID_SIZE;
    length -= DLT_ID_SIZE;

    return id;
}

std::string dltPayloadReadString(const char *&dataPtr, int32_t &length, bool isBigEndian)
{
    uint16_t strLength = dltPayloadRead<uint16_t>(dataPtr, length, isBigEndian);
    if (length < 0 || strLength > length) {
        throw std::runtime_error(QString("Invalid string length %1 > %2").arg(strLength).arg(length).toStdString());
    }
    std::string str;
    str.assign(dataPtr, strLength);
    dataPtr += strLength;
    length -= strLength;

    return str;
}
}

Type parse(const QByteArray& data, bool isBigEndian)
{
    int32_t length = data.length();
    const char *dataPtr = data.data();
    uint32_t serviceId = 0;

    try {
        serviceId = dltPayloadRead<uint32_t>(dataPtr, length, isBigEndian);
        switch (serviceId) {
        case DLT_SERVICE_ID_GET_LOG_INFO:
        {
            GetLogInfo msg;
            msg.status = dltPayloadRead<uint8_t>(dataPtr, length, isBigEndian);
            if ((msg.status != 6) && (msg.status != 7)) {
                return msg;
            }

            const auto numApps = dltPayloadRead<uint16_t>(dataPtr, length, isBigEndian);
            for (uint16_t i = 0; i < numApps; i++) {
                GetLogInfo::App app;
                app.id = asQString(dltPayloadReadId(dataPtr, length));
                const uint16_t numCtx = dltPayloadRead<uint16_t>(dataPtr, length, isBigEndian);
                for (uint16_t j = 0; j < numCtx; j++) {
                    GetLogInfo::App::Ctx ctx;
                    ctx.id = asQString(dltPayloadReadId(dataPtr, length));
                    ctx.logLevel = dltPayloadRead<int8_t>(dataPtr, length, isBigEndian);
                    ctx.traceStatus = dltPayloadRead<int8_t>(dataPtr, length, isBigEndian);
                    ctx.description = QString::fromStdString(dltPayloadReadString(dataPtr, length, isBigEndian));
                    app.ctxs.push_back(std::move(ctx));
                }
                if (msg.status == 7) {
                    app.description = QString::fromStdString(dltPayloadReadString(dataPtr, length, isBigEndian));
                }
                msg.apps.push_back(std::move(app));
            }
            return msg;
        }
        case DLT_SERVICE_ID_GET_SOFTWARE_VERSION:
        {
            return GetSoftwareVersion{};
        }
        case DLT_SERVICE_ID_GET_DEFAULT_LOG_LEVEL:
        {
            GetDefaultLogLevel msg;
            msg.logLevel = dltPayloadRead<int8_t>(dataPtr, length, isBigEndian);
            msg.status = dltPayloadRead<uint8_t>(dataPtr, length, isBigEndian);
            return msg;
        }
        case DLT_SERVICE_ID_SET_LOG_LEVEL:
        {
            SetLogLevel msg;
            msg.status = dltPayloadRead<uint8_t>(dataPtr, length, isBigEndian);
            return msg;
        }
        case DLT_SERVICE_ID_TIMEZONE:
        {
            Timezone msg;
            msg.status = dltPayloadRead<uint8_t>(dataPtr, length, isBigEndian);
            msg.timezone = dltPayloadRead<int32_t>(dataPtr, length, isBigEndian);
            msg.isDst = dltPayloadRead<uint8_t>(dataPtr, length, isBigEndian);
            return msg;
        }
        case DLT_SERVICE_ID_UNREGISTER_CONTEXT:
        {
            UnregisterContext msg;
            msg.status = dltPayloadRead<uint8_t>(dataPtr, length, isBigEndian);
            msg.appid = asQString(dltPayloadReadId(dataPtr, length));
            msg.ctxid = asQString(dltPayloadReadId(dataPtr, length));
            return msg;
        }
        }

        return Uninteresting{serviceId, false};
    } catch (const std::exception&) {
        return Uninteresting{serviceId, true};
    } catch (...) {
        return Uninteresting{serviceId, true};
    }
}

}
