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

#if QT_5_SUPPORTED_VERSION
    int nextWritePosition = (writePosition.loadRelaxed() + 1) % bufferSize;

    while(nextWritePosition == readPosition.loadRelaxed()) // buffer full?
#else
    int nextWritePosition = (writePosition.load() + 1) % bufferSize;

    while(nextWritePosition == readPosition.load()) // buffer full?
#endif
    {
        if(writeSleepTime > 0)
            QThread::currentThread()->usleep(writeSleepTime);

        writeSleepTime = qMin(writeSleepTime + sleepTimeSteps, maxSleepTime);
    }

    writeSleepTime = qMax(writeSleepTime - sleepTimeSteps, 0);
#if QT_5_SUPPORTED_VERSION
    buffer[writePosition.loadRelaxed()].first = msg;
    buffer[writePosition.loadRelaxed()].second = index;

    writePosition.storeRelaxed(nextWritePosition);
#else
    buffer[writePosition.load()].first = msg;
    buffer[writePosition.load()].second = index;

    writePosition.store(nextWritePosition);
#endif
}

bool DltMsgQueue::dequeue(QPair<QSharedPointer<QDltMsg>, int> &dequeuedData)
{
#if QT_5_SUPPORTED_VERSION
    while(readPosition.loadRelaxed() == writePosition.loadRelaxed()) // buffer empty?
#else
    while(readPosition.load() == writePosition.load()) // buffer empty?
#endif
    {
        if(stopRequested)
            return false;

        if(readSleepTime > 0)
            QThread::currentThread()->usleep(readSleepTime);

        readSleepTime = qMin(readSleepTime + sleepTimeSteps, maxSleepTime);
    }

    readSleepTime = qMax(readSleepTime - sleepTimeSteps, 0);
#if QT_5_SUPPORTED_VERSION
    dequeuedData = buffer[readPosition.loadRelaxed()];

    readPosition.storeRelaxed((readPosition.loadRelaxed() + 1) % bufferSize);
#else
    dequeuedData = buffer[readPosition.load()];

    readPosition.store((readPosition.load() + 1) % bufferSize);
#endif


    return true;
}

void DltMsgQueue::enqueueStopRequest()
{
    stopRequested = true;
}

