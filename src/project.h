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


#include <QTreeWidget>
#include <QDockWidget>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QObject>
#include <QDateTime>
#include <QSerialPort>
#include <QPluginLoader>

#if defined(_MSC_VER)
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "settingsdialog.h"
#include "mcudpsocket.h"

extern "C"
{

}

#include "plugininterface.h"

#define DLT_VIEWER_BUFFER_SIZE 256000
#define RCVBUFSIZE 128000   /* Size of receive buffer */
#define RECONNECT_TIMEOUT 3




enum dlt_item_type { ecu_type = QTreeWidgetItem::UserType, application_type, context_type, filter_type, plugin_type };

class EcuItem  : public QTreeWidgetItem
{
public:
    enum {
        INTERFACETYPE_TCP,
        INTERFACETYPE_UDP,
        INTERFACETYPE_SERIAL_DLT,
        INTERFACETYPE_SERIAL_ASCII
    };

    EcuItem(QTreeWidgetItem *parent = 0);
    ~EcuItem();

    void InvalidAll();

    /* configuration all */
    QString id;
    QString default_id = "ECU";
    QString description;
    int interfacetype;
    int loglevel;
    int tracestatus;
    int verbosemode;
    bool timingPackets;
    bool sendGetLogInfo;
    bool sendDefaultLogLevel;
    bool sendGetSoftwareVersion;
    enum {invalid,unknown,valid} status;

    /* configuration update */
    bool updateDataIfOnline;
    void update();

    /* connection */
    QTcpSocket tcpsocket;
    QUdpSocket udpsocket;
    QAbstractSocket * socket;

    QSerialPort *m_serialport;

    /* connection status */
    bool tryToConnect;
    bool connected;

    QString connectError;
    bool autoReconnect;
    int autoReconnectTimeout;
    bool is_multicast;

    /* current received message and buffer for receivig from the socket */
    unsigned long totalBytesRcvd;

    /* AutoReconnecct */
    int32_t totalBytesRcvdLastTimeout;
    bool isAutoReconnectTimeoutPassed();
    void updateAutoReconnectTimestamp();

private:
    QDateTime autoReconnectTimestamp;
    bool operator< ( const QTreeWidgetItem & other ) const;

    /* configuration TCP / UDP */
     QString hostname;
     QString mcastIP;
     QString ethIF; /* needed for UDP multicast, identify interface via IP address */
     unsigned int ipport;
     unsigned int udpport; /*basically the same*/
     bool sendSerialHeaderIp;
     bool syncSerialHeaderIp;

public:
     QDltIPConnection ipcon;

     /* Accsesors to config */
     QString getHostname() {return hostname;}
     QString getmcastIP() {return mcastIP;}
     QString getEthIF() {return ethIF;}
     unsigned int getIpport() {return ipport;}
     unsigned int getUdpport() {return udpport;}
     bool getSendSerialHeaderIp() {return sendSerialHeaderIp;}
     bool getSyncSerialHeaderIp() {return syncSerialHeaderIp;}

     void setHostname(QString hname) {hostname = hname; ipcon.setHostname(hostname);}
     void setIpport(unsigned int tp) {ipport = tp; ipcon.setPort(ipport);}
     void setUdpport(unsigned int tp) {udpport = tp; ipcon.setPort(udpport);}
     void setEthIF(QString ethif) {ethIF = ethif;}
     void setmcastIP(QString mcastip) {mcastIP = mcastip;}
     void setSendSerialHeaderIp(bool b) {sendSerialHeaderIp = b; ipcon.setSendSerialHeader(sendSerialHeaderIp);}
     void setSyncSerialHeaderIp(bool b) {syncSerialHeaderIp = b; ipcon.setSyncSerialHeader(syncSerialHeaderIp);}

private:
     /* Configuration serial */
     QString port;
     QSerialPort::BaudRate baudrate;
     bool sendSerialHeaderSerial;
     bool syncSerialHeaderSerial;

public:
     QDltSerialConnection serialcon;

     /* Accsesors to config */
     QString getPort() {return port;}
     QSerialPort::BaudRate getBaudrate() {return baudrate;}
     bool getSendSerialHeaderSerial() {return sendSerialHeaderSerial;}
     bool getSyncSerialHeaderSerial() {return syncSerialHeaderSerial;}

     void setPort(QString tp) {port = tp; serialcon.setPort(port);}
     void setBaudrate(QSerialPort::BaudRate brt) {baudrate = brt;serialcon.setBaudrate(baudrate);}
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

    QDltFilter filter;

    void update();

private:

};

//Forward declaration
class MyPluginDockWidget;

class PluginItem  : public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:

    PluginItem(QTreeWidgetItem *parent, QDltPlugin *_plugin);
    ~PluginItem();

    enum { TypeFile, TypeDirectory };

    QString getName();

    QString getPluginVersion();

    QString getPluginInterfaceVersion();

    QString getFilename();
    void setFilename(QString f);

    int getType();
    void setType(int t);

    int getMode();
    void setMode(int t);

    void update();

    QDltPlugin* getPlugin() { return plugin; }

    void savePluginModeToSettings();
    void loadPluginModeFromSettings();

    QWidget *widget;
    MyPluginDockWidget *dockWidget;
    QPluginLoader *loader;

private:
    QString name;
    QString pluginVersion;
    QString pluginInterfaceVersion;
    QString filename;

    int type;
    int mode;

    QDltPlugin *plugin;

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
    //SettingsDialog *settings;
    QDltSettingsManager *settings;

private:


};

#endif // PROJECT_H
