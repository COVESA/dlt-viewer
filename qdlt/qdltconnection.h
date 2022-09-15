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

#ifndef QDLT_CONNECTION_H
#define QDLT_CONNECTION_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
//#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"
#include "qdltmsg.h"

class QDLT_EXPORT QDltDataView
{
public:
    QDltDataView(const char* data, int size)
        : m_data(data)
        , m_size(size)
        , m_position()
    {}

    QDltDataView(const QByteArray& byteArray, int position = 0)
        : m_data(byteArray.constData())
        , m_size(byteArray.size())
        , m_position(position)
    {}

    void align(const QByteArray& byteArray, int position = 0)
    {
        m_data = byteArray.constData();
        m_size = byteArray.size();
        m_position = position;
    }

    operator const QByteArray() { return QByteArray::fromRawData(m_data + m_position, m_size - m_position); }

    const QByteArray mid(int pos, int len = -1)
    {
        if (len < 0) len = size() - pos;
        if (pos > size()) pos = size();
        if (pos + len > size()) len = size() - pos;
        return QByteArray::fromRawData(m_data + m_position + pos, len);
    }

    void advance(int num)
    {
        if (num < 0) num = 0;
        m_position += num;
        if (m_position > m_size) m_position = m_size;
    }

    const char* data() { return m_data + m_position; }
    const char* constData() { return m_data + m_position; }
    int size() { return m_size - m_position; }
    void clear() { m_position = m_size; }

private:
    const char * m_data;
    int m_size;
    int m_position;
};

class QDLT_EXPORT QDltConnection
{

public:
    //! The possible DLT connection states of an ECU
    enum QDltConnectionState{QDltConnectionOffline=0, QDltConnectionConnecting, QDltConnectionOnline, QDltConnectionError};

    QDltConnection();
    ~QDltConnection();

    void setSendSerialHeader(bool _sendSerialHeader);
    bool getSendSerialHeader() const;

    void setSyncSerialHeader(bool _syncSerialHeader);
    bool getSyncSerialHeader() const;

    bool parseDlt(QDltMsg &msg);
    bool parseAscii(QDltMsg &msg);

    void clear();
    void add(const QByteArray &bytes);

    QByteArray data;
    QDltDataView dataView = {data};

    unsigned long bytesReceived;
    unsigned long bytesError;
    unsigned long syncFound;

protected:

    bool sendSerialHeader;
    bool syncSerialHeader;

private:

    unsigned char messageCounter;

};

#endif // QDLT_CONNECTION_H
