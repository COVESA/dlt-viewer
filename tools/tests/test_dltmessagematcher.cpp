#include <gtest/gtest.h>

#include "dltmessagematcher.h"
#include <qdltmsg.h>

TEST(DltMessageMatcher, matchAppId) {
    QDltMsg msg;
    msg.setApid("Bla");

    DltMessageMatcher matcher;

    // case insensitive search
    matcher.setSearchAppId("bla");
    EXPECT_TRUE(matcher.match(msg));

    // case sensitive search
    matcher.setCaseSentivity(Qt::CaseSensitive);
    EXPECT_FALSE(matcher.match(msg));
}

TEST(DltMessageMatcher, matchCtxId) {
    QDltMsg msg;
    msg.setCtid("Bla");

    DltMessageMatcher matcher;

    // case insensitive search
    matcher.setSearchCtxId("bla");
    EXPECT_TRUE(matcher.match(msg));

    // case sensitive search
    matcher.setCaseSentivity(Qt::CaseSensitive);
    EXPECT_FALSE(matcher.match(msg));
}

TEST(DltMessageMatcher, matchTimestampRange) {
    QDltMsg msg;
    msg.setTimestamp(50000);
    qInfo() << msg.toStringHeader() << msg.toStringPayload();

    DltMessageMatcher matcher;

    // no timestamp range is set
    EXPECT_TRUE(matcher.match(msg));

    // in the range
    matcher.setTimestapmRange(static_cast<double>(msg.getTimestamp() - 10) / 10'000,
                              static_cast<double>(msg.getTimestamp() + 10) / 10'000);
    EXPECT_TRUE(matcher.match(msg));

    // range is to the left
    matcher.setTimestapmRange(static_cast<double>(msg.getTimestamp() - 100) / 10'000,
                              static_cast<double>(msg.getTimestamp() - 10) / 10'000);
    EXPECT_FALSE(matcher.match(msg));

    // range is to the right
    matcher.setTimestapmRange(static_cast<double>(msg.getTimestamp() + 10) / 10'000,
                              static_cast<double>(msg.getTimestamp() + 100) / 10'000);
    EXPECT_FALSE(matcher.match(msg));
}
