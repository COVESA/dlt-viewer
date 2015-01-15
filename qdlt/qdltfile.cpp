/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 *
 * \file qdlt.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QFile>
#include <QtDebug>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltFile::QDltFile()
{
    filterFlag = false;
    sortByTimeFlag = false;
}

QDltFile::~QDltFile()
{
    clear();
}

void QDltFile::clear()
{
    for(int num=0;num<files.size();num++)
    {
        if(files[num]->infile.isOpen()) {
             files[num]->infile.close();
        }
        delete(files[num]);
    }
    files.clear();
}

int QDltFile::getNumberOfFiles()
{
    return files.size();
}

void QDltFile::setDltIndex(QVector<qint64> &_indexAll, int num){
    if(num<0 || num>=files.size())
        return;

    files[num]->indexAll = _indexAll;
}

int QDltFile::size()
{
    int size=0;

    for(int num=0;num<files.size();num++)
    {
        size += files[num]->indexAll.size();
    }

    return size;
}

qint64 QDltFile::fileSize()
{
    qint64 size=0;

    for(int num=0;num<files.size();num++)
    {
        size += files[num]->infile.size();
    }

    return size;
}

int QDltFile::sizeFilter()
{
    if(filterFlag)
        return indexFilter.size();
    else
        return size();
}

bool QDltFile::open(QString _filename, bool append) {

    qDebug() << "Open file" << _filename << "started";

    /* check if file is already opened */
    if(!append)
        clear();

    /* create new file item */
    QDltFileItem *item = new QDltFileItem();
    files.append(item);

    /* set new filename */
    item->infile.setFileName(_filename);

    /* open the log file read only */
    if(item->infile.open(QIODevice::ReadOnly)==false) {
        /* open file failed */
        qWarning() << "open of file" << _filename << "failed";
        return false;
    }

    qDebug() << "Open file" << _filename << "finished";

    return true;
}

void QDltFile::clearIndex()
{
    for(int num=0;num<files.size();num++)
    {
        files[num]->indexAll.clear();
    }
}

bool QDltFile::createIndex()
{
    bool ret = false;

    qDebug() << "Create index started";

    clearIndex();

    /* Example to use QtConcurrent to call updateIndex in separate Thread*/
    //QFuture<bool> f = QtConcurrent::run(this, &QDltFile::updateIndex);
    //f.waitForFinished();

    ret = updateIndex();

    qDebug() << "Create index finished - "<< size() << "messages found";

    return ret;
}

bool QDltFile::updateIndex()
{
    QByteArray buf;
    qint64 pos = 0;

    mutexQDlt.lock();

    for(int numFile=0;numFile<files.size();numFile++)
    {
        /* check if file is already opened */
        if(!files[numFile]->infile.isOpen()) {
            qDebug() << "updateIndex: Infile is not open";
            mutexQDlt.unlock();
            return false;
        }


        /* start at last found position */
        if(files[numFile]->indexAll.size()) {
            /* move behind last found position */
            pos = files[numFile]->indexAll[files[numFile]->indexAll.size()-1] + 4;
            files[numFile]->infile.seek(pos);
        }
        else {
            /* the file was empty the last call */
            files[numFile]->infile.seek(0);
        }

        /* Align kbytes, 1MB read at a time */
        static const int READ_BUF_SZ = 1024 * 1024;

        /* walk through the whole file and find all DLT0x01 markers */
        /* store the found positions in the indexAll */
        char lastFound = 0;

        while(true) {

            /* read buffer from file */
            buf = files[numFile]->infile.read(READ_BUF_SZ);
            if(buf.isEmpty())
                break; // EOF

            /* Use primitive buffer for faster access */
            int cbuf_sz = buf.size();
            const char *cbuf = buf.constData();

            /* find marker in buffer */
            for(int num=0;num<cbuf_sz;num++) {
                if(cbuf[num] == 'D')
                {
                    lastFound = 'D';
                }
                else if(lastFound == 'D' && cbuf[num] == 'L')
                {
                    lastFound = 'L';
                }
                else if(lastFound == 'L' && cbuf[num] == 'T')
                {
                    lastFound = 'T';
                }
                else if(lastFound == 'T' && cbuf[num] == 0x01)
                {
                    files[numFile]->indexAll.append(pos+num-3);
                    lastFound = 0;
                }
                else
                {
                    lastFound = 0;
                }
            }
            pos += cbuf_sz;
        }
    }

    mutexQDlt.unlock();

    /* success */
    return true;
}

bool QDltFile::createIndexFilter()
{
    /* clear old index */
    indexFilter.clear();

    return updateIndexFilter();
}

bool QDltFile::updateIndexFilter()
{
    QDltMsg msg;
    QByteArray buf;
    int index;

    /* update index filter by starting from last found index in list */

    /* get lattest found index in filter list */
    if(indexFilter.size()>0) {
        index = indexFilter[indexFilter.size()-1] + 1;
    }
    else {
        index = 0;
    }

    for(int num=index;num<size();num++) {
        buf = getMsg(num);
        if(!buf.isEmpty()) {
            msg.setMsg(buf);
            if(checkFilter(msg)) {
                indexFilter.append(num);
            }
        }

    }

    return true;
}

bool QDltFile::checkFilter(QDltMsg &msg)
{  
    if(!filterFlag)
    {
        return true;
    }

    return filterList.checkFilter(msg);
}

QDltFilterList QDltFile::getFilterList()
{
    return filterList;
}

void QDltFile::setFilterList(QDltFilterList &_filterList)
{
    filterList = _filterList;
}

void QDltFile::clearFilterIndex()
{
    /* clear old index */
    indexFilter.clear();

}

void QDltFile::addFilterIndex (int index)
{
    indexFilter.append(index);

}

QColor QDltFile::checkMarker(QDltMsg &msg)
{
    if(!filterFlag)
    {
        return QColor();
    }

    return filterList.checkMarker(msg);
}

QString QDltFile::getFileName(int num)
{
    if(num<0 || num>=files.size())
        return QString();

    return files[num]->infile.fileName();
}

void QDltFile::close()
{
    /* close file */
    clear();
}

QByteArray QDltFile::getMsg(int index)
{
    QByteArray buf;
    int num;


    /* check if index is in range */
    if(index<0 ) {
        qDebug() << "getMsg: Index is out of range";

        /* return empty data buffer */
        return QByteArray();
    }

    for(num=0;num<files.size();num++)
    {
        if(index<files[num]->indexAll.size())
            break;
        else
            index -= files[num]->indexAll.size();
    }

    if(num>=files.size())
    {
        qDebug() << "getMsg: Index is out of range";

        /* return empty data buffer */
        return QByteArray();
    }

    /* check if file is already opened */
    if(!files[num]->infile.isOpen()) {
        /* return empty buffer */
        qDebug() << "getMsg: Infile is not open";

        /* return empty data buffer */
        return QByteArray();
    }

    mutexQDlt.lock();

    QDltFileItem* file = files[num];
    qint64 positionForIndex = file->indexAll[index];

    /* move to file position selected by index */
    file->infile.seek(positionForIndex);

    /* read DLT message from file */
    if(index == (file->indexAll.size()-1))
        /* last message in file */
        buf = file->infile.read(file->infile.size() - positionForIndex);
    else
        /* any other file position */
        buf = file->infile.read(file->indexAll[index+1] - positionForIndex);

    mutexQDlt.unlock();

    /* return DLT message buffer */
    return buf;
}

bool QDltFile::getMsg(int index,QDltMsg &msg)
{
    QByteArray data = getMsg(index);

    if(data.isEmpty())
        return false;

    return msg.setMsg(data);
}

QByteArray QDltFile::getMsgFilter(int index)
{
    if(filterFlag) {
        /* check if index is in range */
        if(index<0 || index>=indexFilter.size()) {
            qDebug() << "getMsgFilter: Index is out of range";

            /* return empty data buffer */
            return QByteArray();
        }
        return getMsg(indexFilter[index]);
    }
    else {
        /* check if index is in range */
        if(index<0 || index>=size()) {
            qDebug() << "getMsgFilter: Index is out of range";

            /* return empty data buffer */
            return QByteArray();
        }
        return getMsg(index);
    }
}

int QDltFile::getMsgFilterPos(int index)
{
    if(filterFlag) {
        /* check if index is in range */
        if(index<0 || index>=indexFilter.size()) {
            qDebug() << "getMsgFilter: Index is out of range";

            /* return invalid */
            return -1;
        }
        return indexFilter[index];
    }
    else {
        /* check if index is in range */
        if(index<0 || index>=size()) {
            qDebug() << "getMsgFilter: Index is out of range";

            /* return invalid */
            return -1;
        }
        return index;
    }
}

void QDltFile::clearFilter()
{
    filterList.clearFilter();
}

void QDltFile::addFilter(QDltFilter *_filter)
{
    filterList.addFilter(_filter);
}

void QDltFile::updateSortedFilter()
{
    filterList.updateSortedFilter();
}

bool QDltFile::isFilter()
{
    return filterFlag;
}

void QDltFile::enableFilter(bool state)
{
    filterFlag = state;
}

void QDltFile::enableSortByTime(bool state)
{
    sortByTimeFlag = state;
}

QVector<qint64> QDltFile::getIndexFilter()
{
    return indexFilter;
}

void QDltFile::setIndexFilter(QVector<qint64> _indexFilter)
{
    indexFilter = _indexFilter;
}
