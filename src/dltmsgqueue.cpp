#include "dltmsgqueue.h"
#include <QThread>

DltMsgQueue::DltMsgQueue(int size)
    : bufferSize(size),
      buffer(new QPair<QSharedPointer<QDltMsg>, int> [size]),
      readPosition(0),
      writePosition(0),
      stopRequested(false),
      writeSleepTime(0),
      readSleepTime(0)
{}

DltMsgQueue::~DltMsgQueue()
{
    if(buffer != nullptr)
        delete[] buffer;
}

void DltMsgQueue::enqueueMsg(const QSharedPointer<QDltMsg> &msg, int index)
{
    int nextWritePosition = (writePosition.load() + 1) % bufferSize;

    while(nextWritePosition == readPosition.load()) // buffer full?
    {
        if(writeSleepTime > 0)
            QThread::currentThread()->usleep(writeSleepTime);

        writeSleepTime = qMin(writeSleepTime + sleepTimeSteps, maxSleepTime);
    }

    writeSleepTime = qMax(writeSleepTime - sleepTimeSteps, 0);

    buffer[writePosition.load()].first = msg;
    buffer[writePosition.load()].second = index;

    writePosition.store(nextWritePosition);
}

bool DltMsgQueue::dequeue(QPair<QSharedPointer<QDltMsg>, int> &dequeuedData)
{
    while(readPosition.load() == writePosition.load()) // buffer empty?
    {
        if(stopRequested)
            return false;

        if(readSleepTime > 0)
            QThread::currentThread()->usleep(readSleepTime);

        readSleepTime = qMin(readSleepTime + sleepTimeSteps, maxSleepTime);
    }

    readSleepTime = qMax(readSleepTime - sleepTimeSteps, 0);

    dequeuedData = buffer[readPosition.load()];

    readPosition.store((readPosition.load() + 1) % bufferSize);

    return true;
}

void DltMsgQueue::enqueueStopRequest()
{
    stopRequested = true;
}

