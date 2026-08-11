#include "indexthreadworker.h"

#include "decodemanager.h"

#include "dlt_common.h"
#include "dlt_protocol.h"

#include "qdlt.h"
#include "qdltfile.h"
#include "qdltmsg.h"
#include "qdltplugin.h"
#include "qdltpluginmanager.h"

#include <QMutexLocker>

IndexThreadWorker::IndexThreadWorker(QDltFile *dltFile, QDltPluginManager *pluginManager, DecodeManager *decodeManager, QObject *parent)
    : QThread(parent)
    , dltFile(dltFile)
    , pluginManager(pluginManager)
    , decodeManager(decodeManager)
    , msgQueue(1024)
    , filtersEnabled(true)
    , pluginsEnabled(true)
    , silentMode(false)
    , pendingIndexUpdateRequests(0)
{
}

IndexThreadWorker::~IndexThreadWorker()
{
}

void IndexThreadWorker::enqueueIndexUpdateRequest()
{
    const int previous = pendingIndexUpdateRequests.fetch_add(1, std::memory_order_acq_rel);
    if(previous == 0)
    {
        msgQueue.enqueueMsg(QSharedPointer<QDltMsg>(), -1);
    }
}

void IndexThreadWorker::requestStop()
{
    msgQueue.enqueueStopRequest();
}

void IndexThreadWorker::setRuntimeConfig(const QDltFilterList &filterList, bool filtersEnabled, bool pluginsEnabled, bool silentMode)
{
    QMutexLocker locker(&configMutex);
    this->filterList = filterList;
    this->filtersEnabled = filtersEnabled;
    this->pluginsEnabled = pluginsEnabled;
    this->silentMode = silentMode;
}

void IndexThreadWorker::run()
{
    QPair<QSharedPointer<QDltMsg>, int> msgPair;
    while(msgQueue.dequeue(msgPair))
    {
        if(msgPair.second == -1)
        {
            processIndexUpdateRequest();
        }
    }
}

void IndexThreadWorker::processIndexUpdateRequest()
{
    if(dltFile == nullptr)
    {
        pendingIndexUpdateRequests.store(0, std::memory_order_release);
        return;
    }

    for(;;)
    {
        pendingIndexUpdateRequests.store(0, std::memory_order_release);

        QDltFilterList localFilterList;
        bool localFiltersEnabled = true;
        bool localPluginsEnabled = false;
        bool localSilentMode = false;

        {
            QMutexLocker locker(&configMutex);
            localFilterList = filterList;
            localFiltersEnabled = filtersEnabled;
            localPluginsEnabled = pluginsEnabled;
            localSilentMode = silentMode;
        }

        const int oldsize = dltFile->size();
        dltFile->updateIndex();
        const int newsize = dltFile->size();

        if(oldsize < newsize)
        {
            emit indexBatchStarted();

            if(localPluginsEnabled && decodeManager != nullptr)
            {
                decodeManager->refreshDecoderChain();
            }

            const QList<QDltPlugin*> activeViewerPlugins = localPluginsEnabled
                    ? pluginManager->getViewerPlugins()
                    : QList<QDltPlugin*>();

            for(int num = oldsize; num < newsize; ++num)
            {
                QSharedPointer<QDltMsg> msg = QSharedPointer<QDltMsg>::create();
                if(!dltFile->getMsg(num, *msg))
                {
                    continue;
                }

                msg->setIndex(num);
                processMessage(
                    msg,
                    num,
                    activeViewerPlugins,
                    localFiltersEnabled,
                    localPluginsEnabled,
                    localSilentMode,
                    localFilterList);
            }

            emit indexBatchFinished();
        }

        if(pendingIndexUpdateRequests.load(std::memory_order_acquire) == 0)
        {
            break;
        }
    }
}

void IndexThreadWorker::processMessage(
    QSharedPointer<QDltMsg> &msg,
    int index,
    const QList<QDltPlugin*> &activeViewerPlugins,
    bool localFiltersEnabled,
    bool localPluginsEnabled,
    bool localSilentMode,
    QDltFilterList &localFilterList)
{
    if(localPluginsEnabled)
    {
        for(int i = 0; i < activeViewerPlugins.size(); ++i)
        {
            QDltPlugin *item = activeViewerPlugins.at(i);
            if(item != nullptr)
            {
                item->updateMsg(index, *msg);
            }
        }
    }

    if(localPluginsEnabled)
    {
        if(decodeManager != nullptr)
        {
            decodeManager->decodeMsg(*msg, localSilentMode);
        }
        else
        {
            (void) pluginManager->decodeMsg(*msg, localSilentMode);
        }
    }

    const bool matched = localFiltersEnabled ? localFilterList.checkFilter(*msg) : true;
    QString markerFilterName;
    if(matched)
    {
        if(const QDltFilter *markerFilter = localFilterList.matchMarkerFilter(*msg); markerFilter != nullptr)
        {
            markerFilterName = markerFilter->name;
        }
    }

    if(localPluginsEnabled)
    {
        for(int i = 0; i < activeViewerPlugins.size(); ++i)
        {
            QDltPlugin *item = activeViewerPlugins.at(i);
            if(item != nullptr)
            {
                item->updateMsgDecoded(index, *msg);
            }
        }
    }

    if(msg->getType() == QDltMsg::DltTypeControl &&
       msg->getSubtype() == QDltMsg::DltControlResponse &&
       msg->getCtrlServiceId() == DLT_SERVICE_ID_GET_SOFTWARE_VERSION)
    {
        QByteArray payload = msg->getPayload();
        QByteArray data = payload.mid(9, (payload.size() > 262) ? 256 : (payload.size() - 9));
        QString version = QDlt::toAscii(data, true).trimmed();
        emit versionStringFound(msg->getEcuid(), version);
    }

    if(msg->getType() == QDltMsg::DltTypeControl &&
       msg->getSubtype() == QDltMsg::DltControlResponse &&
       msg->getCtrlServiceId() == DLT_SERVICE_ID_TIMEZONE)
    {
        QByteArray payload = msg->getPayload();
        if(payload.size() == sizeof(DltServiceTimezone))
        {
            const DltServiceTimezone *service = reinterpret_cast<const DltServiceTimezone*>(payload.constData());
            if(msg->getEndianness() == QDlt::DltEndiannessLittleEndian)
            {
                emit timezoneFound(service->timezone, service->isdst);
            }
            else
            {
                emit timezoneFound(DLT_SWAP_32(service->timezone), service->isdst);
            }
        }
    }

    if(msg->getType() == QDltMsg::DltTypeControl &&
       msg->getSubtype() == QDltMsg::DltControlResponse &&
       msg->getCtrlServiceId() == DLT_SERVICE_ID_UNREGISTER_CONTEXT)
    {
        QByteArray payload = msg->getPayload();
        if(payload.size() == sizeof(DltServiceUnregisterContext))
        {
            const DltServiceUnregisterContext *service = reinterpret_cast<const DltServiceUnregisterContext*>(payload.constData());
            emit unregisterContextFound(
                msg->getEcuid(),
                QDltMsg::getStringFromId(service->apid),
                QDltMsg::getStringFromId(service->ctid));
        }
    }

    emit filterDecision(index, matched, markerFilterName);
}
