#include "ecutree.h"

void EcuTree::add(const QString& ecuId, const qdlt::msg::payload::GetLogInfo& info)
{
    for (const auto& app : info.apps) {
        App appData;
        appData.description = app.description;
        for (const auto& ctx : app.ctxs) {
            Ctx ctxData;
            ctxData.description = ctx.description;
            ctxData.logLevel = ctx.logLevel;
            ctxData.traceStatus = ctx.traceStatus;
            appData.contexts[ctx.id] = std::move(ctxData);
        }
        ecus[ecuId][app.id] = std::move(appData);
    }
}
