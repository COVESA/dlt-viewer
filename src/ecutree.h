#ifndef ECUTREE_H
#define ECUTREE_H

#include <QString>
#include <map>

#include <qdltctrlmsg.h>

#include <QDebug>

class EcuTree {
  public:
    EcuTree();

    void add(QString ecuid, const qdlt::msg::payload::GetLogInfo &info);

    struct App {
        struct Context {
            int8_t logLevel;
            int8_t traceStatus;
            QString description;
        };

        QString description;

        // ctxid -> ctx metadata
        std::map<QString, Context> contexts;
    };
    // appid -> app metadata
    using Apps = std::map<QString, App>;

    // ecu id -> Apps
    std::map<QString, Apps> ecus;
};

struct CtrlMsgData {

    void setEcuId(const QString& ecuId) { this->ecuId = ecuId; }

    void operator()(qdlt::msg::payload::GetLogInfo&& info) { ecuTree.add(ecuId, info); };

    void operator()(qdlt::msg::payload::GetSoftwareVersion&& swversion) {
        softwareVersions[ecuId] = swversion;
    };

    void operator()(qdlt::msg::payload::GetDefaultLogLevel&& loglevel) {
        defaultLogLevels[ecuId] = loglevel;
    };

    void operator()(qdlt::msg::payload::SetLogLevel&&){};

    void operator()(qdlt::msg::payload::Timezone&& timezone) {
        this->timezone = timezone;
    };

    void operator()(qdlt::msg::payload::UnregisterContext&& unregister) {
        unregisteredContexts[ecuId] = unregister;
    };

    QString ecuId;

    EcuTree ecuTree;
    qdlt::msg::payload::Timezone timezone;
    std::map<QString, qdlt::msg::payload::UnregisterContext> unregisteredContexts;
    std::map<QString, qdlt::msg::payload::GetDefaultLogLevel> defaultLogLevels;
    std::map<QString, qdlt::msg::payload::GetSoftwareVersion> softwareVersions;
};

#endif // ECUTREE_H
