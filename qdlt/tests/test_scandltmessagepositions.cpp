#include "qdltfile.h"
#include <gtest/gtest.h>
#include <QFile>
#include <QTemporaryFile>

TEST(ScanDltMessagePositionsTest, EmptyFile) {
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    QVector<qint64> positions = QDltFile::scanDltMessagePositions(&file);
    EXPECT_EQ(positions.size(), 0);
}

TEST(ScanDltMessagePositionsTest, SingleMessage) {
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    QByteArray msg = QByteArray::fromHex("444C5401" + QByteArray(12, '\0').toHex() + "0005"); // DLT0x01 header + padding + length
    file.write(msg);
    file.seek(0);
    QVector<qint64> positions = QDltFile::scanDltMessagePositions(&file);
    ASSERT_EQ(positions.size(), 1);
    EXPECT_EQ(positions[0], 0LL);
}

TEST(ScanDltMessagePositionsTest, MultipleMessages) {
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    QByteArray msg1 = QByteArray::fromHex("444C5401" + QByteArray(12, '\0').toHex() + "0005");
    QByteArray msg2 = QByteArray::fromHex("444C5401" + QByteArray(12, '\0').toHex() + "0005");
    file.write(msg1);
    file.write(msg2);
    file.seek(0);
    QVector<qint64> positions = QDltFile::scanDltMessagePositions(&file);
    ASSERT_EQ(positions.size(), 2);
    EXPECT_EQ(positions[0], 0LL);
    EXPECT_EQ(positions[1], (qint64)msg1.size());
}
