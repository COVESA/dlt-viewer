#include <gtest/gtest.h>

#include <ecutree.h>

namespace {

qdlt::msg::payload::GetLogInfo makeGetLogInfo(
    const QString &appId,
    const QString &appDesc,
    const QString &ctxId,
    const QString &ctxDesc,
    int8_t logLevel = 1,
    int8_t traceStatus = 1)
{
    qdlt::msg::payload::GetLogInfo info;
    info.status = 7;

    qdlt::msg::payload::GetLogInfo::App app;
    app.id = appId;
    app.description = appDesc;

    qdlt::msg::payload::GetLogInfo::App::Ctx ctx;
    ctx.id = ctxId;
    ctx.description = ctxDesc;
    ctx.logLevel = logLevel;
    ctx.traceStatus = traceStatus;

    app.ctxs.push_back(ctx);
    info.apps.push_back(app);
    return info;
}

} // namespace

TEST(EcuTree, PreservesContextsFromMultipleGetLogInfoMessagesForSameApp)
{
    EcuTree tree;

    tree.add("IDCE", makeGetLogInfo("APP1", "Application 1", "CTX1", "Context 1"));
    tree.add("IDCE", makeGetLogInfo("APP1", "Application 1", "CTX2", "Context 2"));

    ASSERT_TRUE(tree.ecus.contains("IDCE"));
    const auto &apps = tree.ecus.at("IDCE");

    ASSERT_TRUE(apps.contains("APP1"));
    const auto &app = apps.at("APP1");

    EXPECT_EQ(app.description, "Application 1");
    EXPECT_EQ(app.contexts.size(), 2u);
    EXPECT_TRUE(app.contexts.contains("CTX1"));
    EXPECT_TRUE(app.contexts.contains("CTX2"));
    EXPECT_EQ(app.contexts.at("CTX1").description, "Context 1");
    EXPECT_EQ(app.contexts.at("CTX2").description, "Context 2");
}
