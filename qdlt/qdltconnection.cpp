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

#include <QtDebug>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltConnection::QDltConnection()
{
    sendSerialHeader = false;
    syncSerialHeader = false;

    clear();
}

QDltConnection::~QDltConnection()
{

}

void QDltConnection::setSendSerialHeader(bool _sendSerialHeader)
{
    sendSerialHeader = _sendSerialHeader;
}

bool QDltConnection::getSendSerialHeader() const
{
    return sendSerialHeader;
}

void QDltConnection::setSyncSerialHeader(bool _syncSerialHeader)
{
    syncSerialHeader = _syncSerialHeader;
}

bool QDltConnection::getSyncSerialHeader() const
{
    return syncSerialHeader;
}

void QDltConnection::clear()
{
    data.clear();
    bytesReceived = 0;
    bytesError = 0;
    syncFound = 0;
}

void QDltConnection::add(const QByteArray &bytes)
{
    bytesReceived += bytes.size();

    data += bytes;
}

bool QDltConnection::parse(QDltMsg &msg)
{
    /* if sync to serial header search for header */
    int found = 0;
    int firstPos = 0;
    int secondPos = 0;

    char lastFound = 0;

    /* Use primitive buffer for faster access */
    int cbuf_sz = data.size();
    const char *cbuf = data.constData();

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
        else if(lastFound == 'L' && cbuf[num] == 'S')
        {
            lastFound = 'S';
        }
        else if(lastFound == 'S' && cbuf[num] == 0x01)
        {
            /* header found */
            found++;
            if(found==1)
            {
                firstPos = num+1;
                syncFound++;
            }
            if(found==2)
            {
                secondPos = num+1;
                break;
            }
            lastFound = 0;
        }
        else
        {
            lastFound = 0;
        }
        if((!syncSerialHeader) && num==3)
            break;
    }

    if(syncSerialHeader && !found)
    {
        /* complete sync header not found */
        if(!lastFound)
        {
            /* clear buffer if even not start of sync header found */
            bytesError += data.size();
            data.clear();
        }
        return false;
    }

    //qDebug() << "found " << found << " firstPos " << firstPos << " secondPos " << secondPos;

    if(found==2)
    {
        /* two sync headers found */
        /* try to read msg */
        if(!msg.setMsg(data.mid(firstPos,secondPos-firstPos-4),false))
        {
            /* no valid msg found, perhaps to short */
            data.remove(0,secondPos-4);
            /* errors found */
            bytesError += secondPos-4;
            return false;
        }
        else
        {
            /* msg read successful */
            data.remove(0,secondPos-4);
            if(firstPos>4)
                /* errors found */
                bytesError += firstPos-4;
            return true;
        }

    }

    if(firstPos>4)
        /* errors found */
        bytesError += firstPos-4;

    /* try to read msg */
    if(!msg.setMsg(data.mid(firstPos),false))
    {
        /* no complete msg found */
        /* perhaps not completely received */
        /* check valid size */
        if(data.size()>DLT_MAX_MESSAGE_LEN)
        {
            /* size exceeds max DLT message size */
            /* clear buffer */
            /* errors found */
            bytesError += data.size();
            data.clear();
        }
        return false;
    }

    /* msg read successful */
    data.remove(0,firstPos+msg.getHeaderSize()+msg.getPayloadSize());
    return true;
}

