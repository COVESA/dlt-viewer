/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file nonverboseplugin.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef NONVERBOSEPLUGIN_H
#define NONVERBOSEPLUGIN_H

#include <QObject>
#include <QHash>
#include "nonverboseplugin.h"
#include "plugininterface.h"

#define NON_VERBOSE_PLUGIN_VERSION "1.0.0"

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

private:
    QString m_error_string;
};

#endif // NONVERBOSEPLUGIN_H
