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

#include "dummycontrolplugin.h"

DummyControlPlugin::DummyControlPlugin()
{
    form = NULL;
    counterMessages = 0;
    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;
    dltFile = 0;
    dltControl = 0;
    ecuList = 0;
}

DummyControlPlugin::~DummyControlPlugin()
{

}

QString DummyControlPlugin::name()
{
    return QString("Dummy Control Plugin");
}

QString DummyControlPlugin::pluginVersion(){
    return DUMMY_CONTROL_PLUGIN_VERSION;
}

QString DummyControlPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DummyControlPlugin::description()
{
    return QString();
}

QString DummyControlPlugin::error()
{
    return QString();
}

bool DummyControlPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool DummyControlPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DummyControlPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DummyControlPlugin::initViewer()
{
    form = new DummyControl::Form(this);
    return form;
}

bool DummyControlPlugin::initControl(QDltControl *control)
{
    dltControl = control;

    return true;
}

bool DummyControlPlugin::initConnections(QStringList list)
{
    ecuList = new QStringList(list);
    form->setConnections(list);

    return false;
}

bool DummyControlPlugin::controlMsg(int , QDltMsg &)
{
    return false;
}

void DummyControlPlugin::updateCounters(int ,int )
{

}

bool DummyControlPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname){

    qDebug() << ecuList->at(index) << "ConnectionState:" << connectionState << "Hostname:" << hostname << endl;

    return true;
}

bool DummyControlPlugin::autoscrollStateChanged(bool enabled)
{
    form->autoscrollStateChanged(enabled);

    return true;
}

void DummyControlPlugin::initMessageDecoder(QDltMessageDecoder* pMessageDecoder)
{
    Q_UNUSED(pMessageDecoder);
}

void DummyControlPlugin::initMainTableView(QTableView* pTableView)
{
    Q_UNUSED(pTableView);
}

void DummyControlPlugin::configurationChanged()
{}

void DummyControlPlugin::selectedIdxMsg(int , QDltMsg &) {

}

void DummyControlPlugin::selectedIdxMsgDecoded(int , QDltMsg &){

}

void DummyControlPlugin::initFileStart(QDltFile *file){
    dltFile = file;
}

void DummyControlPlugin::initMsg(int , QDltMsg &){

}
void DummyControlPlugin::initMsgDecoded(int , QDltMsg &){

}

void DummyControlPlugin::initFileFinish(){

}

void DummyControlPlugin::updateFileStart(){

}

void DummyControlPlugin::updateMsg(int , QDltMsg &){

}

void DummyControlPlugin::updateMsgDecoded(int , QDltMsg &){

}
void DummyControlPlugin::updateFileFinish(){

}

#ifndef QT5
Q_EXPORT_PLUGIN2(dummycontrolplugin, DummyControlPlugin);
#endif
