#include "filterthreadworker.h"

#include <QMutexLocker>

FilterThreadWorker::FilterThreadWorker(QObject *parent)
    : QThread(parent),
      stopRequested(false),
      currentFiltersEnabled(true)
{
}

FilterThreadWorker::~FilterThreadWorker()
{
    stopWorker();
}

void FilterThreadWorker::setFilterConfiguration(const QDltFilterList &filterList, bool filtersEnabled)
{
    // Deep-copy happens here only, i.e. once per filter change, not once per message.
    auto snapshot = QSharedPointer<QDltFilterList>::create(filterList);

    QMutexLocker locker(&queueMutex);
    currentFilterList = snapshot;
    currentFiltersEnabled = filtersEnabled;
}

void FilterThreadWorker::enqueueMessage(const QSharedPointer<QDltMsg> &msg, int index, quint64 generation)
{
    QMutexLocker locker(&queueMutex);

    if(stopRequested)
    {
        return;
    }

    PendingMessage pendingMessage;
    pendingMessage.msg = msg;
    pendingMessage.index = index;
    pendingMessage.generation = generation;

    queue.enqueue(pendingMessage);
    queueCondition.wakeOne();
}

void FilterThreadWorker::clearPending()
{
    QMutexLocker locker(&queueMutex);
    queue.clear();
}

void FilterThreadWorker::stopWorker()
{
    {
        QMutexLocker locker(&queueMutex);
        if(!isRunning())
        {
            return;
        }

        stopRequested = true;
        queueCondition.wakeAll();
    }

    wait();
}

void FilterThreadWorker::run()
{
    QVector<qint64> matchedIndices;
    matchedIndices.reserve(kBatchSize);

    quint64 activeGeneration = 0;
    bool hasActiveGeneration = false;

    while(true)
    {
        PendingMessage pendingMessage;
        QSharedPointer<QDltFilterList> filterListSnapshot;
        bool filtersEnabled = true;
        bool queueIsEmptyAfterPop = false;

        {
            QMutexLocker locker(&queueMutex);
            while(queue.isEmpty() && !stopRequested)
            {
                queueCondition.wait(&queueMutex);
            }

            if(stopRequested && queue.isEmpty())
            {
                break;
            }

            pendingMessage = queue.dequeue();
            filterListSnapshot = currentFilterList; // atomic ref-count bump, no deep copy
            filtersEnabled = currentFiltersEnabled;
            queueIsEmptyAfterPop = queue.isEmpty();
        }

        // const: guarantees the snapshot read under the lock can't be reassigned below
        const QSharedPointer<QDltFilterList> filterList = filterListSnapshot;

        const bool isMatch = !filtersEnabled || !filterList || filterList->checkFilter(*pendingMessage.msg);
        if(!isMatch)
        {
            if(queueIsEmptyAfterPop && !matchedIndices.isEmpty())
            {
                emit matchesReady(matchedIndices, activeGeneration);
                matchedIndices.clear();
                hasActiveGeneration = false;
            }
            continue;
        }

        if(!hasActiveGeneration)
        {
            activeGeneration = pendingMessage.generation;
            hasActiveGeneration = true;
        }
        else if(activeGeneration != pendingMessage.generation)
        {
            if(!matchedIndices.isEmpty())
            {
                emit matchesReady(matchedIndices, activeGeneration);
                matchedIndices.clear();
            }
            activeGeneration = pendingMessage.generation;
        }

        matchedIndices.append(static_cast<qint64>(pendingMessage.index));

        if(matchedIndices.size() >= kBatchSize || queueIsEmptyAfterPop)
        {
            emit matchesReady(matchedIndices, activeGeneration);
            matchedIndices.clear();
            hasActiveGeneration = false;
        }
    }

    if(!matchedIndices.isEmpty() && hasActiveGeneration)
    {
        emit matchesReady(matchedIndices, activeGeneration);
    }

    {
        QMutexLocker locker(&queueMutex);
        stopRequested = false;
        queue.clear();
    }
}
