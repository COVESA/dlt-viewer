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
 * \file qdltsegmentedmsg.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QByteArray>

#include "qdlt.h"
#include "qdltsegmentedmsg.h"


QDltSegmentedMsg::QDltSegmentedMsg()
{

    handle = 0;
    size = 0;
    chunks = 0;
    chunkSize = 0;
    chunksAdded = 0;
    state = DltSegStart;
}

int QDltSegmentedMsg::add(QDltMsg &msg)
{
    QDltArgument argument;

    if(msg.getNumberOfArguments()<1)
    {
        error = "Invalid number of arguments in start segment message";
        return -1;
    }

    msg.getArgument(0,argument);

    if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoStrg)
    {
        error = "Invalid Type in start segment message";
        return -1;
    }

    QString str = argument.getValue().toString();

    if(str=="NWST")
    {
        // start of segmented message
        if(msg.getNumberOfArguments()!=6)
        {
            error = "Invalid number of arguments in start segment message";
            return -1;
        }

        msg.getArgument(1,argument);  // get handle
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoUInt)
        {
            error = "Invalid Type in start segment message";
            return -1;
        }
        handle = argument.getValue().toUInt();

        msg.getArgument(2,argument);  // get header
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoRawd)
        {
            error = "Invalid Type in start segment message";
            return -1;
        }
        header = argument.getData();

        msg.getArgument(3,argument);  // get size
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoUInt)
        {
            error = "Invalid Type in start segment message";
            return -1;
        }
        size = argument.getValue().toUInt();

        msg.getArgument(4,argument);  // get chunks
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoUInt)
        {
            error = "Invalid Type in start segment message";
            return -1;
        }
        chunks = argument.getValue().toUInt();

        msg.getArgument(5,argument);  // get chunk size
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoUInt)
        {
            error = "Invalid Type in start segment message";
            return -1;
        }
        chunkSize = argument.getValue().toUInt();

        if(state != DltSegStart)
        {
            error = "Start segment received several times";
            return -1;
        }

        state = DltSegChunk;

        payload.resize(size);
    }
    else if(str=="NWCH")
    {
        // chunk of segmented message
        if(msg.getNumberOfArguments()!=4)
        {
            error = "Invalid number of arguments in chunk segment message";
            return -1;
        }

        msg.getArgument(1,argument);  // get handle
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoUInt)
        {
            error = "Invalid Type in chunk segment message";
            return -1;
        }
        if(handle != argument.getValue())
        {
            error = "Handle unequal in chunk message";
            return -1;
        }

        msg.getArgument(2,argument);  // get sequence
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoUInt)
        {
            error = "Invalid Type in chunk segment message";
            return -1;
        }
        uint32_t sequence = argument.getValue().toUInt();
        if(sequence>=chunks)
        {
            error = QString("Sequence bigger than number of chunks: Sequence = %1, Chunks = %2").arg(sequence).arg(chunks);
            return -1;
        }

        msg.getArgument(3,argument);  // get chunk
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoRawd)
        {
            error = "Invalid Type in chunk segment message";
            return -1;
        }
        payload.replace(sequence*chunkSize,chunkSize,argument.getData());

        chunksAdded += 1;
    }
    else if(str=="NWEN")
    {
        // end of segmented message
        if(msg.getNumberOfArguments()!=2)
        {
            error = "Invalid number of arguments in end segment message";
            return -1;
        }

        msg.getArgument(1,argument);  // get handle
        if(argument.getTypeInfo()!=QDltArgument::DltTypeInfoUInt)
        {
            error = "Invalid Type in end segment message";
            return -1;
        }
        if(handle != argument.getValue())
        {
            error = "Invalid handle in end chunk";
            return -1;
        }

        if(state != DltSegChunk)
        {
            error = "End chunk received without start chunk";
            return -1;
        }

        if(chunksAdded!=chunks)
        {
            error = QString("Number of chunks mismatch: Received = %1, Total = %2").arg(chunksAdded).arg(chunks);
            return -1;
        }

        state = DltSegFinish;
    }
    else
    {
        error = "Invalid segmented message string";
        return -1;
    }

    return 0;
}

