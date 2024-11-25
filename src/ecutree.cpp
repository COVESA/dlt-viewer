#include "ecutree.h"

EcuTree::EcuTree() {}

void EcuTree::add(QString ecuid, const qdlt::msg::payload::GetLogInfo &info)
{
    for (const auto& app : info.apps) {
        auto appid = QString::fromStdString(qdlt::msg::payload::asString(app.id));
        // FIXME: this will override map entries over and over again
        ecus[ecuid][appid].description = QString::fromStdString(app.description);
        for (const auto& ctx : app.ctxs) {
            auto ctxid = QString::fromStdString(qdlt::msg::payload::asString(ctx.id));
            ecus[ecuid][appid].contexts[ctxid] = {
                ctx.logLevel, ctx.traceStatus, QString::fromStdString(ctx.description)};
        }
    }
}
