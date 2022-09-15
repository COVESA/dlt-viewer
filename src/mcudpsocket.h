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
 * \file mcudpsocket.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef MCUDPSOCKET_H
#define MCUDPSOCKET_H
#include <QUdpSocket>


class MCUdpSocket : public QUdpSocket
{
    Q_OBJECT

public:
    explicit MCUdpSocket();
    ~MCUdpSocket();
    bool joinMulticastGroup(QString multicast, QString receiveadapter);

private:
    char mcast[17];
    char eth[17];
    int result;
    qintptr socketdescriptor;

};

#endif // MCUDPSOCKET_H
