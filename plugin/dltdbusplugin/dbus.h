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
 * \file dbus.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef DLT_DBUS_DECODER_H
#define DLT_DBUS_DECODER_H

#include <QByteArray>
#include <QString>
#include <QList>

#if defined(_MSC_VER)
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "qdlt.h"

#define DBUS_HEADER_ENDIANESS 0
#define DBUS_HEADER_MESSAGE_TYPE 1
#define DBUS_HEADER_FLAGS 2
#define DBUS_HEADER_VERSION 3
#define DBUS_HEADER_LENGTH 4
#define DBUS_HEADER_SERIAL 8
#define DBUS_HEADER_MAX 12

#define DBUS_ENDIANESS_LITTLE 'l'
#define DBUS_ENDIANESS_BIG 'B'

#define DBUS_MESSAGE_TYPE_INVALID	0       // This is an invalid type.
#define DBUS_MESSAGE_TYPE_METHOD_CALL	1	// Method call.
#define DBUS_MESSAGE_TYPE_METHOD_RETURN	2	// Method reply with returned data.
#define DBUS_MESSAGE_TYPE_ERROR	3           // Error reply. If the first argument exists and is a string, it is an error message.
#define DBUS_MESSAGE_TYPE_SIGNAL	4       // Signal emission.
#define DBUS_MESSAGE_TYPE_MAX 4

#define DBUS_FLAG_NO_REPLY_EXPECTED 1
#define DBUS_FLAG_NO_AUTO_START 2

#define DBUS_FIELD_CODE_INVALID	0           // N/A	not allowed	Not a valid field name (error if it appears in a message)
#define DBUS_FIELD_CODE_PATH	1           // OBJECT_PATH	METHOD_CALL, SIGNAL	The object to send a call to, or the object a signal is emitted from. The special path /org/freedesktop/DBus/Local is reserved; implementations should not send messages with this path, and the reference implementation of the bus daemon will disconnect any application that attempts to do so.
#define DBUS_FIELD_CODE_INTERFACE	2       // STRING	SIGNAL	The interface to invoke a method call on, or that a signal is emitted from. Optional for method calls, required for signals. The special interface org.freedesktop.DBus.Local is reserved; implementations should not send messages with this interface, and the reference implementation of the bus daemon will disconnect any application that attempts to do so.
#define DBUS_FIELD_CODE_MEMBER	3           // STRING	METHOD_CALL, SIGNAL	The member, either the method name or signal name.
#define DBUS_FIELD_CODE_ERROR_NAME	4       // STRING	ERROR	The name of the error that occurred, for errors
#define DBUS_FIELD_CODE_REPLY_SERIAL	5	// UINT32	ERROR, METHOD_RETURN	The serial number of the message this message is a reply to. (The serial number is the second UINT32 in the header.)
#define DBUS_FIELD_CODE_DESTINATION	6       // STRING	optional	The name of the connection this message is intended for. Only used in combination with the message bus, see the section called “Message Bus Specification”.
#define DBUS_FIELD_CODE_SENDER	7           // STRING	optional	Unique name of the sending connection. The message bus fills in this field so it is reliable; the field is only meaningful in combination with the message bus.
#define DBUS_FIELD_CODE_SIGNATURE	8       // SIGNATURE	optional	The signature of the message body. If omitted, it is assumed to be the empty signature "" (i.e. the body must be 0-length).
#define DBUS_FIELD_CODE_UNIX_FDS	9       // UINT32	optional	The number of Unix file descriptors that accompany the message. If omitted, it is assumed that no Unix file descriptors accompany the message. The actual file descriptors need to be transferred via platform specific mechanism out-of-band. They must be sent at the same time as part of the message itself. They may not be sent before the first byte of the message itself is transferred or after the last byte of the message itself.
#define DBUS_FIELD_CODE_MAX 9

#define DBUS_TYPE_CHAR_INVALID	0 // (ASCII NUL)	Not a valid type code, used to terminate signatures
#define DBUS_TYPE_CHAR_BYTE	    121 // (ASCII 'y')	8-bit unsigned integer
#define DBUS_TYPE_CHAR_BOOLEAN	98 // (ASCII 'b')	Boolean value, 0 is FALSE and 1 is TRUE. Everything else is invalid.
#define DBUS_TYPE_CHAR_INT16	110 // (ASCII 'n')	16-bit signed integer
#define DBUS_TYPE_CHAR_UINT16	113 // (ASCII 'q')	16-bit unsigned integer
#define DBUS_TYPE_CHAR_INT32	105 // (ASCII 'i')	32-bit signed integer
#define DBUS_TYPE_CHAR_UINT32	117 // (ASCII 'u')	32-bit unsigned integer
#define DBUS_TYPE_CHAR_INT64	120 // (ASCII 'x')	64-bit signed integer
#define DBUS_TYPE_CHAR_UINT64	116 // (ASCII 't')	64-bit unsigned integer
#define DBUS_TYPE_CHAR_DOUBLE	100 // (ASCII 'd')	IEEE 754 double
#define DBUS_TYPE_CHAR_STRING	115 // (ASCII 's')	UTF-8 string (must be valid UTF-8). Must be nul terminated and contain no other nul bytes.
#define DBUS_TYPE_CHAR_OBJECT_PATH	111 // (ASCII 'o')	Name of an object instance
#define DBUS_TYPE_CHAR_SIGNATURE	103 // (ASCII 'g')	A type signature
#define DBUS_TYPE_CHAR_ARRAY	97 // (ASCII 'a')	Array
#define DBUS_TYPE_CHAR_STRUCT	114 // (ASCII 'r'),
#define DBUS_TYPE_CHAR_STRUCT_BEGIN 40 // (ASCII '('),
#define DBUS_TYPE_CHAR_STRUCT_END 41 // (ASCII ')')	Struct; type code 114 'r' is reserved for use in bindings and implementations to represent the general concept of a struct, and must not appear in signatures used on D-Bus.
#define DBUS_TYPE_CHAR_VARIANT	118 // (ASCII 'v') 	Variant type (the type of the value is part of the value itself)
#define DBUS_TYPE_CHAR_DICT_ENTRY	101 // (ASCII 'e'),
#define DBUS_TYPE_CHAR_DICT_ENTRY_BEGIN 123 // (ASCII '{'),
#define DBUS_TYPE_CHAR_DICT_ENTRY_END 125 // (ASCII '}') 	Entry in a dict or map (array of key-value pairs). Type code 101 'e' is reserved for use in bindings and implementations to represent the general concept of a dict or dict-entry, and must not appear in signatures used on D-Bus.
#define DBUS_TYPE_CHAR_UNIX_FD	104 // (ASCII 'h')	Unix file descriptor

#define DBUS_TYPE_INVALID	0 // (ASCII NUL)	Not a valid type code, used to terminate signatures
#define DBUS_TYPE_BYTE	    1 // (ASCII 'y')	8-bit unsigned integer
#define DBUS_TYPE_BOOLEAN	2 // (ASCII 'b')	Boolean value, 0 is FALSE and 1 is TRUE. Everything else is invalid.
#define DBUS_TYPE_INT16	3 // (ASCII 'n')	16-bit signed integer
#define DBUS_TYPE_UINT16	4 // (ASCII 'q')	16-bit unsigned integer
#define DBUS_TYPE_INT32	5 // (ASCII 'i')	32-bit signed integer
#define DBUS_TYPE_UINT32	6 // (ASCII 'u')	32-bit unsigned integer
#define DBUS_TYPE_INT64	7 // (ASCII 'x')	64-bit signed integer
#define DBUS_TYPE_UINT64	8 // (ASCII 't')	64-bit unsigned integer
#define DBUS_TYPE_DOUBLE	9 // (ASCII 'd')	IEEE 754 double
#define DBUS_TYPE_STRING	10 // (ASCII 's')	UTF-8 string (must be valid UTF-8). Must be nul terminated and contain no other nul bytes.
#define DBUS_TYPE_OBJECT_PATH	11 // (ASCII 'o')	Name of an object instance
#define DBUS_TYPE_SIGNATURE	12 // (ASCII 'g')	A type signature
#define DBUS_TYPE_ARRAY	13 // (ASCII 'a')	Array
#define DBUS_TYPE_STRUCT	14 // (ASCII 'r'),
#define DBUS_TYPE_STRUCT_BEGIN 15 // (ASCII '('),
#define DBUS_TYPE_STRUCT_END 16 // (ASCII ')')	Struct; type code 114 'r' is reserved for use in bindings and implementations to represent the general concept of a struct, and must not appear in signatures used on D-Bus.
#define DBUS_TYPE_VARIANT	17 // (ASCII 'v') 	Variant type (the type of the value is part of the value itself)
#define DBUS_TYPE_DICT_ENTRY	18 // (ASCII 'e'),
#define DBUS_TYPE_DICT_ENTRY_BEGIN 19 // (ASCII '{'),
#define DBUS_TYPE_DICT_ENTRY_END 20 // (ASCII '}') 	Entry in a dict or map (array of key-value pairs). Type code 101 'e' is reserved for use in bindings and implementations to represent the general concept of a dict or dict-entry, and must not appear in signatures used on D-Bus.
#define DBUS_TYPE_UNIX_FD	21 // (ASCII 'h')	Unix file descriptor
#define DBUS_TYPE_MAX 21

extern const char *dbus_message_type[];
extern const char *dbus_message_type_short[];
extern const char *dbus_field_code[];
extern const char *dbus_type_string[];

class DltDBusParameter
{
public:
    DltDBusParameter() {}
    ~DltDBusParameter() {}

    char getType() { return type; }
    QString getTypeString();
    void setType(char type) { this->type = type; }

    QVariant getValue() { return value; }
    void setValue(QVariant value) { this->value = value; }

private:
    char type;
    QVariant value;
};

class DltDBusDecoder
{
public:
    DltDBusDecoder();
    ~DltDBusDecoder();

    bool decode(QByteArray &data, bool headerOnly = false);
    bool decodeHeader(QByteArray &data);
    bool decodePayload();

    unsigned char getEndianess() { return endianess; }
    unsigned char getMessageType() { return messageType; }
    unsigned char getFlags() { return flags; }
    unsigned char getVersion() { return version; }
    uint32_t getPayloadLength() { return payloadLength; }
    uint32_t getSerial() { return serial; }
    uint32_t getArrayLength() { return arrayLength; }

    QString getEndianessString();
    QString getMessageTypeString();
    QString getMessageTypeStringShort();
    QString getFlagsString();
    QString getFieldCodeString(unsigned char fieldCode);

    QString getPath() { return path; }
    QString getInterface() { return interfaceName; }
    QString getMember() { return member; }
    QString getErrorName() { return errorName; }
    uint32_t getReplySerial() { return replySerial; }
    QString getDestination() { return destination; }
    QString getSender() { return sender; }
    QByteArray getSignature() { return signature; }
    uint32_t getUnixFds() { return unixFds; }

    QByteArray getPayload() { return payload; }

    QString getLastError() { return error; }

    QList<DltDBusParameter> getParameters() { return parameters; }

protected:

    int padding(uint32_t pos,int alignement);

    int indexOfCascaded(QByteArray signature,char ch, char cascade, int from = 0);
    bool decodePayloadSignature(QByteArray signature,char *dataPtr,int &offset,int maxSize);
    bool decodePayloadParameter(char type,char *dataPtr,int &offset,int maxSize);

    bool readString(QString &data,char *dataPtr,int &offset,int maxSize);
    bool readSignature(QByteArray &data,char *dataPtr,int &offset,int maxSize);
    bool readByte(unsigned char &data,char *dataPtr,int &offset,int maxSize);
    bool readBoolean(bool &data,char *dataPtr,int &offset,int maxSize);
    bool readInt16(int16_t &data,char *dataPtr,int &offset,int maxSize);
    bool readUint16(uint16_t &data,char *dataPtr,int &offset,int maxSize);
    bool readInt32(int32_t &data,char *dataPtr,int &offset,int maxSize);
    bool readUint32(uint32_t &data,char *dataPtr,int &offset,int maxSize);
    bool readInt64(int64_t &data,char *dataPtr,int &offset,int maxSize);
    bool readUint64(uint64_t &data,char *dataPtr,int &offset,int maxSize);
    bool readDouble(double &data,char *dataPtr,int &offset,int maxSize);

private:

    unsigned char endianess;
    unsigned char messageType;
    unsigned char flags;
    unsigned char version;
    uint32_t payloadLength;
    uint32_t serial;
    uint32_t arrayLength;

    QString path;
    QString interfaceName;
    QString member;
    QString errorName;
    uint32_t replySerial;
    QString destination;
    QString sender;
    QByteArray signature;
    uint32_t unixFds;

    QByteArray payload;

    QString error;

    QList<DltDBusParameter> parameters;

};


/*
Gesamtlaenge: 170

Fixed Header: 12
Array Length: 4
Header+HeaderArray: 157
Payload: 10

FIXED HEADER:

Endianess: 6c
Message Type: 04
Flags: 01
Version: 01
Length: 0a 00 00 00
Serial: 02 00 00 00

HEADER Array:

Array Length: 8d 00 00 00 (141)

Byte: 01
Variant Signature: 01 6f
00
String Length: 15 00 00 00
String: 2f 6f 72 67 2f 66 72 65 65 64 65 73 6b 74 6f 70 2f 44 42 75 73 00 (/org/freedesktop/DBus)
00 00
Byte: 02
Variant Signature: 01 73
00
String Length: 14 00 00 00
String: 6f 72 67 2e 66 72 65 65 64 65 73 6b 74 6f 70 2e 44 42 75 73 00 (org.freedesktop.DBus)
00 00 00
Byte: 03
Variant Signature: 01 73
00
String Length: 0c 00 00 00
String: 4e 61 6d 65 41 63 71 75 69 72 65 64 00
NameAcquired
00 00 00
Byte: 06
Variant Signature: 01 73
00
String Length: 05 00 00 00
String: 3a 31 2e 34 33 00
:1.43
00 00
Byte: 08
Variant Signature: 01 67
00
Signature: 01 73
s
00 00
Byte: 07
Variant Signature: 01 73
00
String Length: 14 00 00 00
String: 6f 72 67 2e 66 72 65 65 64 65 73 6b 74 6f 70 2e 44 42 75 73 00
org.freedesktop.DBus
00 00 00

PAYLOAD:

String Length: 05 00 00 00
String: 3a 31 2e 34 33 00
:1.43

 ------dbus message-------l-----------------o-----/org/freedeskto
 p/DBus-----s-----org.freedesktop.DBus------s-----NameAcquired---
 ---s-----:1.43-----g--s----s-----org.freedesktop.DBus--------:1.
 43-

 **/

#endif // DLT_DBUS_DECODER_H
