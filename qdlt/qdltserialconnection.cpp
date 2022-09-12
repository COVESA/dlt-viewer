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
 * \file qdlt.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtDebug>
#include <QSerialPort>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltSerialConnection::QDltSerialConnection()
    : QDltConnection()
{
    port = "";
    baudrate = 0;

    m_qdltserialport = 0;
}

QDltSerialConnection::~QDltSerialConnection()
{
    if(m_qdltserialport != NULL)
    {
        delete m_qdltserialport;
        m_qdltserialport = NULL;
    }
}

void QDltSerialConnection::setPort(QString _port)
{
    port = _port;
}

QString QDltSerialConnection::getPort()
{
    return port;
}

void QDltSerialConnection::setBaudrate(int _baudrate)
{
    baudrate = _baudrate;
}

unsigned int QDltSerialConnection::getBaudrate()
{
    return baudrate;
}
