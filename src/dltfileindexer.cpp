#include "dltfileindexer.h"
#include <QDebug>
#include <QMessageBox>
#include <QApplication>

/* Update UI every 10 megabytes  */
#define DLT_VIEWER_DIALOG_UPDATE_INTERVAL (1024*1024*10)

DltFileIndexer::DltFileIndexer(QObject *parent) :
    QObject(parent)
{
}


DltFileIndexer::DltFileIndexer(QDltFile *argfile, QMainWindow *parent)
{
    this->dltFile = argfile;
    this->progress = new QProgressDialog(QString("Indexing file."), QString("Cancel"), 0, 100, parent);
    this->progress->setModal(true);
    this->progress->setAutoClose(true);
    this->progress->reset();
}

DltFileIndexer::~DltFileIndexer()
{
    delete this->progress;
}

bool DltFileIndexer::index()
{
    lock();
    dltFile->clearIndex();
    dltFile->clearFilterIndex();
    QFile f(dltFile->getFileName());
    if(!f.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file in DltFileIndexer " << f.errorString();
        unlock();
        return false;
    }
    if(f.size() == 0)
    {
        // No need to do anything here.
        f.close();
        unlock();
        return true;
    }

    // Prepare file offset list
    segmentFile(&f);

    // This holds the new index
    QList<unsigned long> indexAll;

    // Show progress dialog
    progress->reset();
    progress->show();
    progress->setMaximum(f.size());

    // Go through the segments and create new index
    char lastFound = 0;
    for(int i=0;i < fileSegments.count();i++)
    {
        QPair<qint64, qint64> currentPair = fileSegments.at(i);
        uchar *fmap = f.map(currentPair.first, currentPair.second);
        if(fmap == 0)
        {
            qWarning() << "Cannot mmap file " << f.errorString();
            f.close();
            progress->reset();
            unlock();
            return false;
        }
        for(int num=0;num < currentPair.second;num++)
        {
            if(fmap[num] == 'D')
            {
                lastFound = 'D';
            }
            else if(lastFound == 'D' && fmap[num] == 'L')
            {
                lastFound = 'L';
            }
            else if(lastFound == 'L' && fmap[num] == 'T')
            {
                lastFound = 'T';
            }
            else if(lastFound == 'T' && fmap[num] == 0x01)
            {
                indexAll.append(currentPair.first+num-3);
                lastFound = 0;
            }
            else
            {
                lastFound = 0;
            }

            /* Update UI every 10 megabytes  */
            if((num%(DLT_VIEWER_DIALOG_UPDATE_INTERVAL)) == 0)
            {
                progress->setLabelText(
                            QString("Indexing. Found %1 messages.")
                            .arg(indexAll.count()));
                progress->setValue(currentPair.first+num);
                if(progress->wasCanceled())
                {
                    progress->reset();
                    QMessageBox::warning((QMainWindow *)this->parent(),
                                         tr("DLT Viewer"),
                                         tr("You cancelled the indexing. File was not completelly indexed."),
                                         QMessageBox::Ok);
                    f.close();
                    unlock();
                    return false;
                }
                QApplication::processEvents();
            }
        }
        f.unmap(fmap);
    }
    dltFile->setDltIndex(indexAll);
    f.close();
    unlock();
    progress->reset();
    return true;
}

void DltFileIndexer::segmentFile(QFile *f)
{
    fileSegments.clear();
    qint64 fSize = f->size();
    qint64 segmentCount;
    // Fast ceil of integer division
    segmentCount = 1 + ((fSize - 1) / DLT_MAX_MMAP_SEGMENT);
    qint64 offset = 0;
    qint64 len = 0;
    for(int i=0;i<segmentCount;i++)
    {
        QPair<qint64, qint64> newpair;
        offset = i*DLT_MAX_MMAP_SEGMENT;
        if(offset + DLT_MAX_MMAP_SEGMENT > fSize)
        {
            len = fSize - (i*DLT_MAX_MMAP_SEGMENT);
        }
        else
        {
            len = DLT_MAX_MMAP_SEGMENT;
        }
        newpair.first = offset;
        newpair.second = len;
        fileSegments.append(newpair);
    }

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

