#include <gtest/gtest.h>

#include <qdltmsg.h>

TEST(QDltMsgCache, payloadCacheInvalidatesOnSetPayload)
{
    QDltMsg msg;
    msg.setMode(QDltMsg::DltModeNonVerbose);
    msg.setVersionNumber(2);

    auto payload1 = QString{"abcd"}.toUtf8();
    msg.setPayload(payload1);

    const auto s1 = msg.toStringPayload();
    EXPECT_TRUE(s1.contains("abcd"));

    auto payload2 = QString{"wxyz"}.toUtf8();
    msg.setPayload(payload2);

    const auto s2 = msg.toStringPayload();
    EXPECT_TRUE(s2.contains("wxyz"));
    EXPECT_FALSE(s2.contains("abcd"));
}

TEST(QDltMsgCache, headerCacheInvalidatesOnFieldChange)
{
    QDltMsg msg;
    msg.setApid("APP1");
    msg.setCtid("CTX1");
    msg.setEcuid("ECU1");
    msg.setTime(123456789);

    const auto h1 = msg.toStringHeader();
    EXPECT_TRUE(h1.contains("APP1"));

    msg.setApid("APP2");

    const auto h2 = msg.toStringHeader();
    EXPECT_TRUE(h2.contains("APP2"));
    EXPECT_FALSE(h2.contains("APP1"));
}
