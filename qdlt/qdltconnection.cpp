/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
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
 * For further information see http://www.covesa.global/.
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
    dataView.align(data);
    bytesReceived = 0;
    bytesError = 0;
    syncFound = 0;
    messageCounter = 0;
}

void QDltConnection::add(const QByteArray &bytes)
{
    bytesReceived += bytes.size();

    data = dataView + bytes;

    dataView.align(data);
}

bool QDltConnection::parseDlt(QDltMsg &msg)
{
    /* if sync to serial header search for header */
    int found = 0;
    int firstPos = 0;
    int secondPos = 0;

    char lastFound = 0;

    /* Use primitive buffer for faster access */
    int cbuf_sz = dataView.size();
    const char *cbuf = dataView.constData();

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
            bytesError += dataView.size();
            dataView.clear();
        }
        return false;
    }

    //qDebug() << "found " << found << " firstPos " << firstPos << " secondPos " << secondPos;

    if(found==2)
    {
        /* two sync headers found */
        /* try to read msg */
        if(!msg.setMsg(dataView.mid(firstPos,secondPos-firstPos-4),false))
        {
            /* no valid msg found, perhaps to short */
            dataView.advance(secondPos-4);
            /* errors found */
            bytesError += secondPos-4;
            return false;
        }
        else
        {
            /* msg read successful */
            dataView.advance(secondPos-4);
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
    if(!msg.setMsg(dataView.mid(firstPos),false))
    {
        /* no complete msg found */
        /* perhaps not completely received */
        /* check valid size */
        if(dataView.size()>DLT_MAX_MESSAGE_LEN)
        {
            /* size exceeds max DLT message size */
            /* clear buffer */
            /* errors found */
            bytesError += dataView.size();
            dataView.clear();
        }
        return false;
    }

    /* msg read successful */
    dataView.advance(firstPos+msg.getHeaderSize()+msg.getPayloadSize());
    return true;
}

bool QDltConnection::parseAscii(QDltMsg &msg)
{
    bool success = false;

    /* Use primitive buffer for faster access */
    int cbuf_sz = dataView.size();
    const char *cbuf = dataView.constData();

    /* find end of line in buffer */
    for(int num=0;num<cbuf_sz;num++) {
        if(cbuf[num] == '\r' || cbuf[num] == '\n')
        {
            // end of line found

            // check if line is empty, do not store empty lines
            if(num!=0)
            {
                // set parameters of DLT message to be generated
                msg.clear();
                msg.setEcuid("");
                msg.setApid("SER");
                msg.setCtid("ASC");
                msg.setMode(QDltMsg::DltModeVerbose);
                msg.setType(QDltMsg::DltTypeLog);
                msg.setSubtype(QDltMsg::DltLogInfo);
                msg.setMessageCounter(messageCounter++);
                msg.setNumberOfArguments(1);

                // add one argument as String
                QDltArgument arg;
                arg.setTypeInfo(QDltArgument::DltTypeInfoStrg);
                arg.setEndianness(QDltArgument::DltEndiannessLittleEndian);
                arg.setOffsetPayload(0);
                arg.setData(QByteArray(cbuf,num)+QByteArray("",1));
                msg.addArgument(arg);

                // generate binary payload and header of DLT message
                msg.genMsg();

                // succesful found a new line to be written as DLT message
                success = true;
            }

            // remove parsed line from buffer
            if( (num < (cbuf_sz-1)) && (cbuf[num+1] == '\n' || cbuf[num+1] == '\r'))
            {
                // \n and \r found, remove two additional characters
                dataView.advance(num+2);
            }
            else
            {
                // only \n or \r found, remove only one character
                dataView.advance(num+1);
            }

            // msg read successful
            return success;
        }
    }

    // no message found
    return success;
}
