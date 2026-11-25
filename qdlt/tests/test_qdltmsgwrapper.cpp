#include <gtest/gtest.h>

#include <qdltmsgwrapper.h>
#include <qdltmsg.h>

#include <utility>

TEST(QDltMsgWrapper, uint32Serialization) {
    uint32_t value = 0x12345678;
    QDltMsgWrapper wrapper(value);
    DltMessage& msg = wrapper.getMessage();

    ASSERT_EQ(msg.datasize, sizeof(uint32_t));
    ASSERT_NE(msg.databuffer, nullptr);
    uint32_t extractedValue = 0;
    memcpy(&extractedValue, msg.databuffer, sizeof(uint32_t));
    ASSERT_EQ(extractedValue, value);
}

TEST(QDltMsgWrapper, structWithContainerSerialization) {
    struct PACKED TestStruct {
        uint16_t field1;
        uint8_t field2;
    };

    TestStruct testStruct = {0xABCD, 0xEF};
    std::vector<uint8_t> container = {0x01, 0x02, 0x03, 0x04, 0x05};

    QDltMsgWrapper wrapper(testStruct, container);
    DltMessage& msg = wrapper.getMessage();

    ASSERT_EQ(msg.datasize, sizeof(TestStruct) + sizeof(uint32_t) + container.size());
    ASSERT_NE(msg.databuffer, nullptr);

    uint32_t offset = 0;
    TestStruct extractedStruct;
    memcpy(&extractedStruct, msg.databuffer + offset, sizeof(TestStruct));
    offset += sizeof(TestStruct);

    uint32_t containerSize = 0;
    memcpy(&containerSize, msg.databuffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    ASSERT_EQ(extractedStruct.field1, testStruct.field1);
    ASSERT_EQ(extractedStruct.field2, testStruct.field2);
    ASSERT_EQ(containerSize, container.size());

    std::vector<uint8_t> extractedContainer(containerSize);
    memcpy(extractedContainer.data(), msg.databuffer + offset, containerSize);
    ASSERT_EQ(extractedContainer, container);
}

TEST(QDltMsg, liveLogLevelIncludesAllMoreSevereLogs) {
    QDltMsg message;
    message.setType(QDltMsg::DltTypeLog);
    for(int maximumLevel = QDltMsg::DltLogOff; maximumLevel <= QDltMsg::DltLogVerbose; ++maximumLevel) {
        for(unsigned char severity = QDltMsg::DltLogOff; severity <= QDltMsg::DltLogVerbose; ++severity) {
            message.setSubtype(severity);
            EXPECT_EQ(message.matchesLogLevel(maximumLevel),
                      severity >= QDltMsg::DltLogFatal && severity <= maximumLevel)
                << "selected=" << maximumLevel << ", incoming=" << static_cast<int>(severity);
        }
    }
}

TEST(QDltMsg, fatalWithoutMatchingLogsStaysIdleAndOtherLevelsResume) {
    QDltMsg messages[5];
    messages[0].setType(QDltMsg::DltTypeLog);
    messages[0].setSubtype(QDltMsg::DltLogInfo);
    messages[1].setType(QDltMsg::DltTypeNwTrace);
    messages[1].setSubtype(QDltMsg::DltNetworkTraceIpc);
    messages[2].setType(QDltMsg::DltTypeLog);
    messages[2].setSubtype(QDltMsg::DltLogError);
    messages[3].setType(QDltMsg::DltTypeLog);
    messages[3].setSubtype(QDltMsg::DltLogWarn);
    messages[4].setType(QDltMsg::DltTypeControl);
    messages[4].setSubtype(QDltMsg::DltControlResponse);

    const std::pair<int, int> cases[] = {
        {QDltMsg::DltLogFatal, 0}, {QDltMsg::DltLogError, 1},
        {QDltMsg::DltLogWarn, 2}, {QDltMsg::DltLogInfo, 3},
        {QDltMsg::DltLogOff, 0}, {QDltMsg::DltLogVerbose, 3},
        {QDltMsg::DltLogDefault, 5}
    };
    for(const auto &testCase : cases) {
        int accepted = 0;
        for(const auto &message : messages) {
            if(message.matchesLogLevel(testCase.first)) {
                ++accepted;
            }
        }
        EXPECT_EQ(accepted, testCase.second) << "selected=" << testCase.first;
    }
}
