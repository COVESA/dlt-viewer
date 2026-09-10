#include "dltfileindexerdefaultfilterthread.h"

DltFileIndexerDefaultFilterThread::DltFileIndexerDefaultFilterThread
(
        QDltDefaultFilter *defaultFilter,
        QDltPluginManager *pluginManager,
        bool silentMode
)
    : defaultFilter(defaultFilter),
      pluginManager(pluginManager),
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
    /* Decode the message already loaded by indexDefaultFilter(). The plugin
     * manager serializes this stage, so re-reading the file through
     * CDecodeCacheService::message() is unnecessary. */
    if (msg && pluginManager)
    {
        pluginManager->decodeMsg(*msg, silentMode);
    }

    /* run through all default filter */
    for(int num = 0; num < defaultFilter->defaultFilterList.size(); num++)
        if(defaultFilter->defaultFilterList[num]->checkFilter(*msg)) // if filter matches message...
            defaultFilter->defaultFilterIndex[num]->indexFilter.append(index); // ... add message to index cache
}
