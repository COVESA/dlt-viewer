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
 * \file qdlt.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLT_TCP_CONNECTION_H
#define QDLT_TCP_CONNECTION_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"
#include "qdltipconnection.h"

class QTcpSocket;

class QDLT_EXPORT QDltTCPConnection : public QDltIPConnection
{
public:

    QDltTCPConnection();
    ~QDltTCPConnection();

    virtual void setDefaultPort();

private:
    QTcpSocket *socket;
};

#endif // QDLT_TCP_CONNECTION_H
