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
