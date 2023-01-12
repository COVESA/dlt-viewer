/**
 * @licence app begin@
 * Copyright (C) 2011-2014  BMW AG
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
 * \file dltdbusplugin.h
 * For further information see http://www.covesa.global/.
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
#include "form.h"

#define DLT_DBUS_PLUGIN_VERSION "2.0.0"

// we restrict the maximum number of APID/CTID pairs because of performance issues
#define MAX_LOGIDS 10
// maximum allowed number of characters for APID/ CTID
#define LOGIDMAXCHAR 4

typedef struct
{
    QString apid;
    QString ctid;
} s_logid;




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

inline size_t qHash(const DltDbusMethodKey &key)
{
    return qHash(key.getSender()) ^ key.getSerial();
}

class DltDBusPlugin : public QObject, QDLTPluginInterface, QDltPluginViewerInterface, QDLTPluginDecoderInterface,  QDltPluginControlInterface
{
    Q_OBJECT
    Q_INTERFACES(QDLTPluginInterface)
    Q_INTERFACES(QDltPluginViewerInterface)
    Q_INTERFACES(QDltPluginControlInterface)
    Q_INTERFACES(QDLTPluginDecoderInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
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

    /* QDltPluginControlInterface */
    bool initControl(QDltControl *control);
    bool initConnections(QStringList list);
    bool controlMsg(int index, QDltMsg &msg);
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState, QString hostname);
    bool autoscrollStateChanged(bool enabled);
    void initMessageDecoder(QDltMessageDecoder* pMessageDecoder);
    void initMainTableView(QTableView* pTableView);
    void configurationChanged();

    /* QDltPluginDecoderInterface */
    bool isMsg(QDltMsg &msg, int triggeredByUser);
    bool decodeMsg(QDltMsg &msg, int triggeredByUser);

    /* internal variables */
    DltDbus::Form *form;

private:

    void methodsAddMsg(QDltMsg &msg);
    void segmentedMsg(QDltMsg &msg);
    int check_logid( QString &tocheck, int index );
    bool plugin_is_active = false;


    QString stringToHtml(QString str);
    bool checkIfDBusMsg(QDltMsg &msg);
    QString decodeMessageToString(DltDBusDecoder &dbusMsg, bool headerOnly = false);

    QDltFile *dltFile;
    QDltControl *dltControl;
    QString errorText;

    // subsequent stringlist is used to store the APID/CTID combination
    // which is used to detect/select payload to be decoded
    QStringList dbus_mesg_identifiers;
    s_logid logid[MAX_LOGIDS];
    int numberof_valid_logids;
    int countread=0;

    bool config_is_loaded=false;

    QString plugin_name_displayed = QString("DLT DBus Plugin");
    QHash<DltDbusMethodKey,QString> methods;
    QMap<uint32_t,QDltSegmentedMsg*> segmentedMessages;

};

#endif // DLTDBUSPLUGIN_H
