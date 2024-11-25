#ifndef ECUTREE_H
#define ECUTREE_H

#include <QString>
#include <map>

#include <qdltctrlmsg.h>

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

#endif // ECUTREE_H
