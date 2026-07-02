#include "dltfileindexerdefaultfilterthread.h"

DltFileIndexerDefaultFilterThread::DltFileIndexerDefaultFilterThread
(
        QDltDefaultFilter *defaultFilter,
        QDltPluginManager *pluginManager,
    QDltFile *dltFile,
    CDecodeCacheService *decodeCacheService,
        bool silentMode
)
    : defaultFilter(defaultFilter),
      pluginManager(pluginManager),
      dltFile(dltFile),
      decodeCacheService(decodeCacheService),
      silentMode(silentMode),
      msgQueue(1024)
{}

DltFileIndexerDefaultFilterThread::~DltFileIndexerDefaultFilterThread()
{}

void DltFileIndexerDefaultFilterThread::enqueueMessage(const QSharedPointer<QDltMsg> &msg, int index)
{
    msgQueue.enqueueMsg(msg, index);
}

void DltFileIndexerDefaultFilterThread::requestStop()
{
    msgQueue.enqueueStopRequest();
}

void DltFileIndexerDefaultFilterThread::run()
{
    QPair<QSharedPointer<QDltMsg>, int> msgPair;

    while(msgQueue.dequeue(msgPair))
        processMessage(msgPair.first, msgPair.second);
}

void DltFileIndexerDefaultFilterThread::processMessage(QSharedPointer<QDltMsg> &msg, int index)
{
    /* Process all decoderplugins */
    if (decodeCacheService && dltFile)
    {
        QDltMsg decoded;
        if (decodeCacheService->message(dltFile,
                                        pluginManager,
                                        index,
                                        true,
                                        silentMode,
                                        decoded,
                                        true))
        {
            *msg = decoded;
        }
    }

    /* run through all default filter */
    for(int num = 0; num < defaultFilter->defaultFilterList.size(); num++)
        if(defaultFilter->defaultFilterList[num]->checkFilter(*msg)) // if filter matches message...
            defaultFilter->defaultFilterIndex[num]->indexFilter.append(index); // ... add message to index cache
}
