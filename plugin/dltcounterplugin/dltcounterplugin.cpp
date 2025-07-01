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
 * \file dummycontrolplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>
#include <QTextStream>

#include "dltcounterplugin.h"

DltCounterPlugin::DltCounterPlugin()
{
    form = NULL;
    counterMessages = 0;
    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;
    dltFile = 0;
    dltControl = 0;
    ecuList = 0;
}

DltCounterPlugin::~DltCounterPlugin()
{

}

QString DltCounterPlugin::name()
{
    return QString("DLT Counter Plugin");
}

QString DltCounterPlugin::pluginVersion(){
    return DLT_COUNTER_PLUGIN_VERSION;
}

QString DltCounterPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DltCounterPlugin::description()
{
    return QString();
}

QString DltCounterPlugin::error()
{
    return QString();
}

bool DltCounterPlugin::loadConfig(QString /*filename */ )
{
    return true;
}

bool DltCounterPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DltCounterPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DltCounterPlugin::initViewer()
{
    form = new DltCounter::Form(this);
    return form;
}

bool DltCounterPlugin::initControl(QDltControl *control)
{
    dltControl = control;

    return true;
}

bool DltCounterPlugin::initConnections(QStringList list)
{
    ecuList = new QStringList(list);
    // form->setConnections(list);

    return false;
}

bool DltCounterPlugin::controlMsg(int , QDltMsg &)
{
    return false;
}

void DltCounterPlugin::updateCounters(int ,int )
{

}

bool DltCounterPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname){
    Q_UNUSED(index)
    Q_UNUSED(connectionState)
    Q_UNUSED(hostname)

#if QT_5_SUPPORTED_VERSION
    //qDebug() << ecuList->at(index) << "ConnectionState:" << connectionState << "Hostname:" << hostname << Qt::endl;
#else
    //qDebug() << ecuList->at(index) << "ConnectionState:" << connectionState << "Hostname:" << hostname << endl;
#endif
    return true;
}

bool DltCounterPlugin::autoscrollStateChanged(bool enabled)
{
    // form->autoscrollStateChanged(enabled);

    return true;
}

void DltCounterPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
    Q_UNUSED(pMessageDecoder);
}

void DltCounterPlugin::initMainTableView(QTableView* pTableView)
{
    Q_UNUSED(pTableView);
}

void DltCounterPlugin::configurationChanged()
{}

void DltCounterPlugin::selectedIdxMsg(int , QDltMsg &) {

}

void DltCounterPlugin::selectedIdxMsgDecoded(int , QDltMsg &){

}

void DltCounterPlugin::initFileStart(QDltFile *file){

    clearAll();
    dltFile = file;
}

// The counter data and the context IDs in the log file are accessed
// The data is saved to QList
void DltCounterPlugin::initMsg(int, QDltMsg &msg){

    QString name = msg.getCtid();
    nameList.append(name);

    unsigned char count = msg.getMessageCounter();
    countList.append(count);
}

// The data which are separated as QList, it will be merged to a QMap.
// The consolidatedMap will be accessed by counter push button and export push button.
void DltCounterPlugin::dataConsolidatedMap() {

    for (int i = 0; i < nameList.size() && i < countList.size(); ++i) {
        QString name = nameList[i];
        int count = static_cast<int>(countList[i]);  // Ensure conversion if needed

        consolidatedMap[name].insert(count);  // QSet ensures uniqueness automatically
    }

}

void DltCounterPlugin::initMsgDecoded(int , QDltMsg &){

}

void DltCounterPlugin::initFileFinish(){

}

void DltCounterPlugin::updateFileStart(){

}

void DltCounterPlugin::updateMsg(int , QDltMsg &){

}

void DltCounterPlugin::updateMsgDecoded(int , QDltMsg &){

}
void DltCounterPlugin::updateFileFinish(){

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(dummycontrolplugin, DummyControlPlugin);
#endif
