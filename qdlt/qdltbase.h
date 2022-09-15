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
 * \file qdlt.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLT_BASE_H
#define QDLT_BASE_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QMutex>
#include <time.h>

#include "export_rules.h"

extern "C" {
    QDLT_C_EXPORT extern const char *qDltMessageType[];
    QDLT_C_EXPORT extern const char *qDltLogInfo[];
    QDLT_C_EXPORT extern const char *qDltTraceType[];
    QDLT_C_EXPORT extern const char *qDltNwTraceType[];
    QDLT_C_EXPORT extern const char *qDltControlType[];
    QDLT_C_EXPORT extern const char *qDltMode[];
    QDLT_C_EXPORT extern const char *qDltEndianness[];
    QDLT_C_EXPORT extern const char *qDltTypeInfo[];
    QDLT_C_EXPORT extern const char *qDltCtrlServiceId[];
    QDLT_C_EXPORT extern const char *qDltCtrlReturnType[];
}

#define DLT_MAX_MESSAGE_LEN 1024*64
#define DEFAULT_COLOR "#FFFFFF"

struct sDltFile;
struct sDltMessage;

class QSerialPort;
class QTcpSocket;
class QUdpSocket;

//! Base class for all DLT classes.
/*!
  This class contains helper functions needed for all DLT operations and classes.
*/
class QDLT_EXPORT QDlt
{

public:
    //! Constructor.
    /*!
    */
    QDlt();

    //! Destructor.
    /*!
    */
    ~QDlt();

    //! Byte swap some bytes.
    /*!
      \param bytes The data to be swapped
      \param size The number of bytes to be swapped, -1 if all bytes of teh byte array
      \param offset Offset in the byte array where to begin to byte swap
    */
     bool swap(QByteArray &bytes,int size = -1, int offset = 0);

    //! Convert byte array to text or HTML output.
    /*!
      \param bytes The data to be converted
      \param withLineNumber Line numbers are added to each line
      \param withBinary data is output in hex format
      \param withAscii data is output in ASCII if possible
      \param blocksize Binary output in one line is grouped in blocks of bytes
      \param linesize Number of bytes written in one line
      \param toHtml true output is don in HTML, false output in text only
      \return The string with ASCII or html output.
    */
    QString toAsciiTable(const QByteArray &bytes, bool withLineNumber, bool withBinary, bool withAscii, int blocksize = 8, int linesize = 16, bool toHtml = true) const;

    //! Convert byte array to text output.
    /*!
      \param bytes The data to be converted
      \param type 1 output in ASCII, 0 output in hex, 2 output in binary
      \param size_bytes grouping of bytes together (0xff for raw format)
      \return The string with ASCII output.
    */
    QString toAscii(const QByteArray &bytes, int type = false, int size_bytes = 0xff) const;

    //! The endianness of the message.
    typedef enum { DltEndiannessUnknown = -2, DltEndiannessLittleEndian = 0, DltEndiannessBigEndian = 1 } DltEndiannessDef;

protected:


private:


};


#endif // QDLT_BASE_H
