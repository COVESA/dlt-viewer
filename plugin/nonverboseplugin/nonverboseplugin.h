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
 * \file nonverboseplugin.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef NONVERBOSEPLUGIN_H
#define NONVERBOSEPLUGIN_H

#include <QObject>
#include <QHash>
#include "plugininterface.h"

#if defined(_MSC_VER)
#include <cstdint>
#else
#include <stdint.h>
#endif

#define NON_VERBOSE_PLUGIN_VERSION "1.0.0"

class DltFibexKey
{
public:
    DltFibexKey(QString id,QString appid,QString ctid)
    {
        this->id = id;
        this->appid = appid;
        this->ctid = ctid;
    }

    friend bool operator==(const DltFibexKey &e1, const DltFibexKey &e2);
    friend uint qHash(const DltFibexKey &key);

    QString id;
    QString appid;
    QString ctid;
};

inline bool operator==(const DltFibexKey &e1, const DltFibexKey &e2)
{
    return (e1.id == e2.id)
           && (e1.appid == e2.appid) && (e1.ctid == e2.ctid);
}

inline uint qHash(const DltFibexKey &key)
{
    return qHash(key.id) ^ qHash(key.appid) ^ qHash(key.ctid);
}

/**
 * The structure of a Fibex PDU information.
 */
class DltFibexPdu
{
public:
    DltFibexPdu() { byteLength=0;typeInfo=0; }

        QString id;
        QString description;
        int32_t byteLength;
        uint32_t typeInfo;
 };

 class DltFibexPduRef
 {
 public:
        DltFibexPduRef() { ref = 0; }

        QString id;
        DltFibexPdu *ref;
 };

 /**
 * The structure of a Fibex Frame information.
 */
class DltFibexFrame
{
public:
    DltFibexFrame() { byteLength=0;messageType=0;messageInfo=0;pduRefCounter=0; }

        QString id;
        QString filename;
        int32_t byteLength;
        uint8_t messageType;
        int8_t  messageInfo;
        QString appid;
        QString ctid;

        QList<DltFibexPduRef*> pdureflist;
        uint32_t pduRefCounter;
};

class NonverbosePlugin : public QObject, QDLTPluginInterface, QDLTPluginDecoderInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDLTPluginDecoderInterface)
#ifdef QT5
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.NonVerbosePlugin")
#endif

public:
    /* QDLTPluginInterface interface */
    QString name();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString description();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginDecoderInterface */
    bool isMsg(QDltMsg &msg, int triggeredByUser);
    bool decodeMsg(QDltMsg &msg, int triggeredByUser);

    /* Faster lookup */
    //is it necessary that this is public?
    QHash<QString, DltFibexPdu *> pdumap;
    QHash<QString, DltFibexFrame *> framemap;
    QHash<DltFibexKey, DltFibexFrame *> framemapwithkey;

private:
    bool parseFile(QString filename);
    void clear();

    QString m_error_string;
};

#endif // NONVERBOSEPLUGIN_H
