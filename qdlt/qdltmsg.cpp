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
#include <QtEndian>

#include "qdltmsg.h"

extern "C"
{
#include "dlt_common.h"
}

QDltMsg::QDltMsg()
{
    clear();
}

QDltMsg::~QDltMsg()
{

}

QString QDltMsg::getTypeString() const
{
    return QString((type>=0 && type<=7)?qDltMessageType[type]:"");
}

QString QDltMsg::getSubtypeString() const
{
    switch(type)
    {
    case DltTypeLog:
        return QString((subtype>=0 && subtype<=7)?qDltLogInfo[subtype]:"");
        break;
    case DltTypeAppTrace:
        return QString((subtype>=0 && subtype<=7)?qDltTraceType[subtype]:"");
        break;
    case DltTypeNwTrace:
        return QString((subtype>=0 && subtype<=7)?qDltNwTraceType[subtype]:"");
        break;
    case DltTypeControl:
        return QString((subtype>=0 && subtype<=7)?qDltControlType[subtype]:"");
        break;
    default:
        return QString("");
    }
}

QString QDltMsg::getModeString() const
{
    return QString((mode>=0 && mode<=1)?qDltMode[mode]:"");
}

QString QDltMsg::getEndiannessString() const
{
    return QString((endianness>=0 && endianness<=1)?qDltEndianness[endianness]:"");
}

unsigned int QDltMsg::getCtrlServiceId() const
{
    return ctrlServiceId;
}

QString QDltMsg::getCtrlServiceIdString() const
{
    if(ctrlServiceId == DLT_SERVICE_ID_UNREGISTER_CONTEXT)
        return QString("unregister_context");
    else if(ctrlServiceId == DLT_SERVICE_ID_CONNECTION_INFO)
        return QString("connection_info");
    else if(ctrlServiceId == DLT_SERVICE_ID_TIMEZONE)
        return QString("timezone");
    else if(ctrlServiceId == DLT_SERVICE_ID_MARKER)
        return QString("marker");
    else
        return QString(( ctrlServiceId<=20 )?qDltCtrlServiceId[ctrlServiceId]:"");
}

unsigned char QDltMsg::getCtrlReturnType() const
{
    return ctrlReturnType;
}

QString QDltMsg::getCtrlReturnTypeString() const
{
    return QString(( ctrlReturnType<=8 )?qDltCtrlReturnType[ctrlReturnType]:"");
}
QString QDltMsg::getTimeString() const
{
    char strtime[256];
    struct tm *time_tm;
    time_tm = localtime(&time);
    if(time_tm)
        strftime(strtime, 256, "%Y/%m/%d %H:%M:%S", time_tm);
    return QString(strtime);
}

QString QDltMsg::getGmTimeWithOffsetString(qlonglong offset, bool dst)
{
    struct tm *time_tm;
    time_tm = gmtime(&time);

    /*Reason for adding:
        tm_mon	months since January	0-11
        tm_year	years since 1900
    */
    QDate date(time_tm->tm_year+1900,time_tm->tm_mon+1,time_tm->tm_mday);
    QTime time(time_tm->tm_hour,time_tm->tm_min,time_tm->tm_sec);

    if(!date.isValid() || !time.isValid())
        return QString("Invalid date");

    QDateTime gmDateTime(date,time,Qt::UTC);

    gmDateTime = gmDateTime.addSecs(offset);

    if(dst)
       gmDateTime = gmDateTime.addSecs(3600);

    return gmDateTime.toString("yyyy/MM/dd hh:mm:ss");
}


bool QDltMsg::setMsg(const QByteArray& buf, bool withStorageHeader)
{
    unsigned int offset;
    QDltArgument argument;
    const DltStorageHeader *storageheader = 0;
    const DltStandardHeader *standardheader = 0;
    const DltExtendedHeader *extendedheader = 0;
    DltStandardHeaderExtra headerextra;
    unsigned int extra_size,headersize,datasize;
    int sizeStorageHeader = 0;

    /* empty message */
    clear();

    /* set offset of storage header */
    if(withStorageHeader) {
        if(buf.size() < (int)(4)) {
            return false; // length error
        }
        quint8 storageHeaderVersion = *((quint8*) (buf.constData() + 3));
        if(storageHeaderVersion==1)
        {
            sizeStorageHeader = sizeof(DltStorageHeader);
        }
        else if(storageHeaderVersion==2)
        {
            if(buf.size() < (int)(4)) {
                return false; // length error
            }
            quint8 ecuIdLength = *((quint8*) (buf.constData() + 13));
            sizeStorageHeader = 14 + ecuIdLength;
        }
        else
        {
            return false; // unknown storage header version
        }
    }

    /* get DLT protocol version */
    if(buf.size() < (int)(sizeStorageHeader+4)) {
        return false;
    }
    quint32 htyp2 = *((quint32*) (buf.constData() + sizeStorageHeader));
    versionNumber = (htyp2 & 0xe0) >> 5;  // Byte 0, Bit 5-7

    if(versionNumber==1)
    {

        if(buf.size() < (int)(sizeStorageHeader+sizeof(DltStandardHeader))) {
            return false;
        }

        if(withStorageHeader) {
            storageheader = (DltStorageHeader*) buf.constData();
        }

        standardheader = (DltStandardHeader*) (buf.constData() + sizeStorageHeader);

        /* calculate complete size of headers */
        extra_size = DLT_STANDARD_HEADER_EXTRA_SIZE(standardheader->htyp)+(DLT_IS_HTYP_UEH(standardheader->htyp) ? sizeof(DltExtendedHeader) : 0);
        headersize = sizeStorageHeader + sizeof(DltStandardHeader) + extra_size;
        if(DLT_SWAP_16(standardheader->len)<(static_cast<int>(headersize) - sizeStorageHeader))
        {
            // there is something wrong with the header, at least size of header
            // at the momment no error, distinguish different errors needed
            datasize = 0;
        }
        else
        {
            datasize =  DLT_SWAP_16(standardheader->len) - (headersize - sizeStorageHeader);
        }

        /* check header length */
        if (buf.size()  < (int)(headersize)) {
            return false;
        }

        /* store payload size */
        payloadSize = datasize;

        /* store header size */
        headerSize = headersize;

        /* copy header */
        header = buf.mid(0,headersize);

        /* load standard header extra parameters and Extended header if used */
        if (extra_size>0)
        {
            /* set extended header ptr and get standard header extra parameters */
            if (DLT_IS_HTYP_UEH(standardheader->htyp)) {
                extendedheader = (DltExtendedHeader*) (buf.constData() + sizeStorageHeader + sizeof(DltStandardHeader) +
                                      DLT_STANDARD_HEADER_EXTRA_SIZE(standardheader->htyp));
            }
            else {
                extendedheader = 0;
            }

            if (DLT_IS_HTYP_WEID(standardheader->htyp))
            {
                memcpy(headerextra.ecu,buf.constData() + sizeStorageHeader + sizeof(DltStandardHeader),DLT_ID_SIZE);
            }

            if (DLT_IS_HTYP_WSID(standardheader->htyp))
            {
                memcpy(&(headerextra.seid),buf.constData() + sizeStorageHeader + sizeof(DltStandardHeader)
                       + (DLT_IS_HTYP_WEID(standardheader->htyp) ? DLT_SIZE_WEID : 0), DLT_SIZE_WSID);
                headerextra.seid = DLT_BETOH_32(headerextra.seid);
            }

            if (DLT_IS_HTYP_WTMS(standardheader->htyp))
            {
                memcpy(&(headerextra.tmsp),buf.constData() + sizeStorageHeader + sizeof(DltStandardHeader)
                       + (DLT_IS_HTYP_WEID(standardheader->htyp) ? DLT_SIZE_WEID : 0)
                       + (DLT_IS_HTYP_WSID(standardheader->htyp) ? DLT_SIZE_WSID : 0),DLT_SIZE_WTMS);
                headerextra.tmsp = DLT_BETOH_32(headerextra.tmsp);
            }
        }

        /* extract ecu id */
        if ( DLT_IS_HTYP_WEID(standardheader->htyp) )
        {
            ecuid = QString(QByteArray(headerextra.ecu,4));
        }
        else
        {
            if(storageheader)
                ecuid = QString(QByteArray(storageheader->ecu,4));
        }

        /* extract application id */
        if ((DLT_IS_HTYP_UEH(standardheader->htyp)) && (extendedheader->apid[0]!=0))
        {
            apid = QString(QByteArray(extendedheader->apid,4));
        }

        /* extract context id */
        if ((DLT_IS_HTYP_UEH(standardheader->htyp)) && (extendedheader->ctid[0]!=0))
        {
            ctid = QString(QByteArray(extendedheader->ctid,4));
        }

        /* extract type */
        if (DLT_IS_HTYP_UEH(standardheader->htyp))
        {
            type = (DltTypeDef) DLT_GET_MSIN_MSTP(extendedheader->msin);
        }

        /* extract subtype */
        if (DLT_IS_HTYP_UEH(standardheader->htyp))
        {
            subtype = DLT_GET_MSIN_MTIN(extendedheader->msin);
        }

        /* extract mode */
        if (DLT_IS_HTYP_UEH(standardheader->htyp))
        {
            if(DLT_IS_MSIN_VERB(extendedheader->msin))
            {
                mode = DltModeVerbose;
            }
            else
            {
                mode = DltModeNonVerbose;
            }
        }
        else
        {
            mode = DltModeNonVerbose;
        }

        /* extract endianness */
        if(DLT_IS_HTYP_MSBF(standardheader->htyp)) {
            endianness = DltEndiannessBigEndian;
        }
        else {
            endianness = DltEndiannessLittleEndian;
        }

        /* extract time */
        if(storageheader) {
            time = storageheader->seconds;
            microseconds = storageheader->microseconds;
        }

        /* extract timestamp */
        if ( DLT_IS_HTYP_WTMS(standardheader->htyp) ) {
            timestamp = headerextra.tmsp; /* big endian to host little endian conversion already done */
        }

        /* extract session id */
        if (DLT_IS_HTYP_WSID(standardheader->htyp)) {
            sessionid = headerextra.seid;
        }

        /* extract message counter */
        messageCounter = standardheader->mcnt;

        /* extract number of arguments */
        if (DLT_IS_HTYP_UEH(standardheader->htyp) && (mode == DltModeVerbose)) {
            numberOfArguments = extendedheader->noar;
        }

        /* check complete length */
        if (buf.size()  < (int)(headersize+payloadSize)) {
            return false;
        }

        /* copy payload */
        if(payloadSize>0)
            payload = buf.mid(headersize,payloadSize);

        /* set messageid if non verbose */
        if((mode == DltModeNonVerbose) && payload.size()>=4) {
            /* message id is always in big endian format */
            if(endianness == DltEndiannessLittleEndian) {
                messageId = (*((unsigned int*) payload.constData()));
            }
            else {
                messageId = DLT_SWAP_32((*((unsigned int*) payload.constData())));
            }
        }

        /* set service id if message of type control */
        if((type == DltTypeControl) && payload.size()>=4) {
            if(endianness == DltEndiannessLittleEndian)
                ctrlServiceId = *((unsigned int*) payload.constData());
            else
                ctrlServiceId = DLT_SWAP_32(*((unsigned int*) payload.constData()));
        }

        /* set return type if message of type control response */
        if((type == QDltMsg::DltTypeControl) && (subtype == QDltMsg::DltControlResponse) && payload.size()>=5) {
            ctrlReturnType = *((unsigned char*) &(payload.constData()[4]));
        }

        /* get the arguments of the payload */
        if(mode==DltModeVerbose) {
            offset = 0;
            arguments.clear();
            for(int num=0;num<numberOfArguments;num++) {
                if(argument.setArgument(payload,offset,endianness)==false) {
                    /* There was an error parsing the arguments */
                    return false;
                }
                arguments.append(argument);
            }
        }

        return true;
    }
    else if(versionNumber==2)
    {
        /* Version 2 */

        if(buf.size() < (int)(sizeStorageHeader+sizeof(DltStandardHeader))) {
            return false;
        }

        if(withStorageHeader) {
            storageheader = (DltStorageHeader*) buf.constData();
        }

        quint32 headerLength = 7;
        if(buf.size() < (int)(sizeStorageHeader+headerLength)) {
            return false; // length error
        }

        /* decode Header Type 2 */
        bool withSessionId = htyp2 & 0x10;
        bool withAppContextId = htyp2 & 0x08;
        bool withEcuId = htyp2 & 0x04;
        quint8 contentInformation = htyp2 & 0x03; // 0x0 = verbose, 0x1 = non verbose, 0x2 = control
        bool withHFMessageInfo = (contentInformation == 0x00) || (contentInformation == 0x02); // verbose or control
        bool withHFNumberOfArguments = (contentInformation == 0x00) || (contentInformation == 0x02); // verbose or control
        bool withHFTimestamp = (contentInformation == 0x00) || (contentInformation == 0x01); // verbose or none verbose
        bool withHFMessageId = (contentInformation == 0x01); // none verbose
        if(contentInformation==0x00)
        {
            mode = DltModeVerbose;
        }
        else if(contentInformation==0x01)
        {
            mode = DltModeNonVerbose;
        }
        else if(contentInformation==0x02)
        {
            mode = DltModeNonVerbose;
        }
        else
        {
            /* error used reserved value */
            return false;
        }
        bool withSegementation = htyp2 & 0x0800;
        bool withPrivacyLevel = htyp2 & 0x0400;
        bool withTags = htyp2 & 0x0200;
        bool withSourceFileNameLineNumber = htyp2 & 0x0100;
        /* TODO: Endianess of payload not defined in DLTv2, undefined, set to LittleEndian by default */
        endianness = DltEndiannessLittleEndian;

        /* get Message Counter : always*/
        quint8 messageCounter = *((quint8*) (buf.constData() + 4 + sizeStorageHeader));
        this->messageCounter = messageCounter;

        /* get Message Length : always*/
        quint16 messageLength = messageLength = qFromBigEndian(*((quint16*) (buf.constData() + 5 + sizeStorageHeader)));

        /* get Message Info : conditional */
        quint8 messageInfo = 0;
        quint8 messageType = 0;
        quint8 messageTypeInfo = 0;
        if(withHFMessageInfo) // verbose or control
        {
            headerLength += 1;
            if(buf.size() < (int)(sizeStorageHeader+headerLength)) {
                return false; // length error
            }
            messageInfo = *((quint8*) (buf.constData() + 7 + sizeStorageHeader));
            messageType =  (messageInfo & 0x0e) >> 1;
            type = (QDltMsg::DltTypeDef) messageType;
            if(messageType>0x03)
            {
                /* error used reserved value */
                return false;
            }
            messageTypeInfo =  (messageInfo & 0xf0) >> 4;
            subtype = (int) messageTypeInfo;
        }

        /* get Number of Arguments : conditional */
        quint8 numberOfArguments;
        if(withHFNumberOfArguments)
        {
            headerLength += 1;
            if(buf.size() < (int)(sizeStorageHeader+headerLength)) {
                return false; // length error
            }
            numberOfArguments = *((quint8*) (buf.constData() + headerLength - 1 + sizeStorageHeader));
            this-> numberOfArguments = numberOfArguments;
        }

        /* get Timestamp : conditional */
        quint32 timestampNanoseconds = 0;
        quint64 timestampSeconds = 0;
        if(withHFTimestamp)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 9)) {
                return false; // length error
            }
            timestampNanoseconds = qFromBigEndian(*((quint32*) (buf.constData() + headerLength + sizeStorageHeader)));
//            timestampSeconds = qFromBigEndian((((quint64)(*((quint32*) (buf.constData() + headerLength + 4 + sizeStorageHeader))))<<8)|
//                                              ((quint64)(*((quint8*) (buf.constData() + headerLength + 8 + sizeStorageHeader)))));
            timestampSeconds = (((quint64)(*((quint8*) (buf.constData() + headerLength + 4 + sizeStorageHeader))))<<32)|
                               (((quint64)(*((quint8*) (buf.constData() + headerLength + 5 + sizeStorageHeader))))<<24)|
                               (((quint64)(*((quint8*) (buf.constData() + headerLength + 6 + sizeStorageHeader))))<<16)|
                               (((quint64)(*((quint8*) (buf.constData() + headerLength + 7 + sizeStorageHeader))))<<8)|
                               (((quint64)(*((quint8*) (buf.constData() + headerLength + 8 + sizeStorageHeader)))));
            if(timestampNanoseconds&0x80000000)
            {
                timestampNanoseconds &= 0x7fffffff;
                timestamp = (timestampSeconds * (quint64)10000) + ((quint64)timestampNanoseconds / (quint64)100000ul);
                if(storageheader) {
                    time = storageheader->seconds;
                    microseconds = storageheader->microseconds;
                }
            }
            else
            {
                time = timestamp;
                microseconds = (quint64)timestampNanoseconds / (quint64)1000ul;
            }
            headerLength += 9;
        }

        /* get Message ID : conditional */
        quint32 messageId = 0;
        if(withHFMessageId)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 4)) {
                return false; // length error
            }
            messageId = qFromBigEndian(*((quint32*) (buf.constData() + headerLength + sizeStorageHeader)));
            this-> messageId = messageId;
            headerLength += 4;
        }

        /* read optional ECU Id */
        if(withEcuId)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 1)) {
                return false; // length error
            }
            quint8 length = *((quint8*) (buf.constData() + headerLength + sizeStorageHeader));
            headerLength += 1;
            if(buf.size() < (int)(sizeStorageHeader + headerLength + length)) {
                return false; // length error
            }
            ecuid = QString(buf.mid(headerLength + sizeStorageHeader,length));
            headerLength += length;
        }

        /* read optional App Id and Ctx Id */
        if(withAppContextId)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 1)) {
                return false; // length error
            }
            quint8 length = *((quint8*) (buf.constData() + headerLength + sizeStorageHeader));
            headerLength += 1;
            if(buf.size() < (int)(sizeStorageHeader + headerLength + length)) {
                return false; // length error
            }
            apid = QString(buf.mid(headerLength + sizeStorageHeader,length));
            headerLength += length;
            length = *((quint8*) (buf.constData() + headerLength + sizeStorageHeader));
            headerLength += 1;
            if(buf.size() < (int)(sizeStorageHeader + headerLength + length)) {
                return false; // length error
            }
            ctid = QString(buf.mid(headerLength + sizeStorageHeader,length));
            headerLength += length;
        }

        /* read optional Session Id */
        quint32 sessionId;
        if(withSessionId)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 4)) {
                return false; // length error
            }
            sessionId = qFromBigEndian(*((quint32*) (buf.constData() + headerLength + sizeStorageHeader)));
            sessionid = sessionId;
            headerLength += 4;
        }

        /* read optional Source File Name and Line Number */
        if(withSourceFileNameLineNumber)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 1)) {
                return false; // length error
            }
            quint8 length = *((quint8*) (buf.constData() + headerLength + sizeStorageHeader));
            headerLength += 1;
            headerLength += length;
        }

        /* read optional Tags */
        if(withTags)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 1)) {
                return false; // length error
            }
            quint8 numberOfTags = *((quint8*) (buf.constData() + headerLength + sizeStorageHeader));
            headerLength += 1;
            for(int num=0;num<numberOfTags;num++)
            {
                if(buf.size() < (int)(sizeStorageHeader + headerLength + 1)) {
                    return false; // length error
                }
                quint8 length = *((quint8*) (buf.constData() + headerLength + sizeStorageHeader));
                headerLength += 1;
                if(buf.size() < (int)(sizeStorageHeader + headerLength + length)) {
                    return false; // length error
                }
                headerLength += length;
            }
        }

        /* read privacy level */
        quint8 privacyLevel;
        if(withPrivacyLevel)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 1)) {
                return false; // length error
            }
            privacyLevel = *((quint8*) (buf.constData() + headerLength + sizeStorageHeader));
            headerLength += 1;
        }

        /* read Segmentation */
        if(withSegementation)
        {
            if(buf.size() < (int)(sizeStorageHeader + headerLength + 1)) {
                return false; // length error
            }
            quint8 length = *((quint8*) (buf.constData() + headerLength + sizeStorageHeader));
            headerLength += length + 1;
        }

        headerSize = sizeStorageHeader + headerLength;
        headersize = headerSize;
        payloadSize = messageLength - (headerSize - sizeStorageHeader);

        /* copy header */
        header = buf.mid(0,headersize);

        /* copy payload */
        if(payloadSize>0)
            payload = buf.mid(headersize,payloadSize);

        /* set service id if message of type control */
        if((type == DltTypeControl) && payload.size()>=4) {
            if(endianness == DltEndiannessLittleEndian)
                ctrlServiceId = *((unsigned int*) payload.constData());
            else
                ctrlServiceId = DLT_SWAP_32(*((unsigned int*) payload.constData()));
        }

        /* set return type if message of type control response */
        if((type == QDltMsg::DltTypeControl) && (subtype == QDltMsg::DltControlResponse) && payload.size()>=5) {
            ctrlReturnType = *((unsigned char*) &(payload.constData()[4]));
        }

        /* get the arguments of the payload */
        if(mode==DltModeVerbose) {
            offset = 0;
            arguments.clear();
            for(int num=0;num<numberOfArguments;num++) {
                if(argument.setArgument(payload,offset,endianness)==false) {
                    /* There was an error parsing the arguments */
                    return false;
                }
                arguments.append(argument);
            }
        }

        return true;
    }
    else
    {
        /* unsupported version */
        return false;
    }
}

bool QDltMsg::getMsg(QByteArray &buf,bool withStorageHeader) {
    DltStorageHeader storageheader;
    DltStandardHeader standardheader;
    DltStandardHeaderExtra headerextra;
    DltExtendedHeader extendedheader;

    /* empty return buffer */
    buf.clear();

    /* prepare payload */
    payload.clear();
    for (int num = 0;num<arguments.size();num++)
    {
        if(!(arguments[num].getArgument(payload,mode==DltModeVerbose)))
            return false;
    }

    /* write storageheader */
    if(withStorageHeader)
    {
        storageheader.pattern[0] = 'D';
        storageheader.pattern[1] = 'L';
        storageheader.pattern[2] = 'T';
        storageheader.pattern[3] = 0x01;
        strncpy(storageheader.ecu,ecuid.toLatin1().constData(),ecuid.size()>3?4:ecuid.size()+1);
        storageheader.microseconds = microseconds;
        storageheader.seconds = time;
        buf += QByteArray((const char *)&storageheader,sizeof(DltStorageHeader));
    }

    /* write standardheader */
    standardheader.htyp = 0x01 << 5; /* intialise with version number 0x1 */
    if(endianness == DltEndiannessBigEndian) {
        standardheader.htyp |= DLT_HTYP_MSBF;
    }
    if(mode == DltModeVerbose) {
        standardheader.htyp |= DLT_HTYP_UEH;
        standardheader.htyp |= DLT_HTYP_WEID;
        standardheader.htyp |= DLT_HTYP_WSID;
        standardheader.htyp |= DLT_HTYP_WTMS;
        standardheader.len = DLT_SWAP_16(sizeof(DltStandardHeader) + sizeof(headerextra.ecu) + sizeof(headerextra.seid) +
                                     sizeof(headerextra.tmsp) + sizeof(DltExtendedHeader) + payload.size());
    }
    else {
        standardheader.len = DLT_SWAP_16(sizeof(DltStandardHeader) + payload.size());
    }
    standardheader.mcnt = messageCounter;
    buf += QByteArray((const char *)&standardheader,sizeof(DltStandardHeader));

    /* write standard header extra */
    if(mode == DltModeVerbose) {
        strncpy(headerextra.ecu,ecuid.toLatin1().constData(),ecuid.size()>3?4:ecuid.size()+1);
        buf += QByteArray((const char *)&(headerextra.ecu),sizeof(headerextra.ecu));
        headerextra.seid = DLT_SWAP_32(sessionid);
        buf += QByteArray((const char *)&(headerextra.seid),sizeof(headerextra.seid));
        headerextra.tmsp = DLT_SWAP_32(timestamp);
        buf += QByteArray((const char *)&(headerextra.tmsp),sizeof(headerextra.tmsp));
    }

    /* write extendedheader */
    if(mode == DltModeVerbose) {
        strncpy(extendedheader.apid,apid.toLatin1().constData(),apid.size()>3?4:apid.size()+1);
        strncpy(extendedheader.ctid,ctid.toLatin1().constData(),ctid.size()>3?4:ctid.size()+1);
        extendedheader.msin = 0;
        if(mode == DltModeVerbose) {
            extendedheader.msin |= DLT_MSIN_VERB;
        }
        extendedheader.msin |= (((unsigned char)type) << 1) & DLT_MSIN_MSTP;
        extendedheader.msin |= (((unsigned char)subtype) << 4) & DLT_MSIN_MTIN;
        extendedheader.noar = numberOfArguments;
        buf += QByteArray((const char *)&extendedheader,sizeof(DltExtendedHeader));
    }

    /* write payload */
    buf += payload;

    return true;
}


void QDltMsg::clear()
{
    ecuid.clear();
    apid.clear();
    ctid.clear();
    type = DltTypeUnknown;
    subtype = DltLogUnknown;
    mode = DltModeUnknown;
    endianness = DltEndiannessUnknown;
    time = 0;
    microseconds = 0;
    timestamp = 0;
    sessionid = 0;
    sessionName.clear();
    numberOfArguments = 0;
    messageId = 0;
    ctrlServiceId = 0;
    ctrlReturnType = 0;
    arguments.clear();
    payload.clear();
    payloadSize = 0;
    header.clear();
    headerSize = 0;
    versionNumber=0;
}

void QDltMsg::clearArguments()
{
    arguments.clear();
}

int QDltMsg::sizeArguments() const
{
    return arguments.size();
}

bool QDltMsg::getArgument(int index,QDltArgument &argument) const
{
      if(index<0 || index>=arguments.size())
          return false;

      argument = arguments.at(index);

      return true;
}

void QDltMsg::addArgument(QDltArgument argument, int index)
{
    if(index == -1)
        arguments.append(argument);
    else
        arguments.insert(index,argument);
}

void QDltMsg::removeArgument(int index)
{
    arguments.removeAt(index);
}


QString QDltMsg::toStringHeader() const
{
    QString text;
    text.reserve(1024);

    text += QString("%1.%2").arg(getTimeString()).arg(getMicroseconds(),6,10,QLatin1Char('0'));
    text += QString(" %1.%2").arg(getTimestamp()/10000).arg(getTimestamp()%10000,4,10,QLatin1Char('0'));
    text += QString(" %1").arg(getMessageCounter());
    text += QString(" %1").arg(getEcuid());
    text += QString(" %1").arg(getApid());
    text += QString(" %1").arg(getCtid());
    text += QString(" %1").arg(getSessionid());
    text += QString(" %2").arg(getTypeString());
    text += QString(" %2").arg(getSubtypeString());
    text += QString(" %2").arg(getModeString());
    text += QString(" %1").arg(getNumberOfArguments());

    return text;
}

QString QDltMsg::toStringPayload() const
{
    QString text;
    QDltArgument argument;
    QByteArray data;

    text.reserve(1024);

    if((getMode()==QDltMsg::DltModeNonVerbose) && (getType()!=QDltMsg::DltTypeControl) && (getNumberOfArguments() == 0)) {
        text += QString("[%1] ").arg(getMessageId());
        data = payload.mid(4,(payload.size()>260)?256:(payload.size()-4));
        if(!data.isEmpty())
        {
            text += toAsciiTable(data,false,false,true,1024,1024,false);
            text += "|";
            text += toAscii(data, false);
        }
        return text;
    }

    if( getType()==QDltMsg::DltTypeControl && getSubtype()==QDltMsg::DltControlResponse) {

        if(getCtrlServiceId() == DLT_SERVICE_ID_MARKER)
        {
            return "MARKER";
        }

        text += QString("[%1 %2] ").arg(getCtrlServiceIdString()).arg(getCtrlReturnTypeString());

        // ServiceID of Get ECU Software Version
        if(getCtrlServiceId() == DLT_SERVICE_ID_GET_SOFTWARE_VERSION)
        {
            // Skip the ServiceID, Status and Lenght bytes and start from the String containing the ECU Software Version
            data = payload.mid(9,(payload.size()>265)?256:(payload.size()-9));
            text += toAscii(data,true);
        }
        else if(getCtrlServiceId() == DLT_SERVICE_ID_CONNECTION_INFO)
        {
            if(payload.size() == sizeof(DltServiceConnectionInfo))
            {
                DltServiceConnectionInfo *service;
                service = (DltServiceConnectionInfo*) payload.constData();
                switch(service->state)
                {
                case DLT_CONNECTION_STATUS_DISCONNECTED:
                    text += "disconnected";
                    break;
                case DLT_CONNECTION_STATUS_CONNECTED:
                    text += "connected";
                    break;
                default:
                    text += "unknown";
                }
                text += " " + QString(QByteArray(service->comid,4));
            }
            else
            {
                data = payload.mid(5,(payload.size()>261)?256:(payload.size()-5));
                text += toAscii(data);
            }
        }
        else if(getCtrlServiceId() == DLT_SERVICE_ID_TIMEZONE)
        {
            if(payload.size() == sizeof(DltServiceTimezone))
            {
                DltServiceTimezone *service;
                service = (DltServiceTimezone*) payload.constData();

                if(endianness == DltEndiannessLittleEndian)
                    text += QString("%1 s").arg(service->timezone);
                else
                    text += QString("%1 s").arg(DLT_SWAP_32(service->timezone));
                text += QString(" %1").arg(service->isdst?"DST":"");
            }
            else
            {
                data = payload.mid(5,(payload.size()>261)?256:(payload.size()-5));
                text += toAscii(data);
            }
        }
        else
        {
            data = payload.mid(5,(payload.size()>261)?256:(payload.size()-5));
            text += toAscii(data);
        }

        return text;
    }

    if( getType()==QDltMsg::DltTypeControl) {
        text += QString("[%1] ").arg(getCtrlServiceIdString());
        data = payload.mid(4,(payload.size()>260)?256:(payload.size()-4));
        text += toAscii(data);

        return text;
    }

    for(int num=0;num<arguments.size();num++) {
        if(getArgument(num,argument)) {
            if(num!=0) {
                text += " ";
            }
            text += argument.toString();
        }

    }

    return text;
}

void QDltMsg::genMsg()
{
    DltStandardHeader standardheader;
    DltStandardHeaderExtra headerextra;
    DltExtendedHeader extendedheader;
    QDltArgument argument;

    // clear existing payload
    payload.clear();

    // Generate payload for all arguments
    for(int num=0;num<arguments.size();num++) {
        if(getArgument(num,argument)) {
            argument.getArgument(payload,true);
        }

    }

    // set payload size
    payloadSize = payload.size();

    // clear existing header
    header.clear();

    // write standardheader
    standardheader.htyp = 0x01 << 5; /* intialise with version number 0x1 */
    if(endianness == DltEndiannessBigEndian) {
        standardheader.htyp |= DLT_HTYP_MSBF;
    }
    if(mode == DltModeVerbose) {
        uint16_t standardheaderlen = sizeof(DltStandardHeader) + sizeof(DltExtendedHeader) + payload.size();
        standardheader.htyp |= DLT_HTYP_UEH;
        if(!ecuid.isEmpty()) {
            standardheader.htyp |= DLT_HTYP_WEID;
            standardheaderlen += sizeof(headerextra.ecu);
        }
        if(sessionid!=0) {
            standardheader.htyp |= DLT_HTYP_WSID;
            standardheaderlen += sizeof(headerextra.seid);
        }
        if(timestamp!=0) {
            standardheader.htyp |= DLT_HTYP_WTMS;
            standardheaderlen += sizeof(headerextra.tmsp);
        }
        standardheader.len = DLT_HTOBE_16(standardheaderlen);
    }
    else {
        standardheader.len = DLT_HTOBE_16(sizeof(DltStandardHeader) + payload.size());
    }
    standardheader.mcnt = messageCounter;
    header += QByteArray((const char *)&standardheader,sizeof(DltStandardHeader));

    // write standard header extra
    if(mode == DltModeVerbose) {
        if(!ecuid.isEmpty()) {
            strncpy(headerextra.ecu,ecuid.toLatin1().constData(),ecuid.size()>3?4:ecuid.size()+1);
            header += QByteArray((const char *)&(headerextra.ecu),sizeof(headerextra.ecu));
        }
        if(sessionid!=0) {
            headerextra.seid = DLT_HTOBE_32(sessionid);
            header += QByteArray((const char *)&(headerextra.seid),sizeof(headerextra.seid));
        }
        if(timestamp!=0) {
            headerextra.tmsp = DLT_HTOBE_32(timestamp);
            header += QByteArray((const char *)&(headerextra.tmsp),sizeof(headerextra.tmsp));
        }
    }

    // write extendedheader
    if(mode == DltModeVerbose) {
        strncpy(extendedheader.apid,apid.toLatin1().constData(),apid.size()>3?4:apid.size()+1);
        strncpy(extendedheader.ctid,ctid.toLatin1().constData(),ctid.size()>3?4:ctid.size()+1);
        extendedheader.msin = 0;
        if(mode == DltModeVerbose) {
            extendedheader.msin |= DLT_MSIN_VERB;
        }
        extendedheader.msin |= (((unsigned char)type) << 1) & DLT_MSIN_MSTP;
        extendedheader.msin |= (((unsigned char)subtype) << 4) & DLT_MSIN_MTIN;
        extendedheader.noar = numberOfArguments;
        header += QByteArray((const char *)&extendedheader,sizeof(DltExtendedHeader));
    }

    // set header size
    headerSize = header.size();

}
