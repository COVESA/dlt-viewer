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
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 *
 * \file qdltsegmentedmsg.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLTSEGMENTEDMSG_H
#define QDLTSEGMENTEDMSG_H

#include "export_rules.h"

#include <qdltmsg.h>
#include <dlt_types.h>

//! Combine segmented network messages
/*!
  This class combines several segmented network message to a single message
*/
class QDLT_EXPORT QDltSegmentedMsg
{
public:

    //! Constructor.
    /*!
      Initialise all parameters.
    */
    QDltSegmentedMsg();

    //! Current state of reassemble
    typedef enum { DltSegStart, DltSegChunk, DltSegError, DltSegFinish } DltSegState;

    //! Add message to segmented message. The handle must match the segmented message.
    /*!
      \param msg A segment of the segmented network message. The handle must match.
      \return unequal zero if there was an error
    */
    int add(QDltMsg &msg);

    //! Get handle of network message
    /*!
      \return handle of network message
    */
    uint32_t getHandle() { return handle; }

    //! Check if the message is now complete.
    /*!
      \return true if message is complete
    */
    uint32_t getSize() { return size; }

    //! Get the number of chunks.
    /*!
      \return number of chunks
    */
    uint32_t getChunks() { return chunks; }

    //! Get size of a single chunk
    /*!
      \return size in bytes
    */
    uint32_t getChunksSize() { return chunkSize; }

    //! Get the header of the complete message.
    /*!
      \return header data
    */
    QByteArray getHeader() { return header; }

    //! Get the payload of the complete message
    /*!
      \return payload data
    */
    QByteArray getPayload() { return payload; }

    //! Check if the message is now complete.
    /*!
      \return true if message is complete
    */
    bool complete() { return (state == DltSegFinish); }

    //! Get error string of last failed function call
    /*!
      \return error message
    */
    QString getError() { return error; }

private:

    //! The handle of the network message
    uint32_t handle;

    //! The complete size
    uint32_t size; /* complete size of the message */

    //! The number of chunks
    uint32_t chunks;

    //! The chunk size
    uint32_t chunkSize;

    //! The header data
    QByteArray header;

    //! The payload data
    QByteArray payload;

    //! The number of already received chunks
    uint32_t chunksAdded;

    //! The current state.
    DltSegState state;

    //! The last error string.
    QString error;
};

#endif // QDLTSEGMENTEDMSG_H
