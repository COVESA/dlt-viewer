#include <gtest/gtest.h>

#include "dltmessagematcher.h"
#include <qdltmsg.h>

TEST(DltMessageMatcher, Dummy) {
    DltMessageMatcher matcher;

    EXPECT_TRUE(matcher.match(QDltMsg{}));
}
