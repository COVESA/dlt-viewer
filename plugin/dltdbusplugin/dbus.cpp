/**
 * @licence app begin@
 * Copyright (C) 2011-2014  BMW AG
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
 * \file dbus.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QDebug>
#include <QString>

#include "qdlt.h"

#include "dbus.h"

const char *dbus_message_type[]={"INVALID","METHOD_CALL","METHOD_RETURN","ERROR","SIGNAL"};
const char *dbus_message_type_short[]={"INVALID","C","R","ERROR","S"};
const char *dbus_field_code[]={"INVALID","PATH","INTERFACE","MEMBER","NAME","REPLY_SERIAL","DESTINATION","SENDER","SIGNATURE","UNIX_FDS"};
const char *dbus_type_string[]={"INVALID","BYTE","BOOLEAN","INT16","UINT16","INT32","UINT32","INT64","UINT64","DOUBLE","STRING","OBJECT_PATH","SIGNATURE","ARRAY","STRUCT","STRUCT_BEGIN","STRUCT_END","VARIANT","DICT_ENTRY","DICT_ENTRY_BEGIN","DICT_ENTRY_END","UNIX_FD"};

QString DltDBusParameter::getTypeString()
{
    if(type<=DBUS_TYPE_MAX)
    {
        return QString(dbus_type_string[(int)type]);
    }
    else
    {
        return QString("invalid");
    }
}

DltDBusDecoder::DltDBusDecoder()
{
    endianess = 0;
    messageType = 0;
    flags = 0;
    version = 0;
    payloadLength = 0;
    serial = 0;
    arrayLength = 0;
    replySerial = 0;
    unixFds = 0;
}

DltDBusDecoder::~DltDBusDecoder()
{

}

QString DltDBusDecoder::getEndianessString()
{
    if(endianess==DBUS_ENDIANESS_LITTLE)
        return QString("Little Endianess");
    else if(endianess==DBUS_ENDIANESS_BIG)
        return QString("Big Endianess");
    else
        return QString("unknown");
}

QString DltDBusDecoder::getMessageTypeString()
{
    if(messageType<=DBUS_MESSAGE_TYPE_MAX)
    {
        return QString(dbus_message_type[messageType]);
    }
    else
    {
        return QString("invalid");
    }
}

QString DltDBusDecoder::getMessageTypeStringShort()
{
    if(messageType<=DBUS_MESSAGE_TYPE_MAX)
    {
        return QString(dbus_message_type_short[messageType]);
    }
    else
    {
        return QString("invalid");
    }
}

QString DltDBusDecoder::getFlagsString()
{
    QString text;
    if(flags&DBUS_FLAG_NO_REPLY_EXPECTED)
            text += "no reply expected ";
    if(flags&DBUS_FLAG_NO_AUTO_START)
    {
        if(!text.isEmpty())
            text += "|";
        text += "no auto start";
    }
    return text;
}

QString DltDBusDecoder::getFieldCodeString(unsigned char fieldCode)
{
    if(fieldCode<=DBUS_FIELD_CODE_MAX)
    {
        return QString(dbus_field_code[fieldCode]);
    }
    else
    {
        return QString("invalid");
    }
}

int DltDBusDecoder::padding(uint32_t pos,int alignement)
{
    if((pos % alignement)==0)
        return 0;
    return alignement-(pos % alignement);
}

bool DltDBusDecoder::decode(QByteArray &data,bool headerOnly)
{
    if(!decodeHeader(data))
        return false;

    if(headerOnly)
        return true;

    return decodePayload();
}

bool DltDBusDecoder::decodeHeader(QByteArray &data)
{
    int offset = 0;
    char* dataPtr = data.data();

    // check header length
    if(data.size()<DBUS_HEADER_MAX)
    {
        error = QString("decodeHeader: size error no header!");
        return false;
    }

    // read header parameter
    if(!readByte(endianess,dataPtr,offset,DBUS_HEADER_MAX))
        return false;
    if(!readByte(messageType,dataPtr,offset,DBUS_HEADER_MAX))
        return false;
    if(!readByte(flags,dataPtr,offset,DBUS_HEADER_MAX))
        return false;
    if(!readByte(version,dataPtr,offset,DBUS_HEADER_MAX))
        return false;
    if(!readUint32(payloadLength,dataPtr,offset,DBUS_HEADER_MAX))
        return false;
    if(!readUint32(serial,dataPtr,offset,DBUS_HEADER_MAX))
        return false;
    
    // check array size
    if(data.size()<(DBUS_HEADER_MAX+4))
    {
        error = QString("decodeHeader: size error: no header array length!");
        return false;
    }

    // read header array length
    if(!readUint32(arrayLength,dataPtr,offset,DBUS_HEADER_MAX+4))
        return false;

    // check array size
    if((payloadLength+arrayLength+DBUS_HEADER_MAX+4)>static_cast<uint32_t>(data.size()))
    {
        error = QString("decodeHeader: size error: message too short!");
        return false;
    }

    // calculate offsets and sizes
    int arrayOffset = DBUS_HEADER_MAX+4;
    int payloadOffset = data.size()-payloadLength;
    int arraySize = data.size()-payloadLength-arrayOffset;

    // get payload
    payload = QByteArray((data.data())+payloadOffset,payloadLength);

    // move to header array
    dataPtr = data.data() + DBUS_HEADER_MAX + 4;
    offset = 0;

    // read all header array field parameters
    while(offset<(int)arrayLength)
    {
        // padding for structure element
        offset+=padding(offset,8);

        // read field code
        unsigned char fieldCode = 0;
        if(!readByte(fieldCode,dataPtr,offset,arraySize))
            return false;

        // read variant signature
        QByteArray variantSignature;
        if(!readSignature(variantSignature,dataPtr,offset,arraySize))
            return false;

        // check variant signature size
        if(variantSignature.size()!=1)
        {
            error = QString("decodeHeader: wrong variant signature size");
            return false;
        }

        //qDebug() << "Field: " << offset << fieldCode << variantSignature.size() << (char)variantSignature[0];

        switch (fieldCode)
        {
            case DBUS_FIELD_CODE_PATH: //	1           // OBJECT_PATH	METHOD_CALL, SIGNAL	The object to send a call to, or the object a signal is emitted from. The special path /org/freedesktop/DBus/Local is reserved; implementations should not send messages with this path, and the reference implementation of the bus daemon will disconnect any application that attempts to do so.
                // read path
                if(!readString(path,dataPtr,offset,arraySize))
                    return false;
                break;
            case DBUS_FIELD_CODE_INTERFACE: //	2       // STRING	SIGNAL	The interface to invoke a method call on, or that a signal is emitted from. Optional for method calls, required for signals. The special interface org.freedesktop.DBus.Local is reserved; implementations should not send messages with this interface, and the reference implementation of the bus daemon will disconnect any application that attempts to do so.
                // read interface name
                if(!readString(interfaceName,dataPtr,offset,arraySize))
                    return false;
                break;
            case DBUS_FIELD_CODE_MEMBER: //	3           // STRING	METHOD_CALL, SIGNAL	The member, either the method name or signal name.
                // read member
                if(!readString(member,dataPtr,offset,arraySize))
                    return false;
                break;
            case DBUS_FIELD_CODE_ERROR_NAME: //	4       // STRING	ERROR	The name of the error that occurred, for errors
                // read error name
                if(!readString(errorName,dataPtr,offset,arraySize))
                    return false;
                break;
            case DBUS_FIELD_CODE_REPLY_SERIAL: //	5	// UINT32	ERROR, METHOD_RETURN	The serial number of the message this message is a reply to. (The serial number is the second UINT32 in the header.)
                // read reply serial
                if(!readUint32(replySerial,dataPtr,offset,arraySize))
                    return false;
                break;
            case DBUS_FIELD_CODE_DESTINATION: //	6       // STRING	optional	The name of the connection this message is intended for. Only used in combination with the message bus, see the section called "Message Bus Specification".
                // read destination
                if(!readString(destination,dataPtr,offset,arraySize))
                    return false;
                break;
            case DBUS_FIELD_CODE_SENDER: //	7           // STRING	optional	Unique name of the sending connection. The message bus fills in this field so it is reliable; the field is only meaningful in combination with the message bus.
                // read sender
                if(!readString(sender,dataPtr,offset,arraySize))
                    return false;
                break;
            case DBUS_FIELD_CODE_SIGNATURE: //	8       // SIGNATURE	optional	The signature of the message body. If omitted, it is assumed to be the empty signature "" (i.e. the body must be 0-length).
                // read signature
                if(!readSignature(signature,dataPtr,offset,arraySize))
                    return false;
                break;
            case DBUS_FIELD_CODE_UNIX_FDS: //	9       // UINT32	optional	The number of Unix file descriptors that accompany the message. If omitted, it is assumed that no Unix file descriptors accompany the message. The actual file descriptors need to be transferred via platform specific mechanism out-of-band. They must be sent at the same time as part of the message itself. They may not be sent before the first byte of the message itself is transferred or after the last byte of the message itself.
                // read unix fds
                if(!readUint32(unixFds,dataPtr,offset,arraySize))
                    return false;
                break;
            default:
                error = QString("unsupported field code: %1").arg((int)fieldCode);
                return false;
                break;
        }
        offset+=padding(offset,8);
    }

    return true;
}

int DltDBusDecoder::indexOfCascaded(QByteArray signature,char ch, char cascade, int from)
{
    int level = 1;

    // qDebug() << "indexOfCascaded: " << QString(signature.mid(from)) << ch << cascade;

    for(int num=from;num<signature.size();num++)
    {
        if(signature[num]==cascade)
        {
            level += 1;
        }
        else if(signature[num]==ch)
        {
            level -= 1;
        }

        if(level == 0)
            return num;
    }

    // end not found
    return -1;
}

bool DltDBusDecoder::decodePayload()
{
    int offset = 0;
    char* dataPtr = payload.data();

    parameters.clear();

    // qDebug() << "Decode Payload: " << payload.size() << QString(signature);

    return decodePayloadSignature(signature,dataPtr,offset,payload.size());
}

bool DltDBusDecoder::decodePayloadSignature(QByteArray signature,char *dataPtr,int &offset,int maxSize)
{
    DltDBusParameter parameter;
    uint32_t lengthArray,lengthArray2;
    bool isArray;

    // qDebug() << "decodePayloadSignature: " << signature.size() << QString(signature);

    // run through the whole signature
    for(int num=0;num<signature.size();)
    {

        if(offset>=payload.size())
        {
            error = QString("decodePayloadSignature: Payload length error!");
            return false;
        }

        isArray = false;
        if(signature[num]==(char)DBUS_TYPE_CHAR_ARRAY)
        {
            // we are in an array
            isArray = true;
            if(!readUint32(lengthArray,dataPtr,offset,maxSize))
                return false;
            if(num >= (signature.size()-1))
            {
                error = QString("decodePayloadSignature: Array length error!");
                return false;
            }
            num++;
            if(num>=signature.size())
                // end already reached
                break;
            // check if two dimensional array
            if(signature[num]==(char)DBUS_TYPE_CHAR_ARRAY)
            {
                // we are in an array
                int lastOffset = offset;
                isArray = true;
                if(!readUint32(lengthArray2,dataPtr,offset,maxSize))
                    return false;
                if(num >= (signature.size()-1))
                {
                    error = QString("decodePayloadSignature: Array length error!");
                    return false;
                }
                //qDebug() << "lengthArray:" << lengthArray << lengthArray2;
                lengthArray = lengthArray - (offset-lastOffset);
                num++;
                if(num>=signature.size())
                    // end already reached
                    break;
            }
        }

        // qDebug() << "Type:" << num << lengthArray << signature[num] << offset ;

        if(signature[num]==(char)DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN || signature[num]==(char)DBUS_TYPE_CHAR_STRUCT_BEGIN)
        {
            // first find end of dictonary
            int posFoundEnd;
            if(signature[num]==(char)DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN)
                posFoundEnd=indexOfCascaded(signature,DBUS_TYPE_CHAR_DICT_ENTRY_END,DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN,num+1);
            else
                posFoundEnd=indexOfCascaded(signature,DBUS_TYPE_CHAR_STRUCT_END,DBUS_TYPE_CHAR_STRUCT_BEGIN,num+1);
            if(posFoundEnd==-1)
            {
                if(signature[num]==(char)DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN)
                    error = QString("decodePayloadSignature: Dictonary end not found!");
                else
                    error = QString("decodePayloadSignature: Struct end not found!");
                return false;
            }
            if(!isArray)
            {
                // this is no array
                offset+=padding(offset,8);
                if(signature[num]==(char)DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN)
                {
                    parameter.setType(DBUS_TYPE_DICT_ENTRY);
                    parameter.setValue(QVariant(QString('{')));
                }
                else
                {
                    parameter.setType(DBUS_TYPE_STRUCT);
                    parameter.setValue(QVariant(QString('(')));
                }
                parameters.append(parameter);
                if(!decodePayloadSignature(signature.mid(num+1,posFoundEnd-num-1),dataPtr,offset,maxSize))
                     return false;
                if(signature[num]==(char)DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN)
                {
                    parameter.setType(DBUS_TYPE_DICT_ENTRY);
                    parameter.setValue(QVariant(QString('}')));
                }
                else
                {
                    parameter.setType(DBUS_TYPE_STRUCT);
                    parameter.setValue(QVariant(QString(')')));
                }
                parameters.append(parameter);
            }
            else
            {
                // this is an array
                int lastOffset = offset;
                offset+=padding(offset,8);
                parameter.setType(DBUS_TYPE_ARRAY);
                parameter.setValue(QVariant(QString('[')));
                parameters.append(parameter);
                while(offset < static_cast<int>(lastOffset+lengthArray))
                {
                    offset+=padding(offset,8);
                    if(signature[num]==(char)DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN)
                    {
                        parameter.setType(DBUS_TYPE_DICT_ENTRY);
                        parameter.setValue(QVariant(QString('{')));
                    }
                    else
                    {
                        parameter.setType(DBUS_TYPE_STRUCT);
                        parameter.setValue(QVariant(QString('(')));
                    }
                    parameters.append(parameter);
                    if(!decodePayloadSignature(signature.mid(num+1,posFoundEnd-num-1),dataPtr,offset,maxSize))
                         return false;
                    if(signature[num]==(char)DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN)
                    {
                        parameter.setType(DBUS_TYPE_DICT_ENTRY);
                        parameter.setValue(QVariant(QString('}')));
                    }
                    else
                    {
                        parameter.setType(DBUS_TYPE_STRUCT);
                        parameter.setValue(QVariant(QString(')')));
                    }
                    parameters.append(parameter);
                }
                parameter.setType(DBUS_TYPE_ARRAY);
                parameter.setValue(QVariant(QString(']')));
                parameters.append(parameter);
            }
            num+=posFoundEnd-num+1;
        }
        else
        {
            // this is no structure or dictionary
            if(!isArray)
            {
                // this no array
                if(!decodePayloadParameter(signature[num],dataPtr,offset,payload.size()))
                    return false;
            }
            else
            {
                // this is an array
                int lastOffset = offset;
                parameter.setType(DBUS_TYPE_ARRAY);
                parameter.setValue(QVariant(QString('[')));
                parameters.append(parameter);
                while(offset < static_cast<int>(lastOffset+lengthArray))
                {
                    if(!decodePayloadParameter(signature[num],dataPtr,offset,payload.size()))
                        return false;
                }
                parameter.setType(DBUS_TYPE_ARRAY);
                parameter.setValue(QVariant(QString(']')));
                parameters.append(parameter);
            }

            num++;
        }
    }
    return true;

}

bool DltDBusDecoder::decodePayloadParameter(char type,char *dataPtr,int &offset,int maxSize)
{
    DltDBusParameter parameter;

    switch(type)
    {
    case DBUS_TYPE_CHAR_INVALID: //	0 // (ASCII NUL)	Not a valid type code, used to terminate signatures
        error = QString("Invalid argument!");
        return false;
        break;
    case DBUS_TYPE_CHAR_BYTE: // 121 // (ASCII 'y')	8-bit unsigned integer
    {
        unsigned char data = 0;

        // read parameter
        if(!readByte(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_BYTE);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_BOOLEAN: //	98 // (ASCII 'b')	Boolean value, 0 is FALSE and 1 is TRUE. Everything else is invalid.
    {
        bool data;

        // read parameter
        if(!readBoolean(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_BOOLEAN);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_INT16: //	110 // (ASCII 'n')	16-bit signed integer
    {
        int16_t data;

        // read parameter
        if(!readInt16(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_INT16);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_UINT16: //	113 // (ASCII 'q')	16-bit unsigned integer
    {
        uint16_t data;

        // read parameter
        if(!readUint16(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_UINT16);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_INT32: //	105 // (ASCII 'i')	32-bit signed integer
    {
        int32_t data;

        // read parameter
        if(!readInt32(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_INT32);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_UINT32: //	117 // (ASCII 'u')	32-bit unsigned integer
    {
        uint32_t data;

        // read parameter
        if(!readUint32(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_UINT32);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_INT64: //	120 // (ASCII 'x')	64-bit signed integer
    {
        int64_t data;

        // read parameter
        if(!readInt64(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_INT64);
        parameter.setValue(QVariant((qint64)data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_UINT64: //	116 // (ASCII 't')	64-bit unsigned integer
    {
        uint64_t data;

        // read parameter
        if(!readUint64(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_UINT64);
        parameter.setValue(QVariant((quint64)data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_DOUBLE: //	100 // (ASCII 'd')	IEEE 754 double
    {
        double data;

        // read parameter
        if(!readDouble(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_DOUBLE);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_STRING: //	115 // (ASCII 's')	UTF-8 string (must be valid UTF-8). Must be nul terminated and contain no other nul bytes.
    {
        QString data;

        // read parameter
        if(!readString(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_STRING);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_OBJECT_PATH: //	111 // (ASCII 'o')	Name of an object instance
    {
        QString data;

        // read parameter
        if(!readString(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_OBJECT_PATH);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_SIGNATURE: //	103 // (ASCII 'g')	A type signature
    {
        QByteArray data;

        // read parameter
        if(!readSignature(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_SIGNATURE);
        parameter.setValue(QVariant(QString(data)));
        parameters.append(parameter);

        break;
    }
    case DBUS_TYPE_CHAR_ARRAY: //	97 // (ASCII 'a')	Array
        error = QString("Multi dimension array not supported yet!");
        return false;
        break;
    case DBUS_TYPE_CHAR_STRUCT: //	114 // (ASCII 'r'),
        error = QString("Struct type r invalid!");
        return false;
        break;
    case DBUS_TYPE_CHAR_STRUCT_BEGIN: // 40 // (ASCII '('),
        error = QString("Struct not supported yet!");
        return false;
        break;
    case DBUS_TYPE_CHAR_STRUCT_END: // 41 // (ASCII ')')	Struct; type code 114 'r' is reserved for use in bindings and implementations to represent the general concept of a struct, and must not appear in signatures used on D-Bus.
        error = QString("Struct not supported yet!");
        return false;
        break;
    case DBUS_TYPE_CHAR_VARIANT: //	118 // (ASCII 'v') 	Variant type (the type of the value is part of the value itself)
    {
        QByteArray data;

        // read parameter
        if(!readSignature(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_VARIANT);
        parameter.setValue(QVariant(QString(data)));
        parameters.append(parameter);

        if(!decodePayloadSignature(data,dataPtr,offset,maxSize))
            return false;

        break;
    }
    case DBUS_TYPE_CHAR_DICT_ENTRY: //	101 // (ASCII 'e'),
        error = QString("Dictonary entry type e invalid!");
        return false;
        break;
    case DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN: // 123 // (ASCII '{'),
        error = QString("Dictonary not supported yet!");
        return false;
        break;
    case DBUS_TYPE_CHAR_DICT_ENTRY_END: // 125 // (ASCII '}') 	Entry in a dict or map (array of key-value pairs). Type code 101 'e' is reserved for use in bindings and implementations to represent the general concept of a dict or dict-entry, and must not appear in signatures used on D-Bus.
        error = QString("Dictonary not supported yet!");
        return false;
        break;
    case DBUS_TYPE_CHAR_UNIX_FD: //	104 // (ASCII 'h')	Unix file descriptor
    {
        uint32_t data;

        // read parameter
        if(!readUint32(data,dataPtr,offset,maxSize))
            return false;

        // add parameter
        parameter.setType(DBUS_TYPE_UNIX_FD);
        parameter.setValue(QVariant(data));
        parameters.append(parameter);

        break;
    }
    default:
        break;
    }

    return true;
}

bool DltDBusDecoder::readString(QString &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,4);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+4) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // read length of string
    uint32_t length = (*((uint32_t*)(dataPtr+offset)));

    // increase offset by length
    offset += 4;

    // check if length fits size, including termination byte zero
    if(static_cast<int>(offset+length+1) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // read string
    //data = QString::fromLatin1(dataPtr+offset,length);
    data= QString::fromUtf8(QByteArray(dataPtr+offset,length));

    // increase offset by size plus termination byte zero
    offset+=length+1;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readSignature(QByteArray &data,char *dataPtr,int &offset,int maxSize)
{
    // check if length fits size
    if((offset+1) > maxSize)
    {
        error = QString("readSignature: length check error");
        return false;
    }

    // read length of string
    unsigned char length = (*((unsigned char*)(dataPtr+offset)));

    // increase offset by length
    offset += 1;

    // check if length fits size, including termination byte zero
    if((offset+length+1) > maxSize)
    {
        error = QString("readSignature: length check error");
        return false;
    }

    // read string
    data = QByteArray(dataPtr+offset,length);

    // increase offset by size plus termination byte zero
    offset+=length+1;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readByte(unsigned char &data,char *dataPtr,int &offset,int maxSize)
{
    // check if length fits size
    if((offset+1) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read byte
    data = (*((unsigned char*)(dataPtr+offset)));

    // increase offset by size
    offset+=1;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readBoolean(bool &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,4);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+4) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read uint32, convert to bool
    data = (bool)(*((uint32_t*)(dataPtr+offset)));

    // increase offset by size
    offset+=4;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readInt16(int16_t &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,2);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+2) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read int16
    data = (*((int16_t*)(dataPtr+offset)));

    // increase offset by size
    offset+=2;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readUint16(uint16_t &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,2);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+2) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read uint16
    data = (*((uint16_t*)(dataPtr+offset)));

    // increase offset by size
    offset+=2;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readInt32(int32_t &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,4);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+4) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read int32
    data = (*((int32_t*)(dataPtr+offset)));

    // increase offset by size
    offset+=4;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readUint32(uint32_t &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,4);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+4) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read uint32
    data = (*((uint32_t*)(dataPtr+offset)));

    // increase offset by size
    offset+=4;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readInt64(int64_t &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,8);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+8) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read int64
    data = (*((int64_t*)(dataPtr+offset)));

    // increase offset by size
    offset+=8;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readUint64(uint64_t &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,8);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+8) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read uint64
    data = (*((uint64_t*)(dataPtr+offset)));

    // increase offset by size
    offset+=8;

    // everything worked fine
    return true;
}

bool DltDBusDecoder::readDouble(double &data,char *dataPtr,int &offset,int maxSize)
{
    // calculate padding bytes
    int paddingBytes = padding(offset,8);

    // check if padding bytes fits size
    if((offset+paddingBytes) > maxSize)
    {
        error = QString("readString: length check error");
        return false;
    }

    // increase offset by padding bytes
    offset += paddingBytes;

    // check if length fits size
    if((offset+8) > maxSize)
    {
        error = QString("readByte: length check error");
        return false;
    }

    // read double
    data = (*((double*)(dataPtr+offset)));

    // increase offset by size
    offset+=8;

    // everything worked fine
    return true;
}
