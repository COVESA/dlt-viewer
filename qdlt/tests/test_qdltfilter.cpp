// SPDX-License-Identifier: MPL-2.0
// Tests for QDltFilter::match() covering all filter fields and combinations.

#include <gtest/gtest.h>

#include "qdltfilter.h"
#include "qdltmsg.h"

// ---------------------------------------------------------------------------
// Helper: build a minimal QDltMsg with the most common fields set
// ---------------------------------------------------------------------------
static QDltMsg makeMsg(const QString &ecuid   = "ECU1",
                       const QString &apid    = "APP1",
                       const QString &ctid    = "CTX1",
                       QDltMsg::DltLogDef loglevel = QDltMsg::DltLogInfo)
{
    QDltMsg msg;
    msg.setEcuid(ecuid);
    msg.setApid(apid);
    msg.setCtid(ctid);
    msg.setType(QDltMsg::DltTypeLog);
    msg.setSubtype(static_cast<unsigned char>(loglevel));
    return msg;
}

// ---------------------------------------------------------------------------
// Default state: filter with no criteria enabled should always match
// ---------------------------------------------------------------------------
TEST(QDltFilter, defaultMatchesEverything)
{
    QDltFilter filter;
    filter.type = QDltFilter::positive;
    filter.compileRegexps();

    EXPECT_TRUE(filter.match(makeMsg()));
    EXPECT_TRUE(filter.match(makeMsg("OTHER", "OTHERAPP", "OTHERCTX")));
}

// ---------------------------------------------------------------------------
// ECU ID filtering
// ---------------------------------------------------------------------------
TEST(QDltFilter, ecuIdMatch_exactHit)
{
    QDltFilter filter;
    filter.enableEcuid = true;
    filter.ecuid = "ECU1";
    filter.compileRegexps();

    EXPECT_TRUE(filter.match(makeMsg("ECU1")));
}

TEST(QDltFilter, ecuIdMatch_miss)
{
    QDltFilter filter;
    filter.enableEcuid = true;
    filter.ecuid = "ECU1";
    filter.compileRegexps();

    EXPECT_FALSE(filter.match(makeMsg("ECU2")));
}

// ---------------------------------------------------------------------------
// Application ID filtering — plain and regex
// ---------------------------------------------------------------------------
TEST(QDltFilter, apidMatch_plain)
{
    QDltFilter filter;
    filter.enableApid = true;
    filter.apid = "APP1";
    filter.enableRegexp_Appid = false;
    filter.compileRegexps();

    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1")));
    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP2")));
}

TEST(QDltFilter, apidMatch_regex)
{
    QDltFilter filter;
    filter.enableApid = true;
    filter.apid = "APP[0-9]+";
    filter.enableRegexp_Appid = true;
    filter.compileRegexps();

    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1")));
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP99")));
    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APPABC")));
}

// ---------------------------------------------------------------------------
// Context ID filtering — plain (substring) and regex
// ---------------------------------------------------------------------------
TEST(QDltFilter, ctidMatch_plain_substring)
{
    QDltFilter filter;
    filter.enableCtid = true;
    filter.ctid = "CTX";
    filter.enableRegexp_Context = false;
    filter.compileRegexps();

    // plain mode uses contains(), so "CTX" is a substring of "CTX1"
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1")));
    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP1", "OTHER")));
}

TEST(QDltFilter, ctidMatch_regex)
{
    QDltFilter filter;
    filter.enableCtid = true;
    filter.ctid = "CTX[0-9]";
    filter.enableRegexp_Context = true;
    filter.compileRegexps();

    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1")));
    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP1", "CTXA")));
}

// ---------------------------------------------------------------------------
// Log level range filtering
// ---------------------------------------------------------------------------
TEST(QDltFilter, logLevelMax_inRange)
{
    QDltFilter filter;
    filter.enableLogLevelMax = true;
    filter.logLevelMax = static_cast<int>(QDltMsg::DltLogWarn); // warn=4
    filter.compileRegexps();

    // fatal(1), error(2), warn(3) are <= max
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogFatal)));
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogError)));
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogWarn)));
}

TEST(QDltFilter, logLevelMax_outOfRange)
{
    QDltFilter filter;
    filter.enableLogLevelMax = true;
    filter.logLevelMax = static_cast<int>(QDltMsg::DltLogWarn);
    filter.compileRegexps();

    // info(5) and debug(6) exceed max
    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogInfo)));
    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogDebug)));
}

TEST(QDltFilter, logLevelMin_inRange)
{
    QDltFilter filter;
    filter.enableLogLevelMin = true;
    filter.logLevelMin = static_cast<int>(QDltMsg::DltLogWarn); // warn=4
    filter.compileRegexps();

    // warn, info, debug are >= min
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogWarn)));
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogInfo)));
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogDebug)));
}

TEST(QDltFilter, logLevelMin_outOfRange)
{
    QDltFilter filter;
    filter.enableLogLevelMin = true;
    filter.logLevelMin = static_cast<int>(QDltMsg::DltLogWarn);
    filter.compileRegexps();

    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogFatal)));
    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogError)));
}

TEST(QDltFilter, logLevelMinMax_combined)
{
    QDltFilter filter;
    filter.enableLogLevelMin = true;
    filter.enableLogLevelMax = true;
    filter.logLevelMin = static_cast<int>(QDltMsg::DltLogError); // >=2
    filter.logLevelMax = static_cast<int>(QDltMsg::DltLogWarn);  // <=4
    filter.compileRegexps();

    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogFatal)));
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogError)));
    EXPECT_TRUE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogWarn)));
    EXPECT_FALSE(filter.match(makeMsg("ECU1", "APP1", "CTX1", QDltMsg::DltLogInfo)));
}

// ---------------------------------------------------------------------------
// Control message filtering
// ---------------------------------------------------------------------------
TEST(QDltFilter, ctrlMsgFilter_matchesControl)
{
    QDltFilter filter;
    filter.enableCtrlMsgs = true;
    filter.compileRegexps();

    QDltMsg ctrlMsg;
    ctrlMsg.setType(QDltMsg::DltTypeControl);
    EXPECT_TRUE(filter.match(ctrlMsg));
}

TEST(QDltFilter, ctrlMsgFilter_rejectsLogMsg)
{
    QDltFilter filter;
    filter.enableCtrlMsgs = true;
    filter.compileRegexps();

    EXPECT_FALSE(filter.match(makeMsg())); // DltTypeLog
}

// ---------------------------------------------------------------------------
// Filter type helpers
// ---------------------------------------------------------------------------
TEST(QDltFilter, filterTypePositive)
{
    QDltFilter filter;
    filter.type = QDltFilter::positive;
    EXPECT_TRUE(filter.isPositive());
    EXPECT_FALSE(filter.isNegative());
    EXPECT_FALSE(filter.isMarker());
}

TEST(QDltFilter, filterTypeNegative)
{
    QDltFilter filter;
    filter.type = QDltFilter::negative;
    EXPECT_FALSE(filter.isPositive());
    EXPECT_TRUE(filter.isNegative());
    EXPECT_FALSE(filter.isMarker());
}

TEST(QDltFilter, filterTypeMarker)
{
    QDltFilter filter;
    filter.type = QDltFilter::marker;
    EXPECT_FALSE(filter.isPositive());
    EXPECT_FALSE(filter.isNegative());
    EXPECT_TRUE(filter.isMarker());
}

// ---------------------------------------------------------------------------
// enableMarker overrides type for isMarker()
// ---------------------------------------------------------------------------
TEST(QDltFilter, enableMarkerOverridesType)
{
    QDltFilter filter;
    filter.type = QDltFilter::positive;
    filter.enableMarker = true;
    EXPECT_TRUE(filter.isMarker());
}

// ---------------------------------------------------------------------------
// compileRegexps returns false for invalid patterns
// ---------------------------------------------------------------------------
TEST(QDltFilter, compileRegexps_invalidPayloadPattern)
{
    QDltFilter filter;
    filter.enableRegexp_Payload = true;
    filter.payload = "[invalid(";
    EXPECT_FALSE(filter.compileRegexps());
}

TEST(QDltFilter, compileRegexps_validPatterns)
{
    QDltFilter filter;
    filter.enableRegexp_Appid   = true;
    filter.enableRegexp_Context = true;
    filter.enableRegexp_Header  = true;
    filter.enableRegexp_Payload = true;
    filter.apid    = "APP.*";
    filter.ctid    = "CTX[0-9]";
    filter.header  = ".*error.*";
    filter.payload = "value=\\d+";
    EXPECT_TRUE(filter.compileRegexps());
}

// ---------------------------------------------------------------------------
// copy operator preserves all fields
// ---------------------------------------------------------------------------
TEST(QDltFilter, copyOperatorPreservesFields)
{
    QDltFilter src;
    src.type               = QDltFilter::negative;
    src.name               = "my filter";
    src.ecuid              = "ECU1";
    src.apid               = "APP1";
    src.ctid               = "CTX1";
    src.enableEcuid        = true;
    src.enableApid         = true;
    src.enableCtid         = true;
    src.enableLogLevelMax  = true;
    src.logLevelMax        = 4;
    src.filterColour       = "#ff0000";

    QDltFilter dst;
    dst = src;

    EXPECT_EQ(dst.type,              src.type);
    EXPECT_EQ(dst.name,              src.name);
    EXPECT_EQ(dst.ecuid,             src.ecuid);
    EXPECT_EQ(dst.apid,              src.apid);
    EXPECT_EQ(dst.ctid,              src.ctid);
    EXPECT_EQ(dst.enableEcuid,       src.enableEcuid);
    EXPECT_EQ(dst.enableApid,        src.enableApid);
    EXPECT_EQ(dst.enableCtid,        src.enableCtid);
    EXPECT_EQ(dst.enableLogLevelMax, src.enableLogLevelMax);
    EXPECT_EQ(dst.logLevelMax,       src.logLevelMax);
    EXPECT_EQ(dst.filterColour,      src.filterColour);
}

// ---------------------------------------------------------------------------
// clear() resets all fields to defaults
// ---------------------------------------------------------------------------
TEST(QDltFilter, clearResetsFields)
{
    QDltFilter filter;
    filter.enableEcuid = true;
    filter.ecuid       = "ECU1";
    filter.logLevelMax = 3;
    filter.enableLogLevelMax = true;

    filter.clear();

    EXPECT_FALSE(filter.enableEcuid);
    EXPECT_TRUE(filter.ecuid.isEmpty());
    EXPECT_FALSE(filter.enableLogLevelMax);
}
