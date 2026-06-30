#include "indexthreadworker.h"

#include "decodemanager.h"
#include "qdltpluginmanager.h"

#include <QFile>
#include <QMutexLocker>

namespace {

struct ScannedMessage
{
    int globalIndex{-1};
    QByteArray rawMessage;
};

QVector<qint64> scanNewMessagePositions(QFile &file, qint64 lastIndexedPosition)
{
    QByteArray buf;
    qint64 pos = 0;
    quint16 lastMessageLength = 0;
    quint8 version = 1;
    qint64 lengthOffset = 2;
    qint64 storageLength = 0;
    QVector<qint64> newPositions;

    if(lastIndexedPosition >= 0)
    {
        pos = lastIndexedPosition;
        file.seek(pos);
        buf = file.read(14);
        if(buf.size() < 7)
        {
            return newPositions;
        }

        if(static_cast<unsigned char>(buf.at(3)) == 2)
        {
            storageLength = 14 + static_cast<unsigned char>(buf.at(13));
        }
        else
        {
            storageLength = 16;
        }

        file.seek(pos + storageLength);
        buf = file.read(7);
        if(buf.size() < 7)
        {
            return newPositions;
        }

        version = (static_cast<unsigned char>(buf.at(0)) & 0xe0) >> 5;
        lengthOffset = (version == 2) ? 5 : 2;
        lastMessageLength = static_cast<unsigned char>(buf.at(lengthOffset));
        lastMessageLength = (lastMessageLength << 8 |
                             static_cast<unsigned char>(buf.at(lengthOffset + 1))) + storageLength;

        pos += (lastMessageLength - 1);
        file.seek(pos);
    }
    else
    {
        file.seek(0);
    }

    static const int readBufSize = 1024 * 1024;
    char lastFound = 0;
    qint64 currentMessagePos = 0;
    qint64 nextMessagePos = 0;
    int counterHeader = 0;
    quint16 messageLength = 0;
    const qint64 fileSize = file.size();

    while(true)
    {
        buf = file.read(readBufSize);
        if(buf.isEmpty())
        {
            break;
        }

        int cbufSize = buf.size();
        const char *cbuf = buf.constData();

        for(int num = 0; num < cbufSize; ++num)
        {
            if(counterHeader > 0)
            {
                ++counterHeader;
                if(storageLength == 13 && counterHeader == 13)
                {
                    storageLength += static_cast<unsigned char>(cbuf[num]) + 1;
                }
                else if(counterHeader == storageLength)
                {
                    version = (static_cast<unsigned char>(cbuf[num]) & 0xe0) >> 5;
                    if(version == 1)
                    {
                        lengthOffset = 2;
                    }
                    else if(version == 2)
                    {
                        lengthOffset = 5;
                    }
                    else
                    {
                        lengthOffset = 2;
                    }
                }
                else if(counterHeader == (storageLength + lengthOffset))
                {
                    messageLength = static_cast<unsigned char>(cbuf[num]);
                }
                else if(counterHeader == (storageLength + 1 + lengthOffset))
                {
                    counterHeader = 0;
                    messageLength = (messageLength << 8 | static_cast<unsigned char>(cbuf[num])) + storageLength;
                    nextMessagePos = currentMessagePos + messageLength;
                    if(nextMessagePos == fileSize)
                    {
                        newPositions.append(currentMessagePos);
                        break;
                    }
                    if((messageLength > storageLength + 2 + lengthOffset) &&
                       (num + messageLength - (storageLength + 2 + lengthOffset) < cbufSize))
                    {
                        num += messageLength - (storageLength + 2 + lengthOffset);
                    }
                }
            }
            else if(cbuf[num] == 'D')
            {
                lastFound = 'D';
            }
            else if(lastFound == 'D' && cbuf[num] == 'L')
            {
                lastFound = 'L';
            }
            else if(lastFound == 'L' && cbuf[num] == 'T')
            {
                lastFound = 'T';
            }
            else if(lastFound == 'T' && (cbuf[num] == 0x01 || cbuf[num] == 0x02))
            {
                if(nextMessagePos == 0)
                {
                    currentMessagePos = pos + num - 3;
                    counterHeader = 3;
                    storageLength = (cbuf[num] == 0x01) ? 16 : 13;
                    if(num + 9 < cbufSize)
                    {
                        num += 9;
                        counterHeader += 9;
                    }
                }
                else if(nextMessagePos == (pos + num - 3))
                {
                    newPositions.append(currentMessagePos);
                    currentMessagePos = pos + num - 3;
                    counterHeader = 3;
                    storageLength = (cbuf[num] == 0x01) ? 16 : 13;
                    if(num + 9 < cbufSize)
                    {
                        num += 9;
                        counterHeader += 9;
                    }
                }
                else if(nextMessagePos <= (pos + num - 3))
                {
                    file.seek(currentMessagePos + 4);
                    pos = currentMessagePos + 4;
                    buf = file.read(readBufSize);
                    cbufSize = buf.size();
                    cbuf = buf.constData();
                    num = 0;
                    nextMessagePos = 0;
                }
                lastFound = 0;
            }
            else
            {
                lastFound = 0;
            }
        }

        pos += cbufSize;
    }

    return newPositions;
}

QVector<ScannedMessage> readNewMessages(QFile &file,
                                        const IndexThreadFileInput &fileInput,
                                        const QVector<qint64> &newPositions)
{
    QVector<ScannedMessage> messages;
    messages.reserve(newPositions.size());

    for(int idx = 0; idx < newPositions.size(); ++idx)
    {
        const qint64 start = newPositions.at(idx);
        const qint64 end = (idx + 1 < newPositions.size()) ? newPositions.at(idx + 1) : file.size();
        if(end <= start)
        {
            continue;
        }

        if(!file.seek(start))
        {
            continue;
        }

        ScannedMessage message;
        message.globalIndex = fileInput.baseGlobalIndex + fileInput.existingMessageCount + idx;
        message.rawMessage = file.read(end - start);
        messages.push_back(message);
    }

    return messages;
}

} // namespace

IndexThreadWorker::IndexThreadWorker(QDltPluginManager *pluginManager, QObject *parent)
    : QThread(parent),
      m_pluginManager(pluginManager)
{
}

IndexThreadWorker::~IndexThreadWorker()
{
    stopWorker();
    wait();
}

void IndexThreadWorker::enqueueBatch(QVector<IndexThreadFileInput> &&files,
                                     const IndexThreadBatchContext &context)
{
    if(files.isEmpty())
    {
        return;
    }

    PendingBatch batch;
    batch.files = std::move(files);
    batch.context = context;

    {
        QMutexLocker lock(&m_queueMutex);
        m_pendingBatches.enqueue(std::move(batch));
    }

    m_waitCondition.wakeOne();
}

void IndexThreadWorker::stopWorker()
{
    {
        QMutexLocker lock(&m_queueMutex);
        m_stopRequested = true;
        m_pendingBatches.clear();
    }

    m_waitCondition.wakeAll();
}

void IndexThreadWorker::run()
{
    while(true)
    {
        PendingBatch batch;

        {
            QMutexLocker lock(&m_queueMutex);

            while(!m_stopRequested && m_pendingBatches.isEmpty())
            {
                m_waitCondition.wait(&m_queueMutex);
            }

            if(m_stopRequested)
            {
                break;
            }

            batch = m_pendingBatches.dequeue();
        }

        emit batchProcessed(processBatch(batch, m_pluginManager));
    }
}

IndexThreadBatchResult IndexThreadWorker::processBatch(const PendingBatch &batch,
                                                       QDltPluginManager *pluginManager)
{
    IndexThreadBatchResult result;
    result.pluginsEnabled = batch.context.pluginsEnabled;

    for(const IndexThreadFileInput &fileInput : batch.files)
    {
        QFile file(fileInput.fileName);
        if(!file.open(QIODevice::ReadOnly))
        {
            continue;
        }

        const QVector<qint64> newPositions = scanNewMessagePositions(file, fileInput.lastIndexedPosition);
        if(newPositions.isEmpty())
        {
            continue;
        }

        IndexThreadFileResult fileResult;
        fileResult.fileIndex = fileInput.fileIndex;
        fileResult.newPositions = newPositions;
        result.fileUpdates.push_back(fileResult);

        const QVector<ScannedMessage> scannedMessages = readNewMessages(file, fileInput, newPositions);
        result.items.reserve(result.items.size() + scannedMessages.size());
        result.matchingIndices.reserve(result.matchingIndices.size() + scannedMessages.size());

        for(const ScannedMessage &input : scannedMessages)
        {
            IndexThreadBatchResultItem item;
            item.index = input.globalIndex;

            if(input.rawMessage.isEmpty())
            {
                result.items.push_back(item);
                continue;
            }

            if(!item.undecodedMsg.setMsg(input.rawMessage, true, batch.context.dltv2Support))
            {
                result.items.push_back(item);
                continue;
            }

            item.undecodedMsg.setIndex(input.globalIndex);
            item.decodedMsg = item.undecodedMsg;

            DecodeManager::instance().decode(pluginManager,
                                             item.decodedMsg,
                                             batch.context.pluginsEnabled,
                                             batch.context.silentMode);

            item.passFilter = !batch.context.filtersEnabled || batch.context.filterList.checkFilter(item.decodedMsg);
            item.valid = true;

            if(item.passFilter)
            {
                result.matchingIndices.push_back(input.globalIndex);
            }

            result.items.push_back(item);
        }
    }

    return result;
}
