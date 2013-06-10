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

#ifndef QDLT_TCP_CONNECTION_H
#define QDLT_TCP_CONNECTION_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"

class QTcpSocket;

class QDLT_EXPORT QDltTCPConnection : public QDltConnection
{
public:

    QDltTCPConnection();
    ~QDltTCPConnection();

    void setHostname(QString _hostname);
    QString getHostname();

    void setTcpPort(unsigned int _tcpport);
    void setDefaultTcpPort();
    unsigned int getTcpPort();

private:

    QString hostname;
    unsigned int tcpport;

    QTcpSocket *socket;

};

#endif // QDLT_TCP_CONNECTION_H
