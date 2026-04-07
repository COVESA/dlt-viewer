#include <gtest/gtest.h>

#include <qdltmsgwrapper.h>

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
