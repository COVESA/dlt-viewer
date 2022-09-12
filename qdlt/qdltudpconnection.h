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
 * \file qdltudpconnection.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLT_UDP_CONNECTION_H
#define QDLT_UDP_CONNECTION_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"

class QUdpSocket;

class QDLT_EXPORT QDltUDPConnection : public QDltIPConnection
{
public:

    QDltUDPConnection();
    virtual ~QDltUDPConnection();

    virtual void setDefaultPort();

private:
    QUdpSocket *socket;
};

#endif // QDLT_UDP_CONNECTION_H
