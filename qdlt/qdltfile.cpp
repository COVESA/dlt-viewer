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
}

QDltFile::~QDltFile()
{
    if(infile.isOpen()) {
        infile.close();
    }
}

void QDltFile::setDltIndex(QList<unsigned long> &_indexAll){
    indexAll = _indexAll;
}

int QDltFile::size()
{
    return indexAll.size();
}

int QDltFile::sizeFilter()
{
    if(filterFlag)
        return indexFilter.size();
    else
        return indexAll.size();
}

bool QDltFile::open(QString _filename) {

    qDebug() << "Open file" << _filename << "started";

    /* check if file is already opened */
    if(infile.isOpen()) {
        qWarning() << "infile.isOpen: file is already open";
        infile.close();
    }

    /* set new filename */
    infile.setFileName(_filename);

    /* open the log file read only */
    if(infile.open(QIODevice::ReadOnly)==false) {
        /* open file failed */
        qWarning() << "open of file" << _filename << "failed";
        return false;
    }

    qDebug() << "Open file" << _filename << "finished";

    return true;
}

void QDltFile::clearIndex()
{
    indexAll.clear();
}

bool QDltFile::createIndex()
{
    bool ret = false;

    qDebug() << "Create index started";

    /* check if file is already opened */
    if(!infile.isOpen()) {
        /* return empty buffer */
        qDebug() << "createIndex: Infile is not open";
        return ret;
    }

    clearIndex();

    /* Example to use QtConcurrent to call updateIndex in sepearte Thread*/
    //QFuture<bool> f = QtConcurrent::run(this, &QDltFile::updateIndex);
    //f.waitForFinished();

    ret = updateIndex();

    qDebug() << "Create index finished - "<< indexAll.size() << "messages found";

    return ret;
}

bool QDltFile::updateIndex()
{
    QByteArray buf;
    unsigned long pos = 0;

    /* check if file is already opened */
    if(!infile.isOpen()) {
        qDebug() << "updateIndex: Infile is not open";
        return false;
    }

    mutexQDlt.lock();

    /* start at last found position */
    if(indexAll.size()) {
        /* move behind last found position */
        pos = indexAll[indexAll.size()-1] + 4;
        infile.seek(pos);
    }
    else {
        /* the file was empty the last call */
        infile.seek(0);
    }

    /* Align kbytes, 1MB read at a time */
    static const int READ_BUF_SZ = 1024 * 1024;

    /* walk through the whole file and find all DLT0x01 markers */
    /* store the found positions in the indexAll */
    char lastFound = 0;

    while(true) {

        /* read buffer from file */
        buf = infile.read(READ_BUF_SZ);
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
                indexAll.append(pos+num-3);
                lastFound = 0;
            }
            else
            {
                lastFound = 0;
            }
        }
        pos += cbuf_sz;
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

    for(int num=index;num<indexAll.size();num++) {
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

QString QDltFile::getFileName()
{
    return infile.fileName();
}

void QDltFile::close()
{
    /* close file */
    infile.close();
}

QByteArray QDltFile::getMsg(int index)
{
    QByteArray buf;

    /* check if file is already opened */
    if(!infile.isOpen()) {
        /* return empty buffer */
        qDebug() << "getMsg: Infile is not open";

        /* return empty data buffer */
        return QByteArray();
    }

    /* check if index is in range */
    if(index<0 || index>=indexAll.size()) {
        qDebug() << "getMsg: Index is out of range";

        /* return empty data buffer */
        return QByteArray();
    }

    mutexQDlt.lock();

    /* move to file position selected by index */
    infile.seek(indexAll[index]);

    /* read DLT message from file */
    if(index == (indexAll.size()-1))
        /* last message in file */
        buf = infile.read(infile.size()-indexAll[index]);
    else
        /* any other file position */
        buf = infile.read(indexAll[index+1]-indexAll[index]);

    mutexQDlt.unlock();

    /* return DLT message buffer */
    return buf;
}

bool QDltFile::getMsg(int index,QDltMsg &msg)
{
    QByteArray data;

    data = getMsg(index);

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
        if(index<0 || index>=indexAll.size()) {
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
        if(index<0 || index>=indexAll.size()) {
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

bool QDltFile::isFilter()
{
    return filterFlag;
}

void QDltFile::enableFilter(bool state)
{
    filterFlag = state;
}

QList<unsigned long> QDltFile::getIndexFilter()
{
    return indexFilter;
}

void QDltFile::setIndexFilter(QList<unsigned long> &_indexFilter)
{
    indexFilter = _indexFilter;
}
