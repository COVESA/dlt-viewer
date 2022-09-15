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
 * \file dummyviewerplugin.cpp
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#include <QtGui>

#include "dummyviewerplugin.h"

DummyViewerPlugin::DummyViewerPlugin()
{
    form = NULL;
    dltFile = NULL;
    counterMessages = 0;
    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;
}

DummyViewerPlugin::~DummyViewerPlugin()
{

}

QString DummyViewerPlugin::name()
{
    return QString("Dummy Viewer Plugin");
}

QString DummyViewerPlugin::pluginVersion(){
    return DUMMY_VIEWER_PLUGIN_VERSION;
}

QString DummyViewerPlugin::pluginInterfaceVersion(){
    return PLUGIN_INTERFACE_VERSION;
}

QString DummyViewerPlugin::description()
{
    return QString();
}

QString DummyViewerPlugin::error()
{
    return QString();
}

bool DummyViewerPlugin::loadConfig(QString /* filename */)
{
    return true;
}

bool DummyViewerPlugin::saveConfig(QString /* filename */)
{
    return true;
}

QStringList DummyViewerPlugin::infoConfig()
{
    return QStringList();
}

QWidget* DummyViewerPlugin::initViewer()
{
    form = new DummyViewer::Form();
    return form;
}


void DummyViewerPlugin::updateCounters(int , QDltMsg &msg)
{
    if(!dltFile)
        return;



            if(msg.getMode() == QDltMsg::DltModeVerbose)
            {
                counterVerboseMessages++;
            }
            if(msg.getMode() == QDltMsg::DltModeNonVerbose)
            {
                counterNonVerboseMessages++;
            }
}


void DummyViewerPlugin::selectedIdxMsg(int index, QDltMsg &/*msg*/) {
    if(!dltFile)
        return;

    //qDebug() << "undecoded: " << msg.toStringPayload();

    form->setSelectedMessage(index);
}

void DummyViewerPlugin::selectedIdxMsgDecoded(int , QDltMsg &/*msg*/){

    //qDebug() << "decoded: " << msg.toStringPayload();

}

void DummyViewerPlugin::initFileStart(QDltFile *file){
if(nullptr == file)
        return;

    dltFile = file;

   if (form)
   {
    form->setMessages(dltFile->size());
   }

    counterMessages = dltFile->size();

    counterNonVerboseMessages = 0;
    counterVerboseMessages = 0;
}

void DummyViewerPlugin::initMsg(int index, QDltMsg &msg){

    updateCounters(index, msg);
}

void DummyViewerPlugin::initMsgDecoded(int , QDltMsg &){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DummyViewerPlugin::initFileFinish(){
   if(nullptr == dltFile)
        return;
   if (form)
   {
    form->setMessages(dltFile->size());
    form->setVerboseMessages(counterVerboseMessages);
    form->setNonVerboseMessages(counterNonVerboseMessages);
   }
}

void DummyViewerPlugin::updateFileStart(){

}

void DummyViewerPlugin::updateMsg(int index, QDltMsg &msg){
        if(!dltFile)
            return;

        updateCounters(index,msg);

        counterMessages = dltFile->size();
}

void DummyViewerPlugin::updateMsgDecoded(int , QDltMsg &){
//empty. Implemented because derived plugin interface functions are virtual.
}

void DummyViewerPlugin::updateFileFinish(){
   if(nullptr == dltFile)
        return;
    if (form)
   {
    form->setMessages(dltFile->size());
    form->setVerboseMessages(counterVerboseMessages);
    form->setNonVerboseMessages(counterNonVerboseMessages);
   }
}

#ifndef QT5
Q_EXPORT_PLUGIN2(dummyviewerplugin, DummyViewerPlugin);
#endif
