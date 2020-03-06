#include "dltfileindexer.h"
#include "dltfileindexerthread.h"
#include "dltfileindexerdefaultfilterthread.h"

#include <QDebug>
#include <QMessageBox>
#include <QApplication>
#include <QTime>
#include <QCryptographicHash>
#include <QMutexLocker>


extern "C" {
    #include "dlt_common.h"
    #include "dlt_user.h"
}

DltFileIndexerKey::DltFileIndexerKey(time_t time,unsigned int microseconds,unsigned int timestamp)
{
    this->time = time;
    this->microseconds = microseconds;
    this->timestamp = timestamp;
}

DltFileIndexer::DltFileIndexer(QObject *parent) :
    QThread(parent)
{
    mode = modeIndexAndFilter;
    this->dltFile = NULL;
    this->pluginManager = NULL;
    defaultFilter = NULL;
    stopFlag = 0;

    pluginsEnabled = true;
    filtersEnabled = true;
    multithreaded = true;
    sortByTimeEnabled = false;
    sortByTimestampEnabled = false;

    maxRun = 0;
    currentRun = 0;
    msecsIndexCounter = 0;
    msecsFilterCounter = 0;
    msecsDefaultFilterCounter = 0;
}

DltFileIndexer::DltFileIndexer(QDltFile *dltFile, QDltPluginManager *pluginManager, QDltDefaultFilter *defaultFilter, QMainWindow *parent) :
    QThread(parent)
{
    mode = modeIndexAndFilter;
    this->dltFile = dltFile;
    this->pluginManager = pluginManager;
    this->defaultFilter = defaultFilter;
    stopFlag = 0;

    pluginsEnabled = true;
    filtersEnabled = true;
    multithreaded = true;
    sortByTimeEnabled = 0;
    sortByTimestampEnabled = 0;
    errors_in_file  = 0;

    maxRun = 0;
    currentRun = 0;
    msecsIndexCounter = 0;
    msecsFilterCounter = 0;
    msecsDefaultFilterCounter = 0;
}

DltFileIndexer::~DltFileIndexer()
{
}

bool DltFileIndexer::index(int num)
{
    QTime time;

    // start performance counter
    time.start();

    // load filter index if enabled
    if(!filterCache.isEmpty() && loadIndexCache(dltFile->getFileName(num)))
    {
        // loading index from filter is succesful
        //qDebug() << "Loaded index cache for file" << dltFile->getFileName(num);
        msecsIndexCounter = time.elapsed();
        return true;
    }

    // prepare indexing
    //dltFile->clearIndex();
    QFile f(dltFile->getFileName(num));

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
    emit(progressText(QString("Indexer %1/%2").arg(currentRun).arg(maxRun)));
    emit(progressMax(f.size()));

    // clear old index
    indexAllList.clear();

    // Go through the segments and create new index
    char lastFound = 0;
    qint64 length;
    qint64 msgindex=0;
    qint64 pos;
    qint64 current_message_pos = 0;
    qint64 next_message_pos = 0;
    int counter_header = 0;
    quint16 message_length = 0;
    qint64 file_size = f.size();
    errors_in_file  = 0;
    char *data = new char[DLT_FILE_INDEXER_SEG_SIZE];
    do
    {
        pos = f.pos();
        length = f.read(data,DLT_FILE_INDEXER_SEG_SIZE);
        for(int num=0;num < length;num++)
        {
            // search length of DLT message
            if(counter_header>0)
            {
                counter_header++;
                if (counter_header==16)
                {
                    // Read low byte of message length
                    message_length = (unsigned char)data[num];
                }
                else if (counter_header==17)
                {
                    // Read high byte of message length
                    counter_header = 0;
                    message_length = (message_length<<8 | ((unsigned char)data[num])) +16;
                    next_message_pos = current_message_pos + message_length;
                    if(next_message_pos==file_size)
                    {
                        // last message found in file
                        indexAllList.append(current_message_pos);
                        break;
                    }
                    // speed up move directly to next message, if inside current buffer
                    if((message_length > 20))
                    {
                        if((num+message_length-20<length))
                        {
                            num+=message_length-20;
                        }
                    }
                }
            }
            // find DLT Header
            else if(data[num] == 'D')
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
                if(next_message_pos == 0)
                {
                    // first message detected or first message after error
                    current_message_pos = pos+num-3;
                    counter_header = 1;
                    if(current_message_pos!=0)
                    {
                        // first messages not at beginning or error occured before
                        errors_in_file++;
                        qDebug() << "ERROR in file detected at index"<< msgindex << "msg length" << message_length << "file position" << current_message_pos;
                        qDebug() << "------------";
                    }
                    // speed up move directly to message length, if inside current buffer
                    if(num+14<length)
                    {
                        num+=14;
                        counter_header+=14;
                    }
                }
                else if( next_message_pos == (pos+num-3) )
                {
                    // Add message only when it is in the correct position in relationship to the last message
                    indexAllList.append(current_message_pos);
                    msgindex++;
                    current_message_pos = pos+num-3;
                    counter_header = 1;
                    // speed up move directly to message length, if inside current buffer
                    if(num+14<length)
                    {
                        num+=14;
                        counter_header+=14;
                    }
                }
                else if(next_message_pos > (pos+num-3))
                {
                    // Header detected before end of message
                     qDebug() << "ERROR: Header detected before end of message at index "<< msgindex << "msg length" << message_length << "file position" << current_message_pos;
                     errors_in_file++;
                }
                else
                {
                    // Header detected after end of message
                    // start search for new message back after last header found
                    qDebug() << "Header detected after end of message, offset:" << (pos+num-3) - next_message_pos;
                    f.seek(current_message_pos+4);
                    pos = current_message_pos+4;
                    length = f.read(data,DLT_FILE_INDEXER_SEG_SIZE);
                    num=0;
                    next_message_pos = 0;
                }
                lastFound = 0;
            }
            else
            {
                lastFound = 0; // go on with search for the startsequence
            }

            /* stop if requested */
            if(stopFlag)
            {
                delete[] data;
                f.close();
                return false;
            }
        } // end of for loop to read within one segment

        emit(progress(pos));
    }
    while(length>0);

    // delete buffer
    delete[] data;

    // close file
    f.close();

    if ( errors_in_file != 0 )
    {
    qDebug() << "Indexing error:" << errors_in_file << "wrong DLT message headers found during indexing" << msgindex << "messages";
    }
    else
    {
    qDebug() << "Created index for file" << dltFile->getFileName(num);
    }

    // update performance counter
    msecsIndexCounter = time.elapsed();

    // write index if enabled
    if(false == filterCache.isEmpty())
    {
        saveIndexCache(dltFile->getFileName(num));
        qDebug() << "Saved index cache for file" << dltFile->getFileName(num) << filterCache;
    }

    return true;
}

bool DltFileIndexer::indexFilter(QStringList filenames)
{
    QSharedPointer<QDltMsg> msg;
    QDltFilterList filterList;
    QTime time;
    //qDebug() << "DltFileIndexer::indexFilter" << __FILE__ << __LINE__;
    // start performance counter
    time.start();

    // get filter list
    filterList = dltFile->getFilterList();

    // load filter index, if enabled and not an initial loading of file
    if(!filterCache.isEmpty() && mode != modeIndexAndFilter && loadFilterIndexCache(filterList,indexFilterList,filenames))
    {
        // loading filter index from filter is succesful
        qDebug() << "Loaded filter index cache for files" << filenames;
        msecsFilterCounter = time.elapsed();
        return true;
    }

    // Initialise progress bar
    emit(progressText(QString("%1/%2").arg(currentRun).arg(maxRun)));
    emit(progressMax(dltFile->size()));

    // clear index filter
    indexFilterList.clear();
    indexFilterListSorted.clear();
    getLogInfoList.clear();

    // get silent mode
    bool silentMode = !QDltOptManager::getInstance()->issilentMode();

    bool hasPlugins = (activeDecoderPlugins.size() + activeViewerPlugins.size()) > 0;
    bool hasFilters = filterList.filters.size() > 0;

    bool useIndexerThread = hasPlugins || hasFilters;

    DltFileIndexerThread indexerThread
            (
                this,
                &filterList,
                sortByTimeEnabled,
                sortByTimestampEnabled,
                &indexFilterList,
                &indexFilterListSorted,
                pluginManager,
                &activeViewerPlugins,
                silentMode
            );

    if(useIndexerThread)
        indexerThread.start(); // thread starts reading its queue

    // Start reading messages
    for(int ix=0;ix<dltFile->size();ix++)
    {
        msg = QSharedPointer<QDltMsg>::create(); // create new instance to be filled by getMsg(), otherwise shared pointer would be empty or pointing to last message

        if(!dltFile->getMsg(ix, *msg))
            continue; // Skip broken messages

        if(useIndexerThread)
            indexerThread.enqueueMessage(msg, ix);
        else
            indexerThread.processMessage(msg, ix);

        // Update progress
        if(ix % 1000 == 0)
        {
            emit(progress(ix));
        }

        // stop if requested
        if(stopFlag)
        {
            if(useIndexerThread)
            {
                indexerThread.requestStop();
                indexerThread.wait();
            }

            return false;
        }
    }

    // destroy threads
    if(useIndexerThread)
    {
        indexerThread.requestStop();
        indexerThread.wait();
    }

    //qDebug() << "Created filter index for files" << filenames;

    // update performance counter
    msecsFilterCounter = time.elapsed();

    // use sorted values if sort by time enabled
    if(sortByTimeEnabled || sortByTimestampEnabled)
        indexFilterList = QVector<qint64>::fromList(indexFilterListSorted.values());

    // write filter index if enabled
    if(!filterCache.isEmpty())
    {
        saveFilterIndexCache(filterList, indexFilterList, filenames);
        //qDebug() << "Saved filter index cache for files" << filenames;
    }

    return true;
}

bool DltFileIndexer::indexDefaultFilter()
{
    QSharedPointer<QDltMsg> msg;
    QTime time;

    // start performance counter
    time.start();

    // Initialise progress bar
    emit(progressText(QString("%1/%2").arg(currentRun).arg(maxRun)));
    emit(progressMax(dltFile->size()));

    // clear all default filter cache index
    defaultFilter->clearFilterIndex();

    // get silent mode
    bool silentMode = !QDltOptManager::getInstance()->issilentMode();

    bool useDefaultFilterThread = defaultFilter->defaultFilterList.size() > 0;

    DltFileIndexerDefaultFilterThread defaultFilterThread
            (
                defaultFilter,
                pluginManager,
                silentMode
            );

    if(useDefaultFilterThread)
        defaultFilterThread.start();

    /* run through the whole open file */
    for(int ix = 0; ix < dltFile->size(); ix++)
    {
        msg = QSharedPointer<QDltMsg>::create();
        /* Fill message from file */
        if(!dltFile->getMsg(ix, *msg))
        {
            /* Skip broken messages */
            continue;
        }

        if(useDefaultFilterThread)
            defaultFilterThread.enqueueMessage(msg, ix);
        else
            defaultFilterThread.processMessage(msg, ix);

        /* Update progress */
        if(ix % 1000 == 0)
        {
            emit(progress(ix));
        }

        /* stop if requested */
        if(stopFlag)
        {
            if(useDefaultFilterThread)
            {
                defaultFilterThread.requestStop();
                defaultFilterThread.wait();
            }

            return false;
        }
    }

    if(useDefaultFilterThread)
    {
        defaultFilterThread.requestStop();
        defaultFilterThread.wait();
    }

    /* update plausibility checks of filter index cache, filename and filesize */
    for(int num=0; num < defaultFilter->defaultFilterIndex.size(); num++)
    {
        QDltFilterIndex *filterIndex;
        QDltFilterList *filterList;
        filterIndex = defaultFilter->defaultFilterIndex[num];
        filterList = defaultFilter->defaultFilterList[num];

        filterIndex->setDltFileName(dltFile->getFileName());
        filterIndex->setAllIndexSize(dltFile->size());

        // write filter index if enabled
        if(!filterCache.isEmpty())
            saveFilterIndexCache(*filterList, filterIndex->indexFilter, QStringList(dltFile->getFileName()));
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

void DltFileIndexer::appendToGetLogInfoList(int value)
{
    getLogInfoList.append(value);
}

void DltFileIndexer::run()
{
    //qDebug() << "DltFileIndexer::run" << __FILE__ << __LINE__;
    // lock mutex while indexing
    QMutexLocker scopedLock(&indexLock);

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
        maxRun = dltFile->getNumberOfFiles()+1;
    else
        maxRun = 1;

    currentRun = 1;

    // index
    if(mode == modeIndexAndFilter)
    {
        for(int num=0;num < dltFile->getNumberOfFiles();num++)
        {
            if(!index(num))
            {
                qDebug() << "Error in indexer" << __FILE__ << __LINE__;
                return;
            }
           // qDebug() << "setDLTIndex" << num << __FILE__ << __LINE__;
            dltFile->setDltIndex(indexAllList,num);
            currentRun++;
        }
        //qDebug() << "emit(finishIndex());" << __FILE__ << __LINE__;
        emit(finishIndex());
    }
    else if(mode == modeNone)
    {
        // only update view
        emit(finishIndex());
    }

    // indexFilter
    if(mode == modeIndexAndFilter || mode == modeFilter)
    {
        QStringList filenames;
        for(int num=0;num<dltFile->getNumberOfFiles();num++)
            filenames.append(QFileInfo(dltFile->getFileName(num)).fileName());
        if((mode != modeNone) && !indexFilter(filenames))
        {
            // error
            return;
        }
        dltFile->enableFilter(filtersEnabled);
        dltFile->setIndexFilter(indexFilterList);
        emit(finishFilter());
    }

    // indexDefaultFilter
    if(mode == modeDefaultFilter)
    {
        if(false == indexDefaultFilter())
        {
            // error
            return;
        }
        emit(finishDefaultFilter());
    }

    //qDebug() << "Indexing done" << __FILE__ <<  __LINE__;
    // print performance counter
    /*
    QTime time;
    time = QTime(0,0);time = time.addMSecs(msecsIndexCounter);
    qDebug() << "Duration Indexing:" << time.toString("hh:mm:ss.zzz") << "msecs";
    time = QTime(0,0);time = time.addMSecs(msecsFilterCounter);
    qDebug() << "Duration Filter Indexing:" << time.toString("hh:mm:ss.zzz") << "msecs";
    time = QTime(0,0);time = time.addMSecs(msecsDefaultFilterCounter);
    qDebug() << "Duration Default Filter Indexing:" << time.toString("hh:mm:ss.zzz") << "msecs";
    */
}

void DltFileIndexer::stop()
{
    // stop the thread
    stopFlag = true;
    wait();
    //qDebug() << "Indexer stopped";
}

// load/safe index from/to file
bool DltFileIndexer::loadIndexCache(QString filename)
{
    QString filenameCache;

    // check if caching is enabled
    if(filterCache.isEmpty())
        return false;

    // get the filename for the cache file
    filenameCache = filenameIndexCache(filename);

    // load the cache file
    if(!loadIndex(filterCache + "/" +filenameCache,indexAllList))
    {
        // loading cache file failed
        return false;
    }

    return true;
}

bool DltFileIndexer::saveIndexCache(QString filename)
{
    QString filenameCache;

    // check if caching is enabled
    if(filterCache.isEmpty())
        return false;

    // get the filename for the cache file
    filenameCache = filenameIndexCache(filename);

    // save the cache file
    if(!saveIndex(filterCache + "/" +filenameCache,indexAllList))
    {
        // saving cache file failed
        return false;
    }

    return true;
}

QString DltFileIndexer::filenameIndexCache(QString filename)
{
    QString hashString;
    QByteArray hashByteArray;
    QByteArray md5;
    QString filenameCache;

    // create string to be hashed
    hashString = QFileInfo(filename).fileName();
    hashString += "_" + QString("%1").arg(dltFile->fileSize());

    // create byte array from hash string
    hashByteArray = hashString.toLatin1();

    // create MD5 from byte array
    md5 = QCryptographicHash::hash(hashByteArray, QCryptographicHash::Md5);

    // create filename
    filenameCache = QString(md5.toHex())+".dix";

    //qDebug() << filename << ">>" << filenameCache;

    return filenameCache;
}

// read/write index cache
bool DltFileIndexer::loadFilterIndexCache(QDltFilterList &filterList, QVector<qint64> &index, QStringList filenames)
{
    QString filenameCache;

    // check if caching is enabled
    if(filterCache.isEmpty())
        return false;

    // get the filename for the cache file
    filenameCache = filenameFilterIndexCache(filterList,filenames);

    // load the cache file
    if(loadIndex(filterCache + "/" +filenameCache,index))
    {
        qDebug() << "loadIndex" << filterCache + "/" +filenameCache << "success";
    }
    else
    {
        qDebug() << "loadIndex" << filterCache + "/" +filenameCache << "failed";
        return false;
    }

    return true;
}

bool DltFileIndexer::saveFilterIndexCache(QDltFilterList &filterList, QVector<qint64> index, QStringList filenames)
{
    QString filename;

    // check if caching is enabled
    if(filterCache.isEmpty())
        return false;

    // get the filename for the cache file
    filename = filenameFilterIndexCache(filterList,filenames);

    // save the cache file
    if(!saveIndex(filterCache + "/" +filename,index))
    {
        // saving of cache file failed
        return false;
    }

    return true;
}

QByteArray DltFileIndexer::md5ActiveDecoderPlugins()
{
    QByteArray md5;
    QString hashString = "Plugins";
    QByteArray hashByteArray;

    // walk through all active decoder plugins and generate String with all plugin names, version and loaded filename
    for(int num=0;num<activeDecoderPlugins.size();num++)
    {
        QDltPlugin *plugin = activeDecoderPlugins[num];

        hashString += plugin->getName();
        hashString += plugin->getPluginVersion();
        hashString += plugin->getFilename();
    }
    hashByteArray = hashString.toLatin1();

    // calculate hash value
    md5 = QCryptographicHash::hash(hashByteArray, QCryptographicHash::Md5);

    return md5;
}

QString DltFileIndexer::filenameFilterIndexCache(QDltFilterList &filterList,QStringList filenames)
{
    QString hashString;
    QByteArray hashByteArray;
    QByteArray md5;
    QByteArray md5FilterList;
    QString filename;

    // get filter list
    md5FilterList = filterList.createMD5();

    // create string to be hashed
    if(sortByTimeEnabled || sortByTimestampEnabled)
        filenames.sort();
    hashString = filenames.join(QString("_"));
    hashString += "_" + QString("%1").arg(dltFile->fileSize());

    // create byte array from hash string
    hashByteArray = hashString.toLatin1();

    // create MD5 from byte array
    md5 = QCryptographicHash::hash(hashByteArray, QCryptographicHash::Md5);

    // create filename
    filename = QString(md5.toHex()) + "_" + QString(md5FilterList.toHex());
    if(this->pluginsEnabled)
    {
        filename += "_" + QString(md5ActiveDecoderPlugins().toHex());
    }
    if(this->sortByTimeEnabled)
    {
        filename += "_S";
    }
    if(this->sortByTimestampEnabled)
    {
        filename += "_STS";
    }
    filename += ".dix";

    //qDebug() << filenames << ">>" << filename;

    return filename;
}

bool DltFileIndexer::saveIndex(QString filename, const QVector<qint64> &index)
{
    quint32 version = DLT_FILE_INDEXER_FILE_VERSION;
    qint64 value;

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

bool DltFileIndexer::loadIndex(QString filename, QVector<qint64> &index)
{
    quint32 version;
    qint64 value;
    int length;

    QFile file(filename);

    index.clear();

    // open cache file
    if(!file.open(QFile::ReadOnly))
    {
        // open file failed
        return false;
    }

    index.reserve(static_cast<int>((file.size() - sizeof(version)) / sizeof(value))); // prevent memory issues through reallocation

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
        if(length==sizeof(value))
            index.append(value);
    }
    while(length==sizeof(value));

    // close cache file
    file.close();

    return true;
}


qint64 DltFileIndexer::getfileerrors(void)
{
    return errors_in_file;
}
