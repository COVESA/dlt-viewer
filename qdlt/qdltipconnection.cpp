/**
 * @licence app begin@
 * Copyright (C) 2015-2016  Harman Becker Automotive Systems GmbH
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
 * \author Olaf Dreyer <olaf.dreyer@harman.com>
 *
 * \file qdltipconnection.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtDebug>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltIPConnection::QDltIPConnection()
: QDltConnection()
, hostname("localhost")
, port(DLT_DAEMON_TCP_PORT)
{
}

QDltIPConnection::~QDltIPConnection()
{
}

void QDltIPConnection::setHostname(QString _hostname)
{
    hostname = _hostname;
}

QString QDltIPConnection::getHostname() const
{
    return hostname;
}

void QDltIPConnection::setPort(unsigned int _ipport)
{
    port = _ipport;
}

unsigned int QDltIPConnection::getPort() const
{
    return port;
}
