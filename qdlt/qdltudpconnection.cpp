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
 * \file qdltudpconnection.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtDebug>
#include <QUdpSocket>

#include "qdltudpconnection.h"

extern "C"
{
#include "dlt_common.h"
}

QDltUDPConnection::QDltUDPConnection()
: QDltIPConnection()
, socket(0)
{
    hostname = "localhost";
    port = DLT_DAEMON_UDP_PORT;
}

QDltUDPConnection::~QDltUDPConnection()
{
    delete socket;
}

void QDltUDPConnection::setDefaultPort()
{
    port = DLT_DAEMON_UDP_PORT;
}
