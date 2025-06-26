#include <gtest/gtest.h>

#include <qdltargument.h>

TEST(QDltArgument, constructor) {
    QDltArgument arg;

    ASSERT_EQ(arg.getTypeInfo(), QDltArgument::DltTypeInfoUnknown);
    ASSERT_EQ(arg.getTypeInfoString(), QString{});
    ASSERT_EQ(arg.getDataSize(), 0);
    ASSERT_EQ(arg.toString(), QString("?")); // a bit weird for data size 0
    ASSERT_EQ(arg.getValue(), QVariant());
}

TEST(QDltArgument, string_ascii) {
    QDltArgument arg;
    // parse from raw payload
    const unsigned char payloadData[] = {0x00, 0x02, 0x00, 0x00, // type info
                                       0x04, 0x00,             // str len
                                       0xc3, 0xbc, 'b',  'e'};
    QByteArray payload = QByteArray::fromRawData((const char*)payloadData, sizeof(payloadData));
    unsigned int offset = 0;
    arg.setArgument(payload, offset, QDlt::DltEndiannessLittleEndian);
    ASSERT_EQ(arg.getTypeInfo(), QDltArgument::DltTypeInfoStrg);
    ASSERT_EQ(arg.getDataSize(), 4);
    // argument type is DltTypeInfoStrg (aka ASCII), the data is interpreted as UTF-8 encoded
    ASSERT_EQ(arg.toString(), QString::fromUtf8("übe"));
    ASSERT_EQ(arg.toString(true), QString{"c3 bc 62 65"});
    ASSERT_EQ(arg.getValue(), QVariant(QString::fromUtf8("übe")));
}

TEST(QDltArgument, string_utf8) {
    QDltArgument arg;
    arg.setTypeInfo(QDltArgument::DltTypeInfoUtf8);
    const unsigned char asc_data[] = {
        0xc3, 0xbc, 'b', 'e' // übe
    };
    arg.setData(QByteArray::fromRawData((const char*)asc_data, sizeof(asc_data)));
    ASSERT_EQ(arg.toString(), QString::fromUtf8((const char*)asc_data, sizeof(asc_data)));

    // parse from raw payload
    const unsigned char asc_data2[] = {0x00, 0x82, 0x00, 0x00, // type info
                                       0x04, 0x00,             // str len
                                       0xc3, 0xbc, 'b',  'e'};
    QByteArray payload = QByteArray::fromRawData((const char*)asc_data2, sizeof(asc_data2));
    unsigned int offset = 0;
    arg.setArgument(payload, offset, QDlt::DltEndiannessLittleEndian);
    ASSERT_EQ(arg.getTypeInfo(), QDltArgument::DltTypeInfoUtf8);
    ASSERT_EQ(arg.getDataSize(), 4);
    ASSERT_EQ(arg.toString().length(), 3);
    ASSERT_EQ(arg.toString(), QString::fromUtf8("übe"));
    ASSERT_EQ(arg.toString(true), QString{"c3 bc 62 65"});
    ASSERT_EQ(arg.getValue(), QVariant(QString::fromUtf8("übe")));
}

TEST(QDltArgument, setValueString) {
    QDltArgument arg;
    ASSERT_EQ(arg.setValue(QVariant(QString{"übe"})), true);
    // this should be a type utf8 string and not ascii:
    ASSERT_EQ(arg.getTypeInfo(), QDltArgument::DltTypeInfoUtf8);
    ASSERT_EQ(arg.getDataSize(), 4);
    ASSERT_EQ(arg.toString(), QString::fromUtf8("übe"));
    ASSERT_EQ(arg.getValue(), QVariant(QString::fromUtf8("übe")));
}
