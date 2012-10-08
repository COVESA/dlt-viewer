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
 * \file dummycontrolplugin.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtGui>

#include "dummycontrolplugin.h"

DummyControlPlugin::DummyControlPlugin()
{
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
    form = new Form(this);
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

bool DummyControlPlugin::controlMsg(int index, QDltMsg &msg)
{
    return false;
}

void DummyControlPlugin::updateCounters(int start,int end)
{

}

bool DummyControlPlugin::stateChanged(int index, QDltConnection::QDltConnectionState connectionState){

    qDebug() << ecuList->at(index) << "ConnectionState: " << connectionState << endl;

    return true;
}


void DummyControlPlugin::selectedIdxMsg(int index, QDltMsg &msg) {

}


void DummyControlPlugin::initFileStart(QDltFile *file){
    dltFile = file;
}

void DummyControlPlugin::initMsg(int index, QDltMsg &msg){

}
void DummyControlPlugin::initMsgDecoded(int index, QDltMsg &msg){

}

void DummyControlPlugin::initFileFinish(){

}

void DummyControlPlugin::updateFileStart(){

}

void DummyControlPlugin::updateMsg(int index, QDltMsg &msg){

}

void DummyControlPlugin::updateMsgDecoded(int index, QDltMsg &msg){

}
void DummyControlPlugin::updateFileFinish(){

}

Q_EXPORT_PLUGIN2(dummycontrolplugin, DummyControlPlugin);
