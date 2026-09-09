#ifndef INDEXTHREADWORKER_H
#define INDEXTHREADWORKER_H

#include <QMutex>
#include <QSharedPointer>
#include <QThread>
#include <atomic>

#include "dltmsgqueue.h"
#include "project.h"

class QDltFile;
class QDltMsg;
class QDltPlugin;
class QDltPluginManager;
class DecodeManager;

class IndexThreadWorker : public QThread
{
    Q_OBJECT

public:
    explicit IndexThreadWorker(QDltFile *dltFile, QDltPluginManager *pluginManager, DecodeManager *decodeManager, QObject *parent = nullptr);
    ~IndexThreadWorker() override;

    void enqueueIndexUpdateRequest();
    void requestStop();

    void setRuntimeConfig(const QDltFilterList &filterList, bool filtersEnabled, bool pluginsEnabled, bool silentMode);

signals:
    void indexBatchStarted();
    void indexBatchFinished();
    void filterDecision(int index, bool matched, QString markerFilterName);
    void versionStringFound(QString ecuId, QString version);
    void timezoneFound(int timezone, unsigned char dst);
    void unregisterContextFound(QString ecuId, QString appId, QString ctxId);

protected:
    void run() override;

private:
    void processIndexUpdateRequest();
    void processMessage(QSharedPointer<QDltMsg> &msg, int index, const QList<QDltPlugin*> &activeViewerPlugins,
                        bool localFiltersEnabled, bool localPluginsEnabled, bool localSilentMode,
                        QDltFilterList &localFilterList);

    QDltFile *dltFile;
    QDltPluginManager *pluginManager;
    DecodeManager *decodeManager;
    DltMsgQueue msgQueue;

    QMutex configMutex;
    QDltFilterList filterList;
    bool filtersEnabled;
    bool pluginsEnabled;
    bool silentMode;
    std::atomic<int> pendingIndexUpdateRequests;
};

#endif // INDEXTHREADWORKER_H
