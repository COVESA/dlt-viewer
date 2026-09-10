#include <gtest/gtest.h>

#include <QFile>
#include <QTemporaryFile>

#include <future>
#include <vector>

#include "messagestore.h"
#include "qdltfile.h"

TEST(QDltFile, callerOwnedReaderReadsIndexedMessages)
{
    QTemporaryFile source;
    ASSERT_TRUE(source.open());
    ASSERT_EQ(source.write("first-second", 12), 12);
    source.flush();
    const QString fileName = source.fileName();
    source.close();

    QDltFile file;
    ASSERT_TRUE(file.open(fileName));
    QVector<qint64> index{0, 5};
    file.setDltIndex(index);

    QFile reader;
    EXPECT_EQ(file.getMsg(0, reader), QByteArray("first"));
    EXPECT_EQ(file.getMsg(1, reader), QByteArray("-second"));
    EXPECT_TRUE(file.getMsg(-1, reader).isEmpty());
}

TEST(QDltFile, concurrentCallerOwnedReadersMatchSerialReads)
{
    QTemporaryFile source;
    ASSERT_TRUE(source.open());
    ASSERT_EQ(source.write("first-second-third", 18), 18);
    source.flush();
    const QString fileName = source.fileName();
    source.close();

    QDltFile file;
    ASSERT_TRUE(file.open(fileName));
    QVector<qint64> index{0, 5, 12};
    file.setDltIndex(index);

    std::vector<QByteArray> expected;
    for (int indexValue = 0; indexValue < 3; ++indexValue)
    {
        QFile reader;
        reader.setFileName(fileName);
        ASSERT_TRUE(reader.open(QIODevice::ReadOnly));
        expected.push_back(file.getMsg(indexValue, reader));
    }

    std::vector<std::future<QByteArray>> reads;
    for (int indexValue = 0; indexValue < 3; ++indexValue)
    {
        reads.push_back(std::async(std::launch::async, [&file, fileName, indexValue]() {
            QFile reader;
            reader.setFileName(fileName);
            if (!reader.open(QIODevice::ReadOnly))
                return QByteArray();
            return file.getMsg(indexValue, reader);
        }));
    }

    for (int indexValue = 0; indexValue < 3; ++indexValue)
        EXPECT_EQ(reads[indexValue].get(), expected[indexValue]);
}

TEST(MessageStore, resolvesRawMessagesAndRejectsInvalidIds)
{
    QTemporaryFile source;
    ASSERT_TRUE(source.open());
    ASSERT_EQ(source.write("first-second", 12), 12);
    source.flush();
    const QString fileName = source.fileName();
    source.close();

    QDltFile file;
    ASSERT_TRUE(file.open(fileName));
    QVector<qint64> index{0, 5};
    file.setDltIndex(index);

    CQDltFileMessageStoreAdapter store(&file);
    EXPECT_EQ(store.rawMessage(store.messageIdForGlobalIndex(1)),
              std::vector<char>({'-', 's', 'e', 'c', 'o', 'n', 'd'}));
    EXPECT_EQ(store.messageIdForGlobalIndex(-1), kInvalidMessageId);
    EXPECT_EQ(store.messageIdForGlobalIndex(2), kInvalidMessageId);
    EXPECT_TRUE(store.rawMessage(kInvalidMessageId).empty());
}