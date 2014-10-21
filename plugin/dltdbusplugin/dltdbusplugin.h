/**
 * @licence app begin@
 * Copyright (C) 2011-2014  BMW AG
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
 * \file dltdbusplugin.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef DLTDBUSPLUGIN_H
#define DLTDBUSPLUGIN_H

#include <QObject>
#include <QHash>
#include <QMap>

#include "dbus.h"

#include "plugininterface.h"
#include "qdltsegmentedmsg.h"
#include "dltdbuscatalog.h"
#include "form.h"

#define DLT_DBUS_PLUGIN_VERSION "1.0.0"

class DltDbusMethodKey
{
public:
    DltDbusMethodKey() { serial = 0; }
    DltDbusMethodKey(const QString &sender, const uint32_t &serial) { this->sender = sender; this->serial=serial; }

    QString getSender()  const { return sender; }
    uint32_t getSerial() const { return serial; }

private:
    QString sender;
    uint32_t serial;
};

inline bool operator==(const DltDbusMethodKey &e1, const DltDbusMethodKey &e2)
{
    return e1.getSender() == e2.getSender()
           && e1.getSerial() == e2.getSerial();
}

inline uint qHash(const DltDbusMethodKey &key)
{
    return qHash(key.getSender()) ^ key.getSerial();
}

class DltDBusPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDLTPluginDecoderInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDLTPluginDecoderInterface)
#ifdef QT5
    Q_PLUGIN_METADATA(IID "org.genivi.DLT.DltDbusPlugin")
#endif

public:
    DltDBusPlugin();
    ~DltDBusPlugin();

    /* QDLTPluginInterface interface */
    QString name();
    QString pluginVersion();
    QString pluginInterfaceVersion();
    QString description();
    QString error();
    bool loadConfig(QString filename);
    bool saveConfig(QString filename);
    QStringList infoConfig();

    /* QDltPluginViewerInterface */
    QWidget* initViewer();
    void initFileStart(QDltFile *file);
    void initFileFinish();
    void initMsg(int index, QDltMsg &msg);
    void initMsgDecoded(int index, QDltMsg &msg);
    void updateFileStart();
    void updateMsg(int index, QDltMsg &msg);
    void updateMsgDecoded(int index, QDltMsg &msg);
    void updateFileFinish();
    void selectedIdxMsg(int index, QDltMsg &msg);
    void selectedIdxMsgDecoded(int index, QDltMsg &msg);

    /* QDltPluginDecoderInterface */
    bool isMsg(QDltMsg &msg, int triggeredByUser);
    bool decodeMsg(QDltMsg &msg, int triggeredByUser);

    /* internal variables */
    Form *form;

private:

    void methodsAddMsg(QDltMsg &msg);
    void segmentedMsg(QDltMsg &msg);

    QString stringToHtml(QString str);
    bool checkIfDBusMsg(QDltMsg &msg);
    QString decodeMessageToString(DltDBusDecoder &dbusMsg, bool headerOnly = false);

    QDltFile *dltFile;
    QString errorText;

    QHash<DltDbusMethodKey,QString> methods;
    QMap<uint32_t,QDltSegmentedMsg*> segmentedMessages;

    DltDBusCatalog catalog;
};

#endif // DLTDBUSPLUGIN_H
