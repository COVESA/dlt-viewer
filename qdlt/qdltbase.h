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
 * \file qdltbase.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef QDLT_BASE_H
#define QDLT_BASE_H

#include <QString>

#include "export_rules.h"

inline constexpr const auto DLT_MAX_MESSAGE_LEN = 1024*64;

//! Base class for all DLT classes.
/*!
  This class contains helper functions needed for all DLT operations and classes.
*/
class QDLT_EXPORT QDlt
{
public:
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
    static QString toAsciiTable(const QByteArray &bytes, bool withLineNumber, bool withBinary, bool withAscii, int blocksize = 8, int linesize = 16, bool toHtml = true);

    //! Convert byte array to text output.
    /*!
      \param bytes The data to be converted
      \param type 1 output in ASCII, 0 output in hex, 2 output in binary
      \param size_bytes grouping of bytes together (0xff for raw format)
      \return The string with ASCII output.
    */
    static QString toAscii(const QByteArray &bytes, int type = false, int size_bytes = 0xff);

    //! The endianness of the message.
    enum DltEndiannessDef { DltEndiannessUnknown = -2, DltEndiannessLittleEndian = 0, DltEndiannessBigEndian = 1 };
};


#endif // QDLT_BASE_H
