#include "dltfileindexer.h"
#include "optmanager.h"

#include <QDebug>
#include <QMessageBox>
#include <QApplication>
#include <QTime>
#include <QCryptographicHash>

extern "C" {
    #include "dlt_common.h"
    #include "dlt_user.h"
}

DltFileIndexer::DltFileIndexer(QObject *parent) :
    QThread(parent)
{
    this->dltFile = 0;
    this->pluginManager = 0;

    pluginsEnabled = true;
    filtersEnabled = true;
    multithreaded = true;

    mode = modeIndexAndFilter;
}


DltFileIndexer::DltFileIndexer(QDltFile *dltFile, QDltPluginManager *pluginManager, QDltDefaultFilter *defaultFilter, QMainWindow *parent) :
    QThread(parent)
{
    this->dltFile = dltFile;
    this->pluginManager = pluginManager;
    this->defaultFilter = defaultFilter;

    pluginsEnabled = true;
    filtersEnabled = true;
    multithreaded = true;

    mode = modeIndexAndFilter;
}

DltFileIndexer::~DltFileIndexer()
{
}

bool DltFileIndexer::index()
{
    QTime time;

    // start performance counter
    time.start();

    // load filter index if enabled
    if(!filterCache.isEmpty() && loadIndexCache())
    {
        // loading index from filter is succesful
        msecsIndexCounter = time.elapsed();
        return true;
    }

    // prepare indexing
    dltFile->clearIndex();
    QFile f(dltFile->getFileName());

    // open file
    if(!f.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file in DltFileIndexer " << f.errorString();
        return false;
    }

    // check if file is empty
    if(f.size() == 0)
    {
        // No need to do anything here.
        f.close();
        return true;
    }

    // Initialise progress bar
    emit(progressText(QString("%1/%2").arg(currentRun).arg(maxRun)));
    emit(progressMax(f.size()));

    // clear old index
    indexAllList.clear();

    // Go through the segments and create new index
    char lastFound = 0;
    qint64 length;
    qint64 pos;
    char *data = new char[DLT_FILE_INDEXER_SEG_SIZE];
    do
    {

        pos = f.pos();
        length = f.read(data,DLT_FILE_INDEXER_SEG_SIZE);
        for(int num=0;num < length;num++)
        {
            if(data[num] == 'D')
            {
                lastFound = 'D';
            }
            else if(lastFound == 'D' && data[num] == 'L')
            {
                lastFound = 'L';
            }
            else if(lastFound == 'L' && data[num] == 'T')
            {
                lastFound = 'T';
            }
            else if(lastFound == 'T' && data[num] == 0x01)
            {
                indexAllList.append(pos+num-3);
                lastFound = 0;
            }
            else
            {
                lastFound = 0;
            }

            /* stop if requested */
            if(stopFlag)
            {
                delete data;
                return false;
            }
        }
        emit(progress(pos));
    }
    while(length>0);

    // delete buffer
    delete data;

    // close file
    f.close();

    // update performance counter
    msecsIndexCounter = time.elapsed();

    // write index if enabled
    if(!filterCache.isEmpty())
        saveIndexCache();

    return true;
}

bool DltFileIndexer::indexFilter()
{
    QDltMsg msg;
    QDltPlugin *item;
    QDltFilterList filterList;
    QTime time;

    // start performance counter
    time.start();

    // get filter list
    filterList = dltFile->getFilterList();

    // load filter index, if enabled and not an initial loading of file
    if(!filterCache.isEmpty() && mode != modeIndexAndFilter && loadFilterIndexCache(filterList,indexFilterList))
    {
        // loading filter index from filter is succesful
        msecsFilterCounter = time.elapsed();
        return true;
    }

    // Initialise progress bar
    emit(progressText(QString("%1/%2").arg(currentRun).arg(maxRun)));
    emit(progressMax(dltFile->size()));

    // clear index filter
    indexFilterList.clear();

    // get silent mode
    bool silentMode = !OptManager::getInstance()->issilentMode();

    // run through all viewer plugins
    // must be run in the UI thread, if some gui actions are performed

    // run through all DLT messages
    for(int ix=0;ix<dltFile->size();ix++)
    {
        /* Fill message from file */
        if(!dltFile->getMsg(ix, msg))
        {
            /* Skip broken messages */
            continue;
        }

        /* check if it is a version messages and
           version string not already parsed */
        if((mode == modeIndex || mode == modeIndexAndFilter) &&
           msg.getType()==QDltMsg::DltTypeControl &&
           msg.getSubtype()==QDltMsg::DltControlResponse &&
           msg.getCtrlServiceId() == DLT_SERVICE_ID_GET_SOFTWARE_VERSION)
        {
            QByteArray payload = msg.getPayload();
            QByteArray data = payload.mid(9,(payload.size()>262)?256:(payload.size()-9));
            QString version = msg.toAscii(data,true);
            version = version.trimmed(); // remove all white spaces at beginning and end
            versionString(msg.getEcuid(),version);
        }

        /* check if it is a timezone message */
        if((mode == modeIndex || mode == modeIndexAndFilter) &&
           msg.getType()==QDltMsg::DltTypeControl &&
           msg.getSubtype()==QDltMsg::DltControlResponse &&
           msg.getCtrlServiceId() == DLT_SERVICE_ID_TIMEZONE)
        {
            QByteArray payload = msg.getPayload();
            if(payload.size() == sizeof(DltServiceTimezone))
            {
                DltServiceTimezone *service;
                service = (DltServiceTimezone*) payload.constData();

                if(msg.getEndianness() == QDltMsg::DltEndiannessLittleEndian)
                    timezone(service->timezone,service->isdst);
                else
                    timezone(DLT_SWAP_32(service->timezone),service->isdst);
            }
        }

        /* check if it is a timezone message */
        if((mode == modeIndex || mode == modeIndexAndFilter) &&
           msg.getType()==QDltMsg::DltTypeControl &&
           msg.getSubtype()==QDltMsg::DltControlResponse &&
           msg.getCtrlServiceId() == DLT_SERVICE_ID_UNREGISTER_CONTEXT)
        {
            QByteArray payload = msg.getPayload();
            if(payload.size() == sizeof(DltServiceUnregisterContext))
            {
                DltServiceUnregisterContext *service;
                service = (DltServiceUnregisterContext*) payload.constData();

                unregisterContext(msg.getEcuid(),QString(QByteArray(service->apid,4)),QString(QByteArray(service->ctid,4)));
            }

        }

        /* Process all viewer plugins */
        if((mode == modeIndex || mode == modeIndexAndFilter) && pluginsEnabled)
        {
            for(int ivp=0;ivp < activeViewerPlugins.size();ivp++)
            {
                item = (QDltPlugin*)activeViewerPlugins.at(ivp);
                item->initMsg(ix, msg);
            }
        }

        /* Process all decoderplugins */
        pluginManager->decodeMsg(msg,silentMode);

        /* Add to filterindex if matches */
        if(filtersEnabled && filterList.checkFilter(msg))
        {
            indexFilterList.append(ix);
        }

        /* Offer messages again to viewer plugins after decode */
        if(pluginsEnabled)
        {
            for(int ivp=0;ivp<activeViewerPlugins.size();ivp++)
            {
                item = (QDltPlugin *)activeViewerPlugins.at(ivp);
                item->initMsgDecoded(ix, msg);
            }
        }

        /* update context configuration when loading file */
        if( (mode == modeIndex || mode == modeIndexAndFilter) &&
            msg.getType()==QDltMsg::DltTypeControl &&
            msg.getSubtype()==QDltMsg::DltControlResponse )
        {
            const char *ptr;
            int32_t length;
            uint32_t service_id=0, service_id_tmp=0;

            QByteArray payload = msg.getPayload();
            ptr = payload.constData();
            length = payload.size();
            DLT_MSG_READ_VALUE(service_id_tmp,ptr,length,uint32_t);
            service_id=DLT_ENDIAN_GET_32( ((msg.getEndianness()==QDltMsg::DltEndiannessBigEndian)?DLT_HTYP_MSBF:0), service_id_tmp);

            if(service_id == DLT_SERVICE_ID_GET_LOG_INFO)
            {
                emit(getLogInfo(ix));
            }
        }

        /* Update progress every 0.5% */
        if( 0 == (ix%1000))
        {
            emit(progress(ix));
        }

        /* stop if requested */
        if(stopFlag)
        {
            return false;
        }
    }

    // run through all viewer plugins
    // must be run in the UI thread, if some gui actions are performed

    // update performance counter
    msecsFilterCounter = time.elapsed();

    // write filter index if enabled
    if(!filterCache.isEmpty())
        saveFilterIndexCache(filterList,indexFilterList);

    return true;
}

bool DltFileIndexer::indexDefaultFilter()
{
    QDltMsg msg;
    QTime time;

    // start performance counter
    time.start();

    // Initialise progress bar
    emit(progressText(QString("%1/%2").arg(currentRun).arg(maxRun)));
    emit(progressMax(dltFile->size()));

    // clear all default filter cache index
    defaultFilter->clearFilterIndex();

    // get silent mode
    bool silentMode = !OptManager::getInstance()->issilentMode();

    /* run through the whole open file */
    for(int ix=0;ix<dltFile->size();ix++)
    {
        /* Fill message from file */
        if(!dltFile->getMsg(ix, msg))
        {
            /* Skip broken messages */
            continue;
        }

        /* Process all decoderplugins */
        pluginManager->decodeMsg(msg,silentMode);

        /* run through all default filter */
        for(int num=0;num<defaultFilter->defaultFilterList.size();num++)
        {
            QDltFilterList *filterList;
            filterList = defaultFilter->defaultFilterList[num];

            /* check if filter matches message */
            if(filterList->checkFilter(msg))
            {
                QDltFilterIndex *filterIndex;
                filterIndex = defaultFilter->defaultFilterIndex[num];

                /* if filter match add message to index cache */
                filterIndex->indexFilter.append(ix);

            }
        }

        /* Update progress every 0.5% */
        if( 0 == (ix%1000))
        {
            emit(progress(ix));
        }

        /* stop if requested */
        if(stopFlag)
        {
            return false;
        }
    }

    /* update plausibility checks of filter index cache, filename and filesize */
    for(int num=0;num<defaultFilter->defaultFilterIndex.size();num++)
    {
        QDltFilterIndex *filterIndex;
        QDltFilterList *filterList;
        filterIndex = defaultFilter->defaultFilterIndex[num];
        filterList = defaultFilter->defaultFilterList[num];

        filterIndex->setDltFileName(dltFile->getFileName());
        filterIndex->setAllIndexSize(dltFile->size());

        // write filter index if enabled
        if(!filterCache.isEmpty())
            saveFilterIndexCache(*filterList,filterIndex->indexFilter);
    }

    // update performance counter
    msecsDefaultFilterCounter = time.elapsed();

    return true;
}


void DltFileIndexer::lock()
{
    indexLock.lock();
}

void DltFileIndexer::unlock()
{
    indexLock.unlock();
}

bool DltFileIndexer::tryLock()
{
    return indexLock.tryLock();
}

void DltFileIndexer::run()
{
    // initialise stop flag
    stopFlag = false;

    // clear performance counter
    msecsIndexCounter = 0;
    msecsFilterCounter = 0;
    msecsDefaultFilterCounter = 0;

    // get all active plugins
    activeViewerPlugins = pluginManager->getViewerPlugins();
    activeDecoderPlugins = pluginManager->getDecoderPlugins();

    // calculate runs
    if(mode == modeIndexAndFilter)
        maxRun = 2;
    else
        maxRun = 1;
    currentRun = 1;

    // index
    if(mode == modeIndex || mode == modeIndexAndFilter)
    {
        if(!index())
        {
            // error
            return;
        }
        dltFile->setDltIndex(indexAllList);
        emit(finishIndex());
        currentRun++;
    }

    // indexFilter
    if(mode == modeIndexAndFilter || mode == modeFilter || mode == modeNone)
    {
        if((mode != modeNone) && !indexFilter())
        {
            // error
            return;
        }
        dltFile->setIndexFilter(indexFilterList);
        emit(finishFilter());
    }

    // indexDefaultFilter
    if(mode == modeDefaultFilter)
    {
        if(!indexDefaultFilter())
        {
            // error
            return;
        }
        emit(finishDefaultFilter());
    }

    // print performance counter
    QTime time;
    time = QTime(0,0);time = time.addMSecs(msecsIndexCounter);
    qDebug() << "Duration Indexing:" << time.toString("hh:mm:ss.zzz") << "msecs";
    time = QTime(0,0);time = time.addMSecs(msecsFilterCounter);
    qDebug() << "Duration Filter Indexing:" << time.toString("hh:mm:ss.zzz") << "msecs";
    time = QTime(0,0);time = time.addMSecs(msecsDefaultFilterCounter);
    qDebug() << "Duration Default Filter Indexing:" << time.toString("hh:mm:ss.zzz") << "msecs";

}

void DltFileIndexer::stop()
{
    // stop the thread
    stopFlag = true;

    while(isRunning())
    {
        // wait until task is not running anymore
        usleep(100000);
    }
}

// load/safe index from/to file
bool DltFileIndexer::loadIndexCache()
{
    QString filename;

    // check if caching is enabled
    if(filterCache.isEmpty())
        return false;

    // get the filename for the cache file
    filename = filenameIndexCache();

    // load the cache file
    if(!loadIndex(filterCache + "\\" +filename,indexAllList))
    {
        // loading cache file failed
        qDebug() << "Load Index from file" << filterCache + "\\" +filename << "failed";
        return false;
    }

    qDebug() << "Loaded Index from file" << filterCache + "\\" +filename;

    return true;
}

bool DltFileIndexer::saveIndexCache()
{
    QString filename;

    // check if caching is enabled
    if(filterCache.isEmpty())
        return false;

    // get the filename for the cache file
    filename = filenameIndexCache();

    // save the cache file
    if(!saveIndex(filterCache + "\\" +filename,indexAllList))
    {
        // saving cache file failed
        qDebug() << "Save Index to file" << filterCache + "\\" +filename << "failed";
        return false;
    }

    qDebug() << "Saved Index to file" << filterCache + "\\" +filename;

    return true;
}

QString DltFileIndexer::filenameIndexCache()
{
    QString hashString;
    QByteArray hashByteArray;
    QByteArray md5;
    QString filename;

    // create string to be hashed
    hashString = QFileInfo(dltFile->getFileName()).baseName();
    hashString += "_" + QString("%1").arg(dltFile->fileSize());
    qDebug() << "hashString:" << hashString;

    // create byte array from hash string
    hashByteArray = hashString.toLatin1();
    qDebug() << "hashByteArray:" << hashByteArray.toHex();

    // create MD5 from byte array
    md5 = QCryptographicHash::hash(hashByteArray, QCryptographicHash::Md5);
    qDebug() << "md5:" << md5.toHex();

    // create filename
    filename = QString(md5.toHex())+".dix";
    qDebug() << "filename:" << filename;

    return filename;
}

// read/write index cache
bool DltFileIndexer::loadFilterIndexCache(QDltFilterList &filterList, QList<unsigned long> &index)
{
    QString filename;

    // check if caching is enabled
    if(filterCache.isEmpty())
        return false;

    // get the filename for the cache file
    filename = filenameFilterIndexCache(filterList);

    // load the cache file
    if(!loadIndex(filterCache + "\\" +filename,index))
    {
        // loading of cache file failed
        qDebug() << "Load Filter Index from file" << filterCache + "\\" +filename << "failed";
        return false;
    }

    qDebug() << "Loaded Filter Index from file" << filterCache + "\\" +filename;

    return true;
}

bool DltFileIndexer::saveFilterIndexCache(QDltFilterList &filterList, QList<unsigned long> &index)
{
    QString filename;

    // check if caching is enabled
    if(filterCache.isEmpty())
        return false;

    // get the filename for the cache file
    filename = filenameFilterIndexCache(filterList);

    // save the cache file
    if(!saveIndex(filterCache + "\\" +filename,index))
    {
        // saving of cache file failed
        qDebug() << "Save Filter Index to file" << filterCache + "\\" +filename << "failed";
        return false;
    }

    qDebug() << "Saved Filter Index to file" << filterCache + "\\" +filename;

    return true;
}

QString DltFileIndexer::filenameFilterIndexCache(QDltFilterList &filterList)
{
    QString hashString;
    QByteArray hashByteArray;
    QByteArray md5;
    QByteArray md5FilterList;
    QString filename;

    // get filter list
    md5FilterList = filterList.createMD5();
    qDebug() << "md5FilterList:" << md5FilterList.toHex();

    // create string to be hashed
    hashString = QFileInfo(dltFile->getFileName()).baseName();
    hashString += "_" + QString("%1").arg(dltFile->fileSize());
    qDebug() << "hashString:" << hashString;

    // create byte array from hash string
    hashByteArray = hashString.toLatin1();
    qDebug() << "hashByteArray:" << hashByteArray.toHex();

    // create MD5 from byte array
    md5 = QCryptographicHash::hash(hashByteArray, QCryptographicHash::Md5);
    qDebug() << "md5:" << md5.toHex();

    // create filename
    filename = QString(md5.toHex())+"_"+QString(md5FilterList.toHex())+".dix";
    qDebug() << "filename:" << filename;

    return filename;
}

bool DltFileIndexer::saveIndex(QString filename, QList<unsigned long> &index)
{
    quint32 version = DLT_FILE_INDEXER_FILE_VERSION;
    unsigned long value;

    QFile file(filename);

    // open cache file
    if(!file.open(QFile::WriteOnly))
    {
        // open file failed
        return false;
    }

    // write version
    file.write((char*)&version,sizeof(version));

    // write complete index
    for(int num=0;num<index.size();num++)
    {
        value = index[num];
        file.write((char*)&value,sizeof(value));
    }

    // close cache file
    file.close();

    return true;
}

bool DltFileIndexer::loadIndex(QString filename, QList<unsigned long> &index)
{
    quint32 version;
    unsigned long value;
    int length;

    QFile file(filename);

    index.clear();

    // open cache file
    if(!file.open(QFile::ReadOnly))
    {
        // open file failed
        return false;
    }

    // read version
    length = file.read((char*)&version,sizeof(version));

    // compare version if valid
    if((length != sizeof(version)) || version != DLT_FILE_INDEXER_FILE_VERSION)
    {
        // wrong version number
        file.close();
        return false;
    }

    // read complete index
    do
    {
        length = file.read((char*)&value,sizeof(value));
        index.append(value);
    }
    while(length==sizeof(value));

    // close cache file
    file.close();

    return true;
}
