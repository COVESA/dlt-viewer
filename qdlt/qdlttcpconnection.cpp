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
 * \file qdlt.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtDebug>
#include <QTcpSocket>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltTCPConnection::QDltTCPConnection()
    : QDltConnection()
{

}

QDltTCPConnection::~QDltTCPConnection()
{
    hostname = "localhost";
    tcpport = DLT_DAEMON_TCP_PORT;
}

void QDltTCPConnection::setHostname(QString _hostname)
{
    hostname = _hostname;
}

QString QDltTCPConnection::getHostname() const
{
    return hostname;
}

void QDltTCPConnection::setTcpPort(unsigned int _tcpport)
{
    tcpport = _tcpport;
}

void QDltTCPConnection::setDefaultTcpPort()
{
    tcpport = DLT_DAEMON_TCP_PORT;
}

unsigned int QDltTCPConnection::getTcpPort() const
{
    return tcpport;
}
