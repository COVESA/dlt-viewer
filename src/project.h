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
 * \file project.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef PROJECT_H
#define PROJECT_H

#include "qdlt.h"

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QTcpSocket>
#include <QDockWidget>
#include <QObject>
#include <QDateTime>
#include <qextserialport.h>
#include "settingsdialog.h"

extern "C"
{
        #include "dlt_common.h"
        #include "dlt_user_shared.h"
}

#include "plugininterface.h"

#define DLT_VIEWER_BUFFER_SIZE 256000
#define RCVBUFSIZE 128000   /* Size of receive buffer */

enum dlt_item_type { ecu_type = QTreeWidgetItem::UserType, application_type, context_type, filter_type, plugin_type };

class EcuItem  : public QTreeWidgetItem
{
public:

    EcuItem(QTreeWidgetItem *parent = 0);
    ~EcuItem();

    void InvalidAll();

    /* configuration all */
    QString id;
    QString description;
    int interfacetype;
    int loglevel;
    int tracestatus;
    int verbosemode;
    bool timingPackets;
    bool sendGetLogInfo;
    enum {invalid,unknown,valid} status;

    /* configuration update */
    bool updateDataIfOnline;
    void update();

    /* connection */
    QTcpSocket socket;

    QextSerialPort *serialport;

    /* connection status */
    int tryToConnect;
    int connected;
    QString connectError;
    bool autoReconnect;
    int autoReconnectTimeout;


    /* current received message and buffer for receivig from the socket */
    int32_t totalBytesRcvd;

    /* AutoReconnecct */
    int32_t totalBytesRcvdLastTimeout;
    bool isAutoReconnectTimeoutPassed();
    void updateAutoReconnectTimestamp();

private:
    QDateTime autoReconnectTimestamp;
    bool operator< ( const QTreeWidgetItem & other ) const;

    /* configuration TCP */
     QString hostname;
     unsigned int tcpport;
     bool sendSerialHeaderTcp;
     bool syncSerialHeaderTcp;

public:
     QDltTCPConnection tcpcon;

     /* Accsesors to config */
     QString getHostname() {return hostname;}
     unsigned int getTcpport() {return tcpport;}
     bool getSendSerialHeaderTcp() {return sendSerialHeaderTcp;}
     bool getSyncSerialHeaderTcp() {return syncSerialHeaderTcp;}

     void setHostname(QString hname) {hostname = hname; tcpcon.setHostname(hostname);}
     void setTcpport(unsigned int tp) {tcpport = tp; tcpcon.setTcpPort(tcpport);}
     void setSendSerialHeaderTcp(bool b) {sendSerialHeaderTcp = b; tcpcon.setSendSerialHeader(sendSerialHeaderTcp);}
     void setSyncSerialHeaderTcp(bool b) {syncSerialHeaderTcp = b; tcpcon.setSyncSerialHeader(syncSerialHeaderTcp);}

private:
     /* Configuration serial */
     QString port;
     BaudRateType baudrate;
     bool sendSerialHeaderSerial;
     bool syncSerialHeaderSerial;

public:
     QDltSerialConnection serialcon;

     /* Accsesors to config */
     QString getPort() {return port;}
     BaudRateType getBaudrate() {return baudrate;}
     bool getSendSerialHeaderSerial() {return sendSerialHeaderSerial;}
     bool getSyncSerialHeaderSerial() {return syncSerialHeaderSerial;}

     void setPort(QString tp) {port = tp; serialcon.setPort(port);}
     void setBaudrate(BaudRateType brt) {baudrate = brt;serialcon.setBaudrate(baudrate);}
     void setSendSerialHeaderSerial(bool b) {sendSerialHeaderSerial = b;serialcon.setSendSerialHeader(sendSerialHeaderSerial);}
     void setSyncSerialHeaderSerial(bool b) {syncSerialHeaderSerial = b;serialcon.setSyncSerialHeader(syncSerialHeaderSerial);}


};

class ApplicationItem  : public QTreeWidgetItem
{
public:

    ApplicationItem(QTreeWidgetItem *parent = 0);
    ~ApplicationItem();

    QString id;
    QString description;

    void update();

private:
    bool operator< ( const QTreeWidgetItem & other ) const;
};

class ContextItem  : public QTreeWidgetItem
{
public:

    ContextItem(QTreeWidgetItem *parent = 0);
    ~ContextItem();

    enum {invalid,unknown,valid} status;

    QString id;
    QString description;
    int loglevel;
    int tracestatus;

    void update();

private:
    bool operator< ( const QTreeWidgetItem & other ) const;
};

class FilterItem  : public QTreeWidgetItem
{
public:

    FilterItem(QTreeWidgetItem *parent = 0);
    ~FilterItem();

    void operator = (FilterItem &item);

    typedef enum { positive = 0, negative, marker } FilterType;

    FilterType type;

    QString name;
    QString ecuId;
    QString applicationId;
    QString contextId;
    QString headerText;
    QString payloadText;

    bool enableRegexp;
    bool enableFilter;
    bool enableEcuId;
    bool enableApplicationId;
    bool enableContextId;
    bool enableHeaderText;
    bool enablePayloadText;
    bool enableLogLevelMin;
    bool enableLogLevelMax;
    bool enableCtrlMsgs;

    QColor filterColour;

    int logLevelMax;
    int logLevelMin;

    void update();

private:

};

//Forward declaration
class MyPluginDockWidget;

class PluginItem  : public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:

    PluginItem(QTreeWidgetItem *parent = 0);
    ~PluginItem();

    enum { ModeDisable=0, ModeEnable, ModeShow };
    enum { TypeFile, TypeDirectory };

    QString getName();
    void setName(QString n);

    QString getPluginVersion();
    void setPluginVersion(QString v);

    QString getPluginInterfaceVersion();
    void setPluginInterfaceVersion(QString iv);

    QString getFilename();
    void setFilename(QString f);

    int getType();
    void setType(int t);

    int getMode();
    void setMode(int m);

    void update();
    void savePluginModeToSettings();
    int getPluginModeFromSettings();

    QDLTPluginInterface *plugininterface;
    QDLTPluginDecoderInterface *plugindecoderinterface;
    QDltPluginViewerInterface  *pluginviewerinterface;
    QDltPluginControlInterface *plugincontrolinterface;
    QDltPluginCommandInterface *plugincommandinterface;
    QWidget *widget;
    MyPluginDockWidget *dockWidget;

private:
    QString name;
    QString pluginVersion;
    QString pluginInterfaceVersion;
    QString filename;

    int type;
    int mode;

};

class MyPluginDockWidget : public QDockWidget{

public:
    MyPluginDockWidget();
    MyPluginDockWidget(PluginItem *i, QWidget *parent=0);
    ~MyPluginDockWidget();
private:
    PluginItem *pluginitem;
    void closeEvent(QCloseEvent *event);
};

class Project
{
public:

    Project();
    ~Project();

    bool Load(QString filename);
    bool Save(QString filename);
    void Clear();

    bool SaveFilter(QString filename);
    bool LoadFilter(QString filename,bool replace);

    QTreeWidget *ecu;
    QTreeWidget *filter;
    QTreeWidget *plugin;
    SettingsDialog *settings;

private:


};

#endif // PROJECT_H
