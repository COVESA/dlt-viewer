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

QDltArgument::QDltArgument()
{
    // clear content of argument
    clear();
}

QDltArgument::~QDltArgument()
{

}

int QDltArgument::getOffsetPayload() const
{
    return offsetPayload;
}

QByteArray QDltArgument::getData() const
{
   return data;
}

QString QDltArgument::getName() const
{
    return name;
}

QString QDltArgument::getUnit() const
{
    return unit;
}

int QDltArgument::getDataSize() const
{
    return data.size();
}

QDltArgument::DltTypeInfoDef QDltArgument::getTypeInfo() const
{
    return typeInfo;
}

QString QDltArgument::getTypeInfoString() const
{
    if(typeInfo<0)
        return QString("");

    return QString(qDltTypeInfo[typeInfo]);
}

bool QDltArgument::setArgument(QByteArray &payload,unsigned int &offset,DltEndiannessDef _endianess)
{
    unsigned short length=0,length2=0,length3=0;

    /* clear old data */
    clear();

    /* store offset */
    offsetPayload = offset;

    /* store new endianness */
    endianness = _endianess;

    /* get type info */
    if((unsigned int)payload.size()<(offset+sizeof(unsigned int)))
        return false;
    if(endianness == DltEndiannessLittleEndian)
        dltType = *((unsigned int*) (payload.constData()+offset));
    else
        dltType = DLT_SWAP_32((*((unsigned int*) (payload.constData()+offset))));
    offset += sizeof(unsigned int);

    if (dltType& DLT_TYPE_INFO_STRG)
    {
        if ((dltType & DLT_TYPE_INFO_SCOD)==DLT_SCOD_UTF8)
        {
            typeInfo = DltTypeInfoUtf8;
        }
        else
        {
            typeInfo = DltTypeInfoStrg;
        }
    }
    else if (dltType & DLT_TYPE_INFO_BOOL)
    {
        typeInfo = DltTypeInfoBool;
    }
    else if (dltType & DLT_TYPE_INFO_SINT)
    {
        typeInfo = DltTypeInfoSInt;
    }
    else if (dltType & DLT_TYPE_INFO_UINT)
    {
        typeInfo = DltTypeInfoUInt;
    }
    else if (dltType & DLT_TYPE_INFO_FLOA)
    {
        typeInfo = DltTypeInfoFloa;
    }
    else if (dltType & DLT_TYPE_INFO_RAWD)
    {
        typeInfo = DltTypeInfoRawd;
    }
    else if (dltType & DLT_TYPE_INFO_TRAI)
    {
        typeInfo = DltTypeInfoTrai;
    }
    else
    {
        typeInfo = DltTypeInfoUnknown;
        return false;
    }

    /* get length of string, raw data or trace info */
    if(typeInfo == DltTypeInfoStrg || typeInfo == DltTypeInfoRawd || typeInfo == DltTypeInfoTrai || typeInfo == DltTypeInfoUtf8)
    {
        if((unsigned int)payload.size()<(offset+sizeof(unsigned short)))
            return false;
        if(endianness == DltEndiannessLittleEndian)
            length = *((unsigned short*) (payload.constData()+offset));
        else
            length = DLT_SWAP_16((*((unsigned short*) (payload.constData()+offset))));

        offset += sizeof(unsigned short);
    }

    /* get variable info */
    if(dltType & DLT_TYPE_INFO_VARI)
    {
        if((unsigned int)payload.size()<(offset+sizeof(unsigned short)))
            return false;
        if(endianness == DltEndiannessLittleEndian)
            length2 = *((unsigned short*) (payload.constData()+offset));
        else
            length2 = DLT_SWAP_16((*((unsigned short*) (payload.constData()+offset))));
        offset += sizeof(unsigned short);
        if(typeInfo == DltTypeInfoSInt || typeInfo == DltTypeInfoUInt || typeInfo == DltTypeInfoFloa)
        {
            if((unsigned int)payload.size()<(offset+sizeof(unsigned short)))
                return false;
            if(endianness == DltEndiannessLittleEndian)
                length3 = *((unsigned short*) (payload.constData()+offset));
            else
                length3 = DLT_SWAP_16((*((unsigned short*) (payload.constData()+offset))));
            offset += sizeof(unsigned short);
        }
        name = QString(payload.mid(offset,length2));
        offset += length2;
        if(typeInfo == DltTypeInfoSInt || typeInfo == DltTypeInfoUInt || typeInfo == DltTypeInfoFloa)
        {
            unit = QString(payload.mid(offset,length3));
            offset += length3;
        }
    }

    /* get fix point quantisation and offset */
    if(dltType & DLT_TYPE_INFO_FIXP)
    {
        /* not supported yet */
        return false;
    }

    /* get data */
    if(typeInfo == DltTypeInfoStrg || typeInfo == DltTypeInfoRawd || typeInfo == DltTypeInfoTrai || typeInfo == DltTypeInfoUtf8)
    {
        if((unsigned int)payload.size()<(offset+length))
            return false;
        data = payload.mid(offset,length);
        offset += length;
    }
    else if(typeInfo == DltTypeInfoBool)
    {
        data = payload.mid(offset,1);
        offset += 1;
    }
    else if(typeInfo == DltTypeInfoSInt || typeInfo == DltTypeInfoUInt)
    {
        switch (dltType & DLT_TYPE_INFO_TYLE)
        {
            case DLT_TYLE_8BIT:
            {
                data = payload.mid(offset,1);
                offset += 1;
                break;
            }
            case DLT_TYLE_16BIT:
            {
                data = payload.mid(offset,2);
                offset += 2;
                break;
            }
            case DLT_TYLE_32BIT:
            {
                data = payload.mid(offset,4);
                offset += 4;
                break;
            }
            case DLT_TYLE_64BIT:
            {
                data = payload.mid(offset,8);
                offset += 8;
                break;
            }
            case DLT_TYLE_128BIT:
            {
                data = payload.mid(offset,16);
                offset += 16;
                break;
            }
            default:
            {
                return false;
            }
        }

    }
    else if(typeInfo == DltTypeInfoFloa)
    {
        switch(dltType & DLT_TYPE_INFO_TYLE)
        {
            case DLT_TYLE_8BIT:
            {
                data = payload.mid(offset,1);
                offset += 1;
                break;
            }
            case DLT_TYLE_16BIT:
             {
                data = payload.mid(offset,2);
                offset += 2;
                break;
            }
            case DLT_TYLE_32BIT:
            {
                data = payload.mid(offset,4);
                offset += 4;
                break;
            }
            case DLT_TYLE_64BIT:
            {
                data = payload.mid(offset,8);
                offset += 8;
                break;
            }
            case DLT_TYLE_128BIT:
            {
                data = payload.mid(offset,16);
                offset += 16;
                break;
            }
            default:
            {
                return false;
            }
        }

    }

    return true;
}

bool QDltArgument::getArgument(QByteArray &payload, bool verboseMode) const
{
    unsigned int dltType = 0;
    bool appendSize = false;

    /* add the type info in verbose mode */
    if(verboseMode) {
        switch(typeInfo) {
        case DltTypeInfoUnknown:
            return false;
        case DltTypeInfoStrg:
            dltType |= DLT_TYPE_INFO_STRG;
            dltType |= DLT_SCOD_ASCII;
            appendSize = true;
            break;
        case DltTypeInfoUtf8:
            dltType |= DLT_TYPE_INFO_STRG;
            dltType |= DLT_SCOD_UTF8;
            appendSize = true;
            break;
        case DltTypeInfoBool:
            dltType |= DLT_TYPE_INFO_BOOL;
            break;
        case DltTypeInfoSInt:
            dltType |= DLT_TYPE_INFO_SINT;
            break;
        case DltTypeInfoUInt:
            dltType |= DLT_TYPE_INFO_UINT;
            break;
        case DltTypeInfoFloa:
            dltType |= DLT_TYPE_INFO_FLOA;
            break;
        case DltTypeInfoRawd:
            dltType |= DLT_TYPE_INFO_RAWD;
            appendSize = true;
            break;
        case DltTypeInfoTrai:
            // dltType |= DLT_TYPE_INFO_TRAI;
            return false;
        default:
            return false;
        }
        if((typeInfo == DltTypeInfoSInt) || (typeInfo == DltTypeInfoUInt) || (typeInfo == DltTypeInfoFloa) || (typeInfo == DltTypeInfoBool)) {
            switch(data.size())
            {
            case 1:
                dltType |= DLT_TYLE_8BIT;
                break;
            case 2:
                dltType |= DLT_TYLE_16BIT;
                break;
            case 4:
                dltType |= DLT_TYLE_32BIT;
                break;
            case 8:
                dltType |= DLT_TYLE_64BIT;
                break;
            case 16:
                dltType |= DLT_TYLE_128BIT;
                break;
            default:
                return false;
            }
        }
        payload += QByteArray((const char*)&dltType,sizeof(unsigned int));
    }

    /* add the string or raw data size to the payload */
    if(appendSize) {
        ushort size = data.size();
        payload += QByteArray((const char*)&size, sizeof(ushort));
    }

    /* add the value to the payload */
    payload += data;

    return true;
}

void QDltArgument::clear()
{
    typeInfo = QDltArgument::DltTypeInfoUnknown;
    offsetPayload = 0;
    data.clear();
    name.clear();
    unit.clear();
    endianness = QDltArgument::DltEndiannessUnknown;
    dltType = 0;
}

QString QDltArgument::toString(bool binary) const
{
    QString text;
    text.reserve(1024);

    if(binary) {
        return toAscii(data);
    }

    switch(getTypeInfo()) {
    case DltTypeInfoUnknown:
        text += QString("?");
        break;
    case DltTypeInfoStrg:
        if(data.size()) {
            text += QString("%1").arg(QString(getData()));
        }
        break;
    case DltTypeInfoUtf8:
        if(data.size()) {
            text += QString::fromUtf8(data.data());
        }
        break;
    case DltTypeInfoBool:
        if(data.size()) {
            if(data.constData()[0])
                text += QString("true");
            else
                text += QString("false");
        }
        else
            text += QString("?");
        break;
    case DltTypeInfoSInt:
        switch(data.size())
        {
        case 1:
            text += QString("%1").arg((short)(*(char*)(data.constData())));
            break;
        case 2:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((short)(*(short*)(data.constData())));
            else
                text += QString("%1").arg((short)DLT_SWAP_16((short)(*(short*)(data.constData()))));
            break;
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((int)(*(int*)(data.constData())));
            else
                text += QString("%1").arg((int)DLT_SWAP_32((int)(*(int*)(data.constData()))));
            break;
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((long long)(*(long long*)(data.constData())));
            else
                text += QString("%1").arg((long long)DLT_SWAP_64((long long)(*(long long*)(data.constData()))));
            break;
        default:
            text += QString("?");
        }

        break;
    case DltTypeInfoUInt:
        if ((dltType & DLT_TYPE_INFO_SCOD)==DLT_SCOD_BIN)
        {
            if((dltType & DLT_TYPE_INFO_TYLE)==DLT_TYLE_8BIT)
                text += toAscii(data,2,1); // show binary
            else if((dltType & DLT_TYPE_INFO_TYLE)==DLT_TYLE_16BIT)
                text += toAscii(data,2,2); // show binary
        }
        else if ((dltType & DLT_TYPE_INFO_SCOD)==DLT_SCOD_HEX)
        {
            if((dltType & DLT_TYPE_INFO_TYLE)==DLT_TYLE_8BIT)
                text += toAscii(data,0,1); // show 8 bit hex
            else if((dltType & DLT_TYPE_INFO_TYLE)==DLT_TYLE_16BIT)
                text += toAscii(data,0,2); // show 16 bit hex
            else if((dltType & DLT_TYPE_INFO_TYLE)==DLT_TYLE_32BIT)
                text += toAscii(data,0,4); // show 32 bit hex
            else if((dltType & DLT_TYPE_INFO_TYLE)==DLT_TYLE_64BIT)
                text += toAscii(data,0,8); // show 64 bit hex
        }
        else
        {
            switch(data.size())
            {
            case 1:
                text += QString("%1").arg((unsigned short)(*(unsigned char*)(data.constData())));
                break;
            case 2:
                if(endianness == DltEndiannessLittleEndian)
                    text += QString("%1").arg((unsigned short)(*(unsigned short*)(data.constData())));
                else
                    text += QString("%1").arg((unsigned short)DLT_SWAP_16((unsigned short)(*(unsigned short*)(data.constData()))));
                break;
            case 4:
                if(endianness == DltEndiannessLittleEndian)
                    text += QString("%1").arg((unsigned int)(*(unsigned int*)(data.constData())));
                else
                    text += QString("%1").arg((unsigned int)DLT_SWAP_32((unsigned int)(*(unsigned int*)(data.constData()))));
                break;
            case 8:
                if(endianness == DltEndiannessLittleEndian)
                    text += QString("%1").arg((unsigned long long)(*(unsigned long long*)(data.constData())));
                else
                    text += QString("%1").arg((unsigned long long)DLT_SWAP_64((unsigned long long)(*(unsigned long long*)(data.constData()))));
                break;
            default:
                text += QString("?");
            }
        }
        break;
    case DltTypeInfoFloa:
        switch(data.size())
        {
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((double)(*(float*)(data.constData())));
            else
            {
                unsigned int tmp;
                tmp = DLT_SWAP_32((unsigned int)(*(unsigned int*)(data.constData())));
                void *buf = (void *) &tmp;
                text += QString("%1").arg((double)(*((float*)buf)));
            }
            break;
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                text += QString("%1").arg((double)(*(double*)(data.constData())));
            else {
                unsigned int tmp;
                tmp = DLT_SWAP_64((unsigned long long)(*(unsigned long long*)(data.constData())));
                void *buf = (void *) &tmp;
                text += QString("%1").arg((double)(*((double*)buf)));
            }
            break;
        default:
            text += QString("?");
        }
        break;
    case DltTypeInfoRawd:
        text += toAscii(data,0); // show raw format (no leading 0x)
        break;
    case DltTypeInfoTrai:
        text += QString("?");
        break;
    default:
        text += QString("?");
    }

    return text;
}

QVariant QDltArgument::getValue() const
{
    switch(typeInfo) {
    case DltTypeInfoUnknown:
        break;
    case DltTypeInfoStrg:
        if(data.size()) {
            return QVariant(QString(getData()));
        }
        break;
    case DltTypeInfoUtf8:
        if(data.size()) {
            return QVariant(QString::fromUtf8(data.data()));
        }
        break;
    case DltTypeInfoBool:
        if(data.size()) {
            return QVariant((bool)(data.constData()[0]));
        }
        break;
    case DltTypeInfoSInt:
        switch(data.size())
        {
        case 1:
            return QVariant((short)(*(char*)(data.constData())));
        case 2:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((short)(*(short*)(data.constData())));
            else
                return QVariant(DLT_SWAP_16((short)(*(short*)(data.constData()))));
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((int)(*(int*)(data.constData())));
            else
                return QVariant(DLT_SWAP_32((int)(*(int*)(data.constData()))));
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((long long)(*(long long*)(data.constData())));
            else
                return QVariant(DLT_SWAP_64((long long)(*(long long*)(data.constData()))));
        default:
            break;
        }
        break;
    case DltTypeInfoUInt:
        switch(data.size())
        {
        case 1:
            return QVariant((unsigned short)(*(unsigned char*)(data.constData())));
        case 2:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((unsigned short)(*(unsigned short*)(data.constData())));
            else
                return QVariant(DLT_SWAP_16((unsigned short)(*(unsigned short*)(data.constData()))));
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((unsigned int)(*(unsigned int*)(data.constData())));
            else
                return QVariant(DLT_SWAP_32((unsigned int)(*(unsigned int*)(data.constData()))));
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((unsigned long long)(*(unsigned long long*)(data.constData())));
            else
                return QVariant(DLT_SWAP_64((unsigned long long)(*(unsigned long long*)(data.constData()))));
        default:
            break;
        }

        break;
    case DltTypeInfoFloa:
        switch(data.size())
        {
        case 4:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((double)(*(float*)(data.constData())));
            else
            {
                unsigned int tmp;
                tmp = DLT_SWAP_32((unsigned int)(*(unsigned int*)(data.constData())));
                void *buf = (void*) &tmp;
                return QVariant((double)(*((float*)buf)));
            }
        case 8:
            if(endianness == DltEndiannessLittleEndian)
                return QVariant((double)(*(double*)(data.constData())));
            else {
                unsigned int tmp;
                tmp = DLT_SWAP_64((unsigned long long)(*(unsigned long long*)(data.constData())));
                void *buf = (void*) &tmp;
                return QVariant((double)(*((double*)buf)));
            }
        default:
            break;
        }
        break;
    case DltTypeInfoRawd:
        return QVariant(data);
        break;
    case DltTypeInfoTrai:
        break;
    default:
        break;
    }

    return QVariant();
}

bool QDltArgument::setValue(QVariant value, bool verboseMode)
{
    Q_UNUSED(verboseMode);

    endianness = QDltArgument::DltEndiannessLittleEndian;

    switch(value.type())
    {
    case QVariant::ByteArray:
        data = value.toByteArray();
        typeInfo = QDltArgument::DltTypeInfoRawd;
        return true;
    case QVariant::String:
        data = value.toByteArray();
        typeInfo = QDltArgument::DltTypeInfoStrg;
        return true;
    case QVariant::Bool:
        {
        bool bvalue = value.toBool();
        unsigned char cvalue = bvalue;
        data = QByteArray((const char*)&cvalue,sizeof(unsigned char));
        typeInfo = QDltArgument::DltTypeInfoSInt;
        return true;
        }
        break;
    case QVariant::Int:
        {
        int bvalue = value.toInt();
        data = QByteArray((const char*)&bvalue,sizeof(int));
        typeInfo = QDltArgument::DltTypeInfoSInt;
        return true;
        }
    case QVariant::LongLong:
        {
        long long bvalue = value.toLongLong();
        data = QByteArray((const char*)&bvalue,sizeof(long long));
        typeInfo = QDltArgument::DltTypeInfoSInt;
        return true;
        }
    case QVariant::UInt:
        {
        unsigned int bvalue = value.toUInt();
        data = QByteArray((const char*)&bvalue,sizeof(int));
        typeInfo = QDltArgument::DltTypeInfoUInt;
        return true;
        }
    case QVariant::ULongLong:
        {
        unsigned long long bvalue = value.toULongLong();
        data = QByteArray((const char*)&bvalue,sizeof(unsigned long long));
        typeInfo = QDltArgument::DltTypeInfoUInt;
        return true;
        }
    case QVariant::Double:
        {
        double bvalue = value.toInt();
        data = QByteArray((const char*)&bvalue,sizeof(double));
        typeInfo = QDltArgument::DltTypeInfoFloa;
        return true;
        }
        break;
    default:
        break;
    }

    return false;
}

