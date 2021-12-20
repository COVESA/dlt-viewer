#ifndef DLTMSGQUEUE_H
#define DLTMSGQUEUE_H

#include <QSemaphore>
#include <QSharedPointer>

#include "qdltmsg.h"

class DltMsgQueue
{
public:
    DltMsgQueue(int bufferSize);
    ~DltMsgQueue();
    void enqueueMsg(const QSharedPointer<QDltMsg> &msg, int index);
    bool dequeue(QPair<QSharedPointer<QDltMsg>, int> &dequeuedData);
    void enqueueStopRequest();

private:
    int bufferSize;
    QPair<QSharedPointer<QDltMsg>, int> *buffer;
    QAtomicInt readPosition, writePosition;
    volatile bool stopRequested;
    int writeSleepTime; // Microseconds to sleep if buffer is full during a write attempt
    int readSleepTime; // Microseconds to sleep if buffer is empty during a read attempt

    const int maxSleepTime = 1000 * 10;
    const int sleepTimeSteps = 10;
};

#endif // DLTMSGQUEUE_H
