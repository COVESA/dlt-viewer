#ifndef DLTFILEINDEXER_H
#define DLTFILEINDEXER_H

#include <QObject>
#include <QThread>
#include <QProgressDialog>
#include <QMainWindow>
#include <QPair>
#include <QMutex>

#include "qdlt.h"

#define DLT_FILE_INDEXER_SEG_SIZE (1024*1024)
#define DLT_FILE_INDEXER_FILE_VERSION 2

class DltFileIndexerKey
{
public:
    DltFileIndexerKey(time_t time,unsigned int microseconds);

    friend bool operator< (const DltFileIndexerKey &key1, const DltFileIndexerKey &key2);

private:
    time_t time;
    unsigned int microseconds;
};

inline bool operator< (const DltFileIndexerKey &key1, const DltFileIndexerKey &key2)
{
    if(key1.time<key2.time)
        return true;
    if(key1.time>key2.time)
        return false;
    return (key1.microseconds<key2.microseconds);
}

class DltFileIndexer : public QThread
{
    Q_OBJECT
public:

    // constructors
    explicit DltFileIndexer(QObject *parent = 0);
    DltFileIndexer(QDltFile *argFile, QDltPluginManager *pluginManager, QDltDefaultFilter *defaultFilter, QMainWindow *parent = 0);

    // destructor
    ~DltFileIndexer();

    typedef enum { modeNone, modeIndexAndFilter, modeFilter, modeDefaultFilter } IndexingMode;

    // create main index
    bool index(int num);

    qint64 getfileerrors(void);

    // create index based on filters and apply plugins
    bool indexFilter(QStringList filenames);
    bool indexDefaultFilter();

    // load/save filter index from/to file
    bool loadFilterIndexCache(QDltFilterList &filterList, QVector<qint64> &index, QStringList filenames);
    bool saveFilterIndexCache(QDltFilterList &filterList, QVector<qint64> index, QStringList filenames);
    QString filenameFilterIndexCache(QDltFilterList &filterList, QStringList filenames);
    QByteArray md5ActiveDecoderPlugins(); // generate hash value over all active decoder plugins

    // load/save index from/to file
    bool loadIndexCache(QString filename);
    bool saveIndexCache(QString filename);
    QString filenameIndexCache(QString filename);

    // load/save index from/to file
    bool saveIndex(QString filename, const QVector<qint64> &index);
    bool loadIndex(QString filename, QVector<qint64> &index);

    // Accessors to mutex
    void lock();
    void unlock();
    bool tryLock();

    // set/get indexing mode
    void setMode(IndexingMode mode) { this->mode = mode; }
    IndexingMode getMode() { return mode; }

    // enable/disable plugins
    void setPluginsEnabled(bool enable) { pluginsEnabled = enable; }
    bool getPluginsEnabled() { return pluginsEnabled; }

    // enable/disable filters
    void setFiltersEnabled(bool enable) { filtersEnabled = enable; }
    bool getFiltersEnabled() { return filtersEnabled; }

    // enable/disable sort by time
    void setSortByTimeEnabled(bool enable) { sortByTimeEnabled = enable; }
    bool setSortByTimeEnabled() { return sortByTimeEnabled; }

    // enable/disable multithreaded
    void setMultithreaded(bool enable) { multithreaded = enable; }
    bool getMultithreaded() { return multithreaded; }

    // get and set filter cache
    void setFilterCache(QString path) { filterCache = path; }
    QString getFilterCache() { return filterCache; }

    // get index of all messages
    QVector<qint64> getIndexAll() { return indexAllList; }
    QVector<qint64> getIndexFilters() { return indexFilterList; }
    QList<int> getGetLogInfoList() { return getLogInfoList; }

    // let worker thread append to getLogInfoList
    void appendToGetLogInfoList(int value);

    // reset / clear file indexes
    void clearindex() { indexAllList.clear(); }

    // main thread routine
    void run();

protected:

private:

    // the current set mode of indexing
    IndexingMode mode;

    /* Lock, to reserve exclusive indexing rights
     * to this component when running */
    QMutex indexLock;

    // File to work on
    QDltFile *dltFile;

    // Plugins to be used
    QDltPluginManager *pluginManager;

    // DefaultFilter to be used
    QDltDefaultFilter *defaultFilter;

    // stop flag
    bool stopFlag;

    // active plugins
    QList<QDltPlugin*> activeViewerPlugins;
    QList<QDltPlugin*> activeDecoderPlugins;

    // full index
    QVector<qint64> indexAllList;

    // filtered index
    QVector<qint64> indexFilterList;
    QMultiMap<DltFileIndexerKey,qint64> indexFilterListSorted;

    // getLogInfoList
    QList<int> getLogInfoList;

    // some flags
    bool pluginsEnabled;
    bool filtersEnabled;
    bool multithreaded;
    bool sortByTimeEnabled;

    // filter cache path
    QString filterCache;

    // file errors
    qint64 errors_in_file;

    // run counter
    int maxRun, currentRun;

    // performance counter
    int msecsIndexCounter;
    int msecsFilterCounter;
    int msecsDefaultFilterCounter;

signals:

    // the maximum progress value
    void progressMax(quint64 index);

    // the current progress value
    void progress(quint64 index);

    // progress text change fro different parts
    void progressText(QString text);

    // version log message parsed
    void versionString(QString ecuId, QString version);

    // control message timezone parsed
    void timezone(int timezone, unsigned char dst);

    // control message unregister context
    void unregisterContext(QString ecuId,QString appId,QString ctId);

    // index creation finished
    void finishIndex();

    // complete index creation finished
    void finishFilter();

    // complete index creation of default filter finished
    void finishDefaultFilter();

public slots:

    void stop();

};

#endif // DLTFILEINDEXER_H
