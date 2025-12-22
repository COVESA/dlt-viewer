#include <gtest/gtest.h>

#include "dltmessagematcher.h"
#include <qdltmsg.h>

TEST(DltMessageMatcher, matchAppId) {
    QDltMsg msg;
    msg.setApid("Bla");

    DltMessageMatcher matcher;

    // case insensitive search
    matcher.setSearchAppId("bla");
    EXPECT_TRUE(matcher.match(msg, QString{}));

    // case sensitive search
    matcher.setCaseSentivity(Qt::CaseSensitive);
    EXPECT_FALSE(matcher.match(msg, QString{}));
}

TEST(DltMessageMatcher, matchCtxId) {
    QDltMsg msg;
    msg.setCtid("Bla");

    DltMessageMatcher matcher;

    // case insensitive search
    matcher.setSearchCtxId("bla");
    EXPECT_TRUE(matcher.match(msg, QString{}));

    // case sensitive search
    matcher.setCaseSentivity(Qt::CaseSensitive);
    EXPECT_FALSE(matcher.match(msg, QString{}));
}

TEST(DltMessageMatcher, matchTimestampRange) {
    QDltMsg msg;
    msg.setTimestamp(50000);


    DltMessageMatcher matcher;

    // no timestamp range is set
    EXPECT_TRUE(matcher.match(msg, QString{}));

    // in the range
    matcher.setTimestampRange(static_cast<double>(msg.getTimestamp() - 10) / 10'000,
                              static_cast<double>(msg.getTimestamp() + 10) / 10'000);
    EXPECT_TRUE(matcher.match(msg, QString{}));

    // range is to the left
    matcher.setTimestampRange(static_cast<double>(msg.getTimestamp() - 100) / 10'000,
                              static_cast<double>(msg.getTimestamp() - 10) / 10'000);
    EXPECT_FALSE(matcher.match(msg, QString{}));

    // range is to the right
    matcher.setTimestampRange(static_cast<double>(msg.getTimestamp() + 10) / 10'000,
                              static_cast<double>(msg.getTimestamp() + 100) / 10'000);
    EXPECT_FALSE(matcher.match(msg, QString{}));
}

TEST(DltMessageMatcher, matchMessageHeader) {
    QDltMsg msg;
    msg.setMicroseconds(4242);
    msg.setTimestamp(45);
    msg.setMessageCounter(2);
    msg.setEcuid("ecuId");
    msg.setApid("appId");
    msg.setCtid("ctxId");
    msg.setSessionid(56);
    msg.setType(QDltMsg::DltTypeNwTrace);
    msg.setSubtype(3);
    msg.setMode(QDltMsg::DltModeNonVerbose);
    msg.setNumberOfArguments(255);
    msg.setTime(123456789);

    DltMessageMatcher matcher;
    matcher.setHeaderSearchEnabled(true);
    matcher.setPayloadSearchEnabled(false);

    // simple text match
    // empty header matches
    EXPECT_TRUE(matcher.match(msg, ""));
    EXPECT_TRUE(matcher.match(msg, "2 ecuId appId"));
    EXPECT_FALSE(matcher.match(msg, "4243"));

    // regexp match
    // simple text as regexp
    EXPECT_TRUE(matcher.match(msg, QRegularExpression("ctxId")));
    // actual regexp: message starts with a date
    EXPECT_TRUE(matcher.match(msg, QRegularExpression("^\\d\\d\\d\\d/\\d\\d/\\d\\d ")));
    // actual regexp: somewhere in the middle there is "appId"-word separated from a next word with a space,
    // at the end of the string there is a number
    EXPECT_TRUE(matcher.match(msg, QRegularExpression("appId \\w+ .+\\d+$")));
}

TEST(DltMessageMatcher, matchMessagePayload) {
    QDltMsg msg;
    msg.setIndex(42);
    msg.setEcuid("efgh");
    msg.setMode(QDltMsg::DltModeNonVerbose);
    auto ba = QString{"abcd"}.toUtf8();
    msg.setPayload(ba);
    // version number is set to make QDltMsg::toStringPayload produce string with payload
    msg.setVersionNumber(2);

    DltMessageMatcher matcher;
    matcher.setHeaderSearchEnabled(false);
    matcher.setPayloadSearchEnabled(true);

    // simple text match
    // empty header matches
    EXPECT_TRUE(matcher.match(msg, ""));
    EXPECT_FALSE(matcher.match(msg, "efgh"));
    EXPECT_TRUE(matcher.match(msg, "abc"));

    //regexp match
    // simple text as regexp
    EXPECT_TRUE(matcher.match(msg, QRegularExpression("cd")));
    // actual regexp, match string "[0]  abcd|61 62 63 64"
    EXPECT_TRUE(matcher.match(msg, QRegularExpression("^\\[\\d\\]\\s+\\w+|[\\d\\s]+$")));
}

TEST(DltMessageMatcher, doNotMatchEmptyMessagePayload) {
    QDltMsg msg;
    msg.setApid("abc");
    msg.setCtid("def");
    auto ba = QString{}.toUtf8();
    msg.setPayload(ba);

    DltMessageMatcher matcher;
    matcher.setHeaderSearchEnabled(true);
    matcher.setPayloadSearchEnabled(true);

    // simple text does not match empty payload
    EXPECT_FALSE(matcher.match(msg, "efgh"));
}
