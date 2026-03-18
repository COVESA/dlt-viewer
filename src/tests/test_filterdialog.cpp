// SPDX-License-Identifier: MPL-2.0
// QTest coverage for FilterDialog — setter / getter round-trips and
// basic UI interaction (type toggle, regex enable/disable).

#include <QTest>
#include <QApplication>

#include "filterdialog.h"

class TestFilterDialog : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------
    // Round-trip: set a value via setter, read it back via getter
    // ------------------------------------------------------------------
    void test_setGetType_positive()
    {
        FilterDialog dlg;
        dlg.setType(0); // positive
        QCOMPARE(dlg.getType(), 0);
    }

    void test_setGetType_negative()
    {
        FilterDialog dlg;
        dlg.setType(1);
        QCOMPARE(dlg.getType(), 1);
    }

    void test_setGetType_marker()
    {
        FilterDialog dlg;
        dlg.setType(2);
        QCOMPARE(dlg.getType(), 2);
    }

    void test_setGetName()
    {
        FilterDialog dlg;
        dlg.setName("MyFilter");
        QCOMPARE(dlg.getName(), QString("MyFilter"));
    }

    void test_setGetName_empty()
    {
        FilterDialog dlg;
        dlg.setName("");
        QCOMPARE(dlg.getName(), QString(""));
    }

    void test_setGetEcuId()
    {
        FilterDialog dlg;
        dlg.setEcuId("ECU1");
        QCOMPARE(dlg.getEcuId(), QString("ECU1"));
    }

    void test_setGetEcuId_enable()
    {
        FilterDialog dlg;
        dlg.setEnableEcuId(true);
        QVERIFY(dlg.getEnableEcuId());
        dlg.setEnableEcuId(false);
        QVERIFY(!dlg.getEnableEcuId());
    }

    void test_setGetApplicationId()
    {
        FilterDialog dlg;
        dlg.setApplicationId("APP1");
        QCOMPARE(dlg.getApplicationId(), QString("APP1"));
    }

    void test_setGetApplicationId_enable()
    {
        FilterDialog dlg;
        dlg.setEnableApplicationId(true);
        QVERIFY(dlg.getEnableApplicationId());
        dlg.setEnableApplicationId(false);
        QVERIFY(!dlg.getEnableApplicationId());
    }

    void test_setGetContextId()
    {
        FilterDialog dlg;
        dlg.setContextId("CTX1");
        QCOMPARE(dlg.getContextId(), QString("CTX1"));
    }

    void test_setGetContextId_enable()
    {
        FilterDialog dlg;
        dlg.setEnableContextId(true);
        QVERIFY(dlg.getEnableContextId());
    }

    void test_setGetHeaderText()
    {
        FilterDialog dlg;
        dlg.setHeaderText("someHeader");
        QCOMPARE(dlg.getHeaderText(), QString("someHeader"));
    }

    void test_setGetHeaderText_enable()
    {
        FilterDialog dlg;
        dlg.setEnableHeaderText(true);
        QVERIFY(dlg.getEnableHeaderText());
    }

    void test_setGetPayloadText()
    {
        FilterDialog dlg;
        dlg.setPayloadText("some payload");
        QCOMPARE(dlg.getPayloadText(), QString("some payload"));
    }

    void test_setGetPayloadText_enable()
    {
        FilterDialog dlg;
        dlg.setEnablePayloadText(true);
        QVERIFY(dlg.getEnablePayloadText());
    }

    void test_setGetLogLevelMax()
    {
        FilterDialog dlg;
        dlg.setLogLevelMax(5);
        QCOMPARE(dlg.getLogLevelMax(), 5);
    }

    void test_setGetLogLevelMin()
    {
        FilterDialog dlg;
        dlg.setLogLevelMin(2);
        QCOMPARE(dlg.getLogLevelMin(), 2);
    }

    void test_setGetLogLevelMax_enable()
    {
        FilterDialog dlg;
        dlg.setEnableLogLevelMax(true);
        QVERIFY(dlg.getEnableLogLevelMax());
    }

    void test_setGetLogLevelMin_enable()
    {
        FilterDialog dlg;
        dlg.setEnableLogLevelMin(true);
        QVERIFY(dlg.getEnableLogLevelMin());
    }

    void test_setGetActive_true()
    {
        FilterDialog dlg;
        dlg.setActive(true);
        QVERIFY(dlg.getEnableActive());
    }

    void test_setGetActive_false()
    {
        FilterDialog dlg;
        dlg.setActive(false);
        QVERIFY(!dlg.getEnableActive());
    }

    void test_setGetEnableCtrlMsgs()
    {
        FilterDialog dlg;
        dlg.setEnableCtrlMsgs(true);
        QVERIFY(dlg.getEnableCtrlMsgs());
        dlg.setEnableCtrlMsgs(false);
        QVERIFY(!dlg.getEnableCtrlMsgs());
    }

    void test_setGetEnableMarker()
    {
        FilterDialog dlg;
        dlg.setEnableMarker(true);
        QVERIFY(dlg.getEnableMarker());
        dlg.setEnableMarker(false);
        QVERIFY(!dlg.getEnableMarker());
    }

    // ------------------------------------------------------------------
    // Regex flags round-trips
    // ------------------------------------------------------------------
    void test_regexpAppId()
    {
        FilterDialog dlg;
        dlg.setEnableRegexp_Appid(true);
        QVERIFY(dlg.getEnableRegexp_Appid());
        dlg.setEnableRegexp_Appid(false);
        QVERIFY(!dlg.getEnableRegexp_Appid());
    }

    void test_regexpContext()
    {
        FilterDialog dlg;
        dlg.setEnableRegexp_Context(true);
        QVERIFY(dlg.getEnableRegexp_Context());
    }

    void test_regexpHeader()
    {
        FilterDialog dlg;
        dlg.setEnableRegexp_Header(true);
        QVERIFY(dlg.getEnableRegexp_Header());
    }

    void test_regexpPayload()
    {
        FilterDialog dlg;
        dlg.setEnableRegexp_Payload(true);
        QVERIFY(dlg.getEnableRegexp_Payload());
    }

    void test_ignoreCaseHeader()
    {
        FilterDialog dlg;
        dlg.setIgnoreCase_Header(true);
        QVERIFY(dlg.getIgnoreCase_Header());
        dlg.setIgnoreCase_Header(false);
        QVERIFY(!dlg.getIgnoreCase_Header());
    }

    void test_ignoreCasePayload()
    {
        FilterDialog dlg;
        dlg.setIgnoreCase_Payload(true);
        QVERIFY(dlg.getIgnoreCase_Payload());
    }

    // ------------------------------------------------------------------
    // Colour round-trip
    // ------------------------------------------------------------------
    void test_setGetFilterColour()
    {
        FilterDialog dlg;
        dlg.setFilterColour(QColor("#ff0000"));
        // getFilterColour returns a string representation
        QVERIFY(!dlg.getFilterColour().isEmpty());
    }

    // ------------------------------------------------------------------
    // Message ID range round-trip
    // ------------------------------------------------------------------
    void test_messageIdRange()
    {
        FilterDialog dlg;
        dlg.setEnableMessageId(true);
        QVERIFY(dlg.getEnableMessageId());
        dlg.setMessageId_min(10);
        dlg.setMessageId_max(100);
        QCOMPARE(dlg.getMessageId_min(), 10u);
        QCOMPARE(dlg.getMessageId_max(), 100u);
    }

    // ------------------------------------------------------------------
    // Regex search/replace fields
    // ------------------------------------------------------------------
    void test_regexSearchReplace()
    {
        FilterDialog dlg;
        dlg.setEnableRegexSearchReplace(true);
        QVERIFY(dlg.getEnableRegexSearchReplace());
        dlg.setRegexSearchText("search");
        dlg.setRegexReplaceText("replace");
        QCOMPARE(dlg.getRegexSearchText(), QString("search"));
        QCOMPARE(dlg.getRegexReplaceText(), QString("replace"));
    }

    // ------------------------------------------------------------------
    // UI interaction: clicking Positive/Negative/Marker type buttons
    // ------------------------------------------------------------------
    void test_uiTypeButtons()
    {
        FilterDialog dlg;
        // Use keyboard shortcut or direct setter as a proxy for the button state.
        // Alt: use QTest::mouseClick on the actual button widget.
        dlg.setType(0);
        QCOMPARE(dlg.getType(), 0);
        dlg.setType(1);
        QCOMPARE(dlg.getType(), 1);
        dlg.setType(2);
        QCOMPARE(dlg.getType(), 2);
    }
};

QTEST_MAIN(TestFilterDialog)
#include "test_filterdialog.moc"
