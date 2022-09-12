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
 * \file qdltipconnection.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef QDLT_IP_CONNECTION_H
#define QDLT_IP_CONNECTION_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"
#include "qdltconnection.h"

class QDLT_EXPORT QDltIPConnection : public QDltConnection
{
public:

    QDltIPConnection();
    virtual ~QDltIPConnection();

    void setHostname(QString _hostname);
    QString getHostname() const;

    void setPort(unsigned int _ipport);
    virtual void setDefaultPort() { port = 3490; }
    unsigned int getPort() const;

protected:
    QString hostname;
    unsigned int port;
};

#endif // QDLT_IP_CONNECTION_H
