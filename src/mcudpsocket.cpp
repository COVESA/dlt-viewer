#include "mcudpsocket.h"
#include <QDebug>

/*
extern "C"
{

}
*/
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
 * \file mcudpsocket.cpp
 * Gernot Wirschal 23.01.2019
 * This file was added to solve a QT problem when joining UDP multicast
 * group on Windows in case a specific incoming interface has to be seleted.
 * @licence end@
 */

#ifdef WIN32
#pragma comment(lib, "wsock32.lib")
#include <io.h>
#include <time.h>
#include <WinSock2.h>
#include <WS2tcpip.h> // needed because of ip_mreq
#else
#include <unistd.h>     /* for read(), close() */
#include <sys/time.h>	/* for gettimeofday() */
#include <arpa/inet.h>  // needed because of ip_mreq
#endif

MCUdpSocket::MCUdpSocket()
{
}

MCUdpSocket::~MCUdpSocket()
{
}

bool MCUdpSocket::joinMulticastGroup(QString multicast, QString receiveadapter)
{
    struct ip_mreq mreq;
    socketdescriptor = this->socketDescriptor();
    snprintf(mcast,16,"%s",multicast.toStdString().c_str());
    snprintf(eth,16,"%s",receiveadapter.toStdString().c_str());
    mreq.imr_multiaddr.s_addr = inet_addr(mcast);
    mreq.imr_interface.s_addr =inet_addr(eth);
    result =  setsockopt(socketdescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,  (char *) &mreq, sizeof(mreq));

    if ( result == 0)
     return true;
    else
     return false;
}
