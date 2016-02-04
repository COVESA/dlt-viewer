#ifndef DLTFILEINDEXERDEFAULTFILTERTHREAD_H
#define DLTFILEINDEXERDEFAULTFILTERTHREAD_H

#include "dltfileindexer.h"
#include "dltmsgqueue.h"
#include <QThread>

class DltFileIndexerDefaultFilterThread :public QThread
{
    Q_OBJECT
public:
    DltFileIndexerDefaultFilterThread(QDltDefaultFilter *defaultFilter, QDltPluginManager *pluginManager, bool silentMode);
    ~DltFileIndexerDefaultFilterThread();
    void enqueueMessage(const QSharedPointer<QDltMsg> &msg, int index);
    void processMessage(QSharedPointer<QDltMsg> &msg, int index);
    void requestStop();

protected:
    void run();

private:
    QDltDefaultFilter *defaultFilter;
    QDltPluginManager *pluginManager;
    bool silentMode;

    DltMsgQueue msgQueue;
};

#endif // DLTFILEINDEXERDEFAULTFILTERTHREAD_H
