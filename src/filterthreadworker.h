#ifndef FILTERTHREADWORKER_H
#define FILTERTHREADWORKER_H

#include <QMutex>
#include <QQueue>
#include <QSharedPointer>
#include <QThread>
#include <QVector>
#include <QWaitCondition>
#include <QtGlobal>

#include "qdltfilterlist.h"
#include "qdltmsg.h"

class FilterThreadWorker : public QThread
{
    Q_OBJECT

public:
    explicit FilterThreadWorker(QObject *parent = nullptr);
    ~FilterThreadWorker() override;

    void setFilterConfiguration(const QDltFilterList &filterList, bool filtersEnabled);
    void enqueueMessage(const QSharedPointer<QDltMsg> &msg, int index, quint64 generation);
    void clearPending();
    void stopWorker();

signals:
    void matchesReady(const QVector<qint64> &indices, quint64 generation);

protected:
    void run() override;

private:
    struct PendingMessage
    {
        QSharedPointer<QDltMsg> msg;
        int index = -1;
        quint64 generation = 0;
    };

    QMutex queueMutex;
    QWaitCondition queueCondition;
    QQueue<PendingMessage> queue;
    bool stopRequested;

    // Shared immutable snapshot: swapped (not copied) per message so filter changes only deep-copy once.
    QSharedPointer<QDltFilterList> currentFilterList;
    bool currentFiltersEnabled;

    static const int kBatchSize = 256;
};

#endif // FILTERTHREADWORKER_H
