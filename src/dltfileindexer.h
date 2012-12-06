#ifndef DLTFILEINDEXER_H
#define DLTFILEINDEXER_H

#include <QObject>
#include <QProgressDialog>
#include <QMainWindow>
#include <QPair>
#include <QMutex>

#include "qdlt.h"

/**
 * Maximum slice the memory mapper can handle.
 * This can be anything from 1-3G on win32.
 * We use a safe half a gigabyte value
 **/
#define DLT_MAX_MMAP_SEGMENT (1024*1024*512)

class DltFileIndexer : public QObject
{
    Q_OBJECT
public:
    explicit DltFileIndexer(QObject *parent = 0);
    DltFileIndexer(QDltFile *argFile, QMainWindow *parent = 0);
    ~DltFileIndexer();
    bool index();

    /* Accessors to mutex */
    void lock();
    void unlock();
    bool tryLock();

private:
    DltFileIndexer();
    QProgressDialog *progress;
    /* Offset/Length pairs of file segments */
    QList<QPair<qint64, qint64> > fileSegments;
    /* Lock, to reserve exclusive indexing rights
     * to this component when running */
    QMutex indexLock;
    /* File to work on */
    QDltFile *dltFile;
private:
    void segmentFile(QFile *f);
signals:
    
public slots:
    
};

#endif // DLTFILEINDEXER_H
