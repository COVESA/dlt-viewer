#include <gtest/gtest.h>

#include <qdltctrlmsg.h>

#include <QByteArray>

TEST(CtrlPayload, parse) {
    // this is real payload of a dlt control message which collect from dlt-daemon
    const auto data = QByteArray::fromHex("030000000701005359530001004d475200ffff2200436f6e74657874206f66206d61696e20646c742073797374656d206d616e616765721200444c542053797374656d204d616e6167657272656d6f");

    auto payload = qdlt::msg::payload::parse(data, false);

    ASSERT_TRUE(std::holds_alternative<qdlt::msg::payload::GetLogInfo>(payload));
    auto getLogInfo = std::get<qdlt::msg::payload::GetLogInfo>(payload);
    EXPECT_EQ(getLogInfo.status, 7);
    ASSERT_EQ(getLogInfo.apps.size(), 1);

    auto app = getLogInfo.apps[0];
    EXPECT_EQ(app.id, "SYS");
    EXPECT_EQ(app.description, "DLT System Manager");
    ASSERT_EQ(app.ctxs.size(), 1);

    auto ctx = app.ctxs[0];
    EXPECT_EQ(ctx.id, "MGR");
    EXPECT_EQ(ctx.logLevel, '\xff');
    EXPECT_EQ(ctx.traceStatus, '\xff');
    EXPECT_EQ(ctx.description, "Context of main dlt system manager");
}
