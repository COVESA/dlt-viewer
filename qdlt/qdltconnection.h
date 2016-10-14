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
 * \file qdlt.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef QDLT_CONNECTION_H
#define QDLT_CONNECTION_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"

class QDLT_EXPORT QDltConnection
{

public:
    //! The possible DLT connection states of an ECU
    enum QDltConnectionState{QDltConnectionOffline=0, QDltConnectionConnecting, QDltConnectionOnline, QDltConnectionError};

    QDltConnection();
    ~QDltConnection();

    void setSendSerialHeader(bool _sendSerialHeader);
    bool getSendSerialHeader() const;

    void setSyncSerialHeader(bool _syncSerialHeader);
    bool getSyncSerialHeader() const;

    bool parse(QDltMsg &msg);

    void clear();
    void add(const QByteArray &bytes);

    QByteArray data;

    unsigned long bytesReceived;
    unsigned long bytesError;
    unsigned long syncFound;

protected:

    bool sendSerialHeader;
    bool syncSerialHeader;



};

#endif // QDLT_CONNECTION_H
