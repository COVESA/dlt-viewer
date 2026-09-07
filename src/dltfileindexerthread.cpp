//#include <QDebug>
#include "dltfileindexerthread.h"
#include "dlt_protocol.h"
#include "dlt_common.h"

DltFileIndexerThread::DltFileIndexerThread
(
        DltFileIndexer *indexer,
        QDltFilterList *filterList,
        bool sortByTimeEnabled,
        bool sortByTimestampEnabled,
        QVector<qint64> *indexFilterList,
        QMap<DltFileIndexerKey,qint64> *indexFilterListSorted,
        QDltPluginManager *pluginManager,
        QList<QDltPlugin*> *activeViewerPlugins,
        QList<QDltPlugin*> *activeDecoderPlugins,
        bool silentMode
)
    :indexer(indexer),
      filterList(filterList),
      sortByTimeEnabled(sortByTimeEnabled),
      sortByTimestampEnabled(sortByTimestampEnabled),
      indexFilterList(indexFilterList),
      indexFilterListSorted(indexFilterListSorted),
      pluginManager(pluginManager),
      activeViewerPlugins(activeViewerPlugins),
      activeDecoderPlugins(activeDecoderPlugins),
      silentMode(silentMode),
      filterNeedsDecodedText(filterList ? filterList->needsDecodedText() : false),
      msgQueue(1024)
{

}

DltFileIndexerThread::~DltFileIndexerThread()
{

}

void DltFileIndexerThread::enqueueMessage(const QSharedPointer<QDltMsg> &msg, int index)
{
    msgQueue.enqueueMsg(msg, index);
}

void DltFileIndexerThread::requestStop()
{
    msgQueue.enqueueStopRequest();
}

void DltFileIndexerThread::run()
{
    QPair<QSharedPointer<QDltMsg>, int> msgPair;
    while(msgQueue.dequeue(msgPair))
        processMessage(*msgPair.first, msgPair.second);
}

void DltFileIndexerThread::processMessage(QSharedPointer<QDltMsg> &msg, int index)
{
    processMessage(*msg, index);
}

void DltFileIndexerThread::processMessage(QDltMsg &msg, int index)
{
    DltFileIndexer::IndexingMode mode = indexer->getMode();
    bool pluginsEnabled = indexer->getPluginsEnabled();
    QDltPlugin *item;
    bool bool_result = false;

    /* check if it is a version messages and
    version string not already parsed */
    if((mode == DltFileIndexer::modeIndexAndFilter) &&
       msg.getType() == QDltMsg::DltTypeControl &&
       msg.getSubtype() == QDltMsg::DltControlResponse &&
       msg.getCtrlServiceId() == DLT_SERVICE_ID_GET_SOFTWARE_VERSION)
    {
        QByteArray payload = msg.getPayload();
        QByteArray data = payload.mid(9, (payload.size() > 262) ? 256 : (payload.size() - 9));
        QString version = QDlt::toAscii(data,true);
        version = version.trimmed(); // remove all white spaces at beginning and end
        indexer->versionString(msg.getEcuid(),version);
    }

    /* check if it is a timezone message */
    if((mode == DltFileIndexer::modeIndexAndFilter) &&
       msg.getType() == QDltMsg::DltTypeControl &&
       msg.getSubtype() == QDltMsg::DltControlResponse &&
       msg.getCtrlServiceId() == DLT_SERVICE_ID_TIMEZONE)
    {
        QByteArray payload = msg.getPayload();
        if(payload.size() == sizeof(DltServiceTimezone))
        {
            DltServiceTimezone *service;
            service = (DltServiceTimezone*) payload.constData();

            if(msg.getEndianness() == QDlt::DltEndiannessLittleEndian)
                indexer->timezone(service->timezone, service->isdst);
            else
                indexer->timezone(DLT_SWAP_32(service->timezone), service->isdst);
        }
    }

    /* check if it is a timezone message */
    if((mode == DltFileIndexer::modeIndexAndFilter) &&
       msg.getType()==QDltMsg::DltTypeControl &&
       msg.getSubtype()==QDltMsg::DltControlResponse &&
       msg.getCtrlServiceId() == DLT_SERVICE_ID_UNREGISTER_CONTEXT)
    {
        QByteArray payload = msg.getPayload();
        if(payload.size() == sizeof(DltServiceUnregisterContext))
        {
            DltServiceUnregisterContext *service;
            service = (DltServiceUnregisterContext *) payload.constData();

            indexer->unregisterContext(msg.getEcuid(), QDltMsg::getStringFromId(service->apid), QDltMsg::getStringFromId(service->ctid));
        }
    }

    /* Process all viewer plugins */
    if((mode == DltFileIndexer::modeIndexAndFilter) && pluginsEnabled)
    {
        for(int ivp = 0; ivp < activeViewerPlugins->size(); ivp++)
        {
            item = (QDltPlugin *) activeViewerPlugins->at(ivp);
            item->initMsg(index, msg);
        }
    }

    /*
     * In modeFilter (pure CFI rebuild) skip decoder plugins only when no active filter
     * inspects decoded header/payload text - otherwise CFI would filter on raw undecoded
     * bytes and silently produce wrong matches for content-based filters.
     */
    if (pluginsEnabled &&
        ((mode == DltFileIndexer::modeIndexAndFilter) || filterNeedsDecodedText))
    /*
     * In modeFilter (pure CFI rebuild) we only need filter matching and index generation.
     * Running decoder plugins here is expensive and can dominate runtime on large files.
     */
    if ((mode == DltFileIndexer::modeIndexAndFilter) && pluginsEnabled)
     {
     (void) pluginManager->decodeMsg(msg, silentMode);
     }

    /* Process all decoder plugins.
     * Use the pre-fetched active decoder list to avoid taking pluginManager's
     * mutex once per message in the CFI hot loop. */
    if (pluginsEnabled && activeDecoderPlugins)
    {
        for (int idp = 0; idp < activeDecoderPlugins->size(); ++idp)
        {
            QDltPlugin *decoder = activeDecoderPlugins->at(idp);
            if(decoder && decoder->decodeMsg(msg, silentMode))
                break;
        }
    }

    bool_result = filterList->checkFilter(msg);
    if ( bool_result == true)
    {
        if(const QDltFilter *markerFilter = filterList->matchMarkerFilter(msg); markerFilter != nullptr)
        {
            indexer->addMarkerCount(markerFilter->name);
        }

        if(sortByTimeEnabled)
         {
            indexFilterListSorted->insert(DltFileIndexerKey(msg.getTime(), msg.getMicroseconds(), index), index);
         }
        else if(sortByTimestampEnabled)
         {
            indexFilterListSorted->insert(DltFileIndexerKey(msg.getTimestamp(), index), index);
         }
        else
         {
            indexFilterList->append(index);
         }
    }

    /* Offer messages again to viewer plugins after decode */
    if((mode == DltFileIndexer::modeIndexAndFilter) && pluginsEnabled)
    {
        for(int ivp = 0; ivp < activeViewerPlugins->size(); ivp++)
        {
            item = (QDltPlugin *) activeViewerPlugins->at(ivp);
            item->initMsgDecoded(index, msg);
        }
    }

    /* update context configuration when loading file */
    if((mode == DltFileIndexer::modeIndexAndFilter) &&
        msg.getType() == QDltMsg::DltTypeControl &&
        msg.getSubtype() == QDltMsg::DltControlResponse)
    {
        const char *ptr;
        int32_t length;
        uint32_t service_id=0, service_id_tmp=0;

        QByteArray payload = msg.getPayload();
        ptr = payload.constData();
        length = payload.size();
        DLT_MSG_READ_VALUE(service_id_tmp,ptr, length, uint32_t);
        service_id=DLT_ENDIAN_GET_32(((msg.getEndianness() == QDlt::DltEndiannessBigEndian) ? DLT_HTYP_MSBF:0), service_id_tmp);

        if(service_id == DLT_SERVICE_ID_GET_LOG_INFO)
        {
            indexer->appendToGetLogInfoList(index);
        }
    }
}
